
#include "tracker.h"
#include "utils.h"
#include "csvparser.h"

#include <osmscout/gpx/Export.h>
#include <cmath>
#include <QDebug>

#define SEGMENT_SIZE    50
#define MOVING_SPEED    1.0
#define COURSE_SPEED    3.0
#define BASE_DIRECTORY  "TRACKER"

Tracker::Tracker(QObject* parent)
: QObject(parent)
, m_p(nullptr)
, m_vehicle(osmscout::Vehicle::vehicleCar)
, m_vehicleState(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_vehicleCoord(0, 0)
, m_vehicleBearing(new osmscout::Bearing())
, m_elevation(0)
, m_currentSpeed(0)
, m_distance(0)
, m_duration(0)
, m_ascent(0)
, m_descent(0)
//, m_nextRouteStep()
//, m_remainingDistance()
{
}

Tracker::~Tracker()
{
  if (m_p)
    delete m_p;
  qInfo(__FUNCTION__);
}

bool Tracker::init(const QString& root)
{
  QThread* thread = new QThread();
  thread->setObjectName("tracker");
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  m_p = new TrackerModule(thread, root);
  m_p->moveToThread(thread);
  thread->start();

  connect(this, &Tracker::locationUpdate, m_p, &TrackerModule::onLocationChanged, Qt::QueuedConnection);
  connect(this, &Tracker::azimuthUpdate, m_p, &TrackerModule::onAzimuthChanged, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::positionChanged, this, &Tracker::onPositionChanged, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::dataChanged, this, &Tracker::onDataChanged, Qt::QueuedConnection);
  connect(this, &Tracker::doReset, m_p, &TrackerModule::onReset, Qt::QueuedConnection);
  connect(this, &Tracker::doStartRecording, m_p, &TrackerModule::onStartRecording, Qt::QueuedConnection);
  connect(this, &Tracker::doResumeRecording, m_p, &TrackerModule::onResumeRecording, Qt::QueuedConnection);
  connect(this, &Tracker::doStopRecording, m_p, &TrackerModule::onStopRecording, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::recordingChanged, this, &Tracker::onRecordingChanged, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::recordingFailed, this, &Tracker::recordingFailed, Qt::QueuedConnection);

  return true;
}

osmscout::VehiclePosition* Tracker::getTrackerPosition() const
{
  std::shared_ptr<osmscout::GeoCoord> nextPosition(nullptr);
  //if (!m_nextRouteStep.getType().isEmpty())
  //  nextPosition = std::make_shared<osmscout::GeoCoord>(m_nextRouteStep.GetCoord());
  return new osmscout::VehiclePosition(m_vehicle, m_vehicleState, m_vehicleCoord, m_vehicleBearing, nextPosition);
}

void Tracker::setRecording(const QString& filename)
{
  emit resumeRecording(filename);
}

void Tracker::locationChanged(bool positionValid, double lat, double lon,
                              bool horizontalAccuracyValid, double horizontalAccuracy,
                              double alt /*= 0.0*/)
{
  Q_UNUSED(horizontalAccuracyValid)
  Q_UNUSED(horizontalAccuracy)
  m_elevation = alt;
  emit locationUpdate(positionValid, lat, lon, alt);
}

void Tracker::azimuthChanged(double azimuth)
{
  emit azimuthUpdate(azimuth);
}

void Tracker::reset()
{
  emit doReset();
}

void Tracker::startRecording()
{
  emit doStartRecording();
}

void Tracker::resumeRecording(const QString& filename)
{
  emit doResumeRecording(filename);
}

void Tracker::stopRecording()
{
  emit doStopRecording();
}

void Tracker::onPositionChanged(const osmscout::PositionAgent::PositionState state,
                       const osmscout::GeoCoord coord,
                       const std::shared_ptr<osmscout::Bearing> bearing)
{
  m_vehicleState = state;
  m_vehicleCoord = coord;
  m_vehicleBearing = bearing;
  emit trackerPositionChanged();
}

void Tracker::onDataChanged(double kmph, double meters, double seconds, double ascent, double descent)
{
  m_currentSpeed = kmph;
  m_distance = meters;
  m_duration = seconds;
  m_ascent = ascent;
  m_descent = descent;
  emit trackerDataChanged();
}

void Tracker::onRecordingChanged(const QString& filename)
{
  m_recording = filename;
  emit trackerRecordingChanged();
}

TrackerModule::TrackerModule(QThread* thread, const QString& root)
: m_t(thread)
, m_baseDir()
, m_timer()
, m_state(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_azimuth(0)
, m_currentSpeed(0)
, m_lastPosition()
, m_distance(0)
, m_duration(0)
, m_ascent(0)
, m_descent(0)
, m_recording(false)
, m_segment()
, m_file(nullptr)
, m_formater(nullptr)
{
  QDir rootDir(root);
  if (!rootDir.exists(BASE_DIRECTORY))
    rootDir.mkdir(BASE_DIRECTORY);
  m_baseDir.setPath(rootDir.absoluteFilePath(BASE_DIRECTORY));
  m_segment.reserve(SEGMENT_SIZE);
  m_formater = new osmin::CSVParser(',', '"');
  m_timer.moveToThread(thread);
  connect(&m_timer, &QTimer::timeout, this, &TrackerModule::onTimeout);
}

TrackerModule::~TrackerModule()
{
  onFlushRecording();
  if (m_t)
    m_t->quit();
  qInfo(__FUNCTION__);
}

void TrackerModule::onTimeout()
{
}

void TrackerModule::onLocationChanged(bool positionValid, double lat, double lon, double alt)
{
  if (!positionValid)
    m_state = osmscout::PositionAgent::PositionState::NoGpsSignal;
  else
    m_state = osmscout::PositionAgent::PositionState::NoRoute;

  osmscout::Timestamp now = std::chrono::system_clock::now();

  if (m_lastPosition)
  {
    double d = osmin::Utils::sphericalDistance(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon);
    auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(now - m_lastPosition.time);
    if (sec.count() > 0)
    {
      record();
      m_currentSpeed = 3.6 * d / sec.count();
      // update tracking data when moving
      if (m_currentSpeed > MOVING_SPEED)
      {
        m_distance += d;
        m_duration += sec.count();
        double a = alt - m_lastPosition.elevation;
        if (a > 0)
          m_ascent += a;
        else
          m_descent += a;
      }
      emit dataChanged(m_currentSpeed, m_distance, m_duration, m_ascent, m_descent);
    }
    if (m_currentSpeed < COURSE_SPEED)
      m_lastPosition.bearing = osmscout::Bearing::Degrees(m_azimuth);
    else
      m_lastPosition.bearing = osmscout::Bearing::Radians(osmin::Utils::sphericalBearingFinal(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon));
  }
  else
  {
    m_lastPosition.bearing = osmscout::Bearing::Degrees(m_azimuth);
  }

  m_lastPosition.coord.Set(lat, lon);
  m_lastPosition.elevation = alt;
  m_lastPosition.time = now;
  emit positionChanged(m_state, m_lastPosition.coord, std::make_shared<osmscout::Bearing>(m_lastPosition.bearing));
}

void TrackerModule::onAzimuthChanged(double degrees)
{
  m_azimuth = degrees;
  if (m_currentSpeed < COURSE_SPEED)
  {
    m_lastPosition.bearing = osmscout::Bearing::Degrees(m_azimuth);
    emit positionChanged(m_state, m_lastPosition.coord, std::make_shared<osmscout::Bearing>(m_lastPosition.bearing));
  }
}

void TrackerModule::onReset()
{
  m_distance = 0;
  m_duration = 0;
  m_ascent = 0;
  m_descent = 0;
}

void TrackerModule::onStartRecording()
{
  qDebug(__FUNCTION__);
  if (m_recording)
    onStopRecording();
  // prepare the new file to record data
  m_file.reset(new QFile(m_baseDir.absoluteFilePath(QDateTime::currentDateTime().toString(Qt::ISODate).append(".csv"))));
  if (!m_file->open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text))
  {
    emit recordingFailed();
  }
  else
  {
    m_file->close();
    m_recording = true;
    emit recordingChanged(m_file->fileName());
  }
}

void TrackerModule::onResumeRecording(const QString& filename)
{
  if (m_recording)
    return;
  // register file to record data
  m_file.reset(new QFile(filename));
  if (!m_file->open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text))
  {
    emit recordingChanged(QString());
    emit recordingFailed();
  }
  else
  {
    m_file->close();
    m_recording = true;
    emit recordingChanged(m_file->fileName());
  }
}

void TrackerModule::onStopRecording()
{
  qDebug(__FUNCTION__);
  onFlushRecording();
  QSharedPointer<QFile> file = m_file;
  m_recording = false;
  emit recordingChanged(QString());
  if (file.isNull() || !file->open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  // fill segment
  osmscout::gpx::TrackSegment segment;
  segment.points.reserve(1000);
  size_t c = 0;
  for (;;)
  {
    QList<QByteArray*> row = m_formater->deserialize(file->readLine(0x3ff));
    if (row.length() == 0)
      break;
    else if (row.length() >= 5)
    {
      if (++c > segment.points.size())
        segment.points.reserve(segment.points.size() + 1000);
      osmscout::Timestamp time(std::chrono::milliseconds(static_cast<long>(std::round(QString::fromUtf8(row[0]->constData()).toDouble() * 1000))));
      double lat = QString::fromUtf8(row[1]->constData()).toDouble();
      double lon = QString::fromUtf8(row[2]->constData()).toDouble();
      double cse = QString::fromUtf8(row[3]->constData()).toDouble();
      double alt = QString::fromUtf8(row[4]->constData()).toDouble();
      osmscout::gpx::TrackPoint point(osmscout::GeoCoord(lat, lon));
      point.time = osmscout::gpx::Optional<osmscout::Timestamp>::of(time);
      point.course = osmscout::gpx::Optional<double>::of(cse);
      point.elevation = osmscout::gpx::Optional<double>::of(alt);
      segment.points.emplace_back(point);
    }
    qDeleteAll(row);
  }
  file->close();
  // generate a simple GPX file
  if (c > 0)
  {
    osmscout::gpx::Track track;
    track.segments.push_back(segment);
    track.name = osmscout::gpx::Optional<std::string>::of("track");
    osmscout::gpx::GpxFile gpx;
    gpx.tracks.push_back(track);
    gpx.desc = osmscout::gpx::Optional<std::string>::of("Generated by Osmin Traker");
    gpx.time = osmscout::gpx::Optional<osmscout::Timestamp>::of(segment.points[0].time.get());
    osmscout::gpx::Waypoint beg(segment.points[0].coord);
    beg.elevation = osmscout::gpx::Optional<double>::of(segment.points[0].elevation.get());
    beg.symbol = osmscout::gpx::Optional<std::string>::of("Flag");
    beg.time = osmscout::gpx::Optional<osmscout::Timestamp>::of(segment.points[0].time.get());
    osmscout::gpx::Waypoint end(segment.points[segment.points.size() - 1].coord);
    end.elevation = osmscout::gpx::Optional<double>::of(segment.points[segment.points.size() - 1].elevation.get());
    end.symbol = osmscout::gpx::Optional<std::string>::of("Flag");
    end.time = osmscout::gpx::Optional<osmscout::Timestamp>::of(segment.points[segment.points.size() - 1].time.get());
    gpx.waypoints.push_back(beg);
    gpx.waypoints.push_back(end);
    QDateTime fdate = QDateTime();
    fdate.setMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(beg.time.get().time_since_epoch()).count());
    gpx.name = osmscout::gpx::Optional<std::string>::of(fdate.toString(Qt::ISODate).toUtf8().toStdString());
    QString fname = fdate.toString(Qt::ISODate).append(".gpx");
    if (!osmscout::gpx::ExportGpx(gpx, m_baseDir.absoluteFilePath(fname).toUtf8().toStdString()))
    {
      emit recordingFailed();
      return;
    }
  }
  file->remove();
}

void TrackerModule::onFlushRecording()
{
  qDebug(__FUNCTION__);
  QSharedPointer<QFile> file = m_file;
  if (!m_recording || file.isNull())
  {
    m_segment.clear();
    return;
  }
  if (m_file->open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text))
  {
    while (!m_segment.empty())
    {
      const auto p = m_segment.front();
      QList<QByteArray*> row;
      QString num;
      auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(p.time.get().time_since_epoch());
      row << new QByteArray(num.setNum(sec.count(), 'f', 3).toUtf8());
      row << new QByteArray(num.setNum(p.coord.GetLat(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(p.coord.GetLon(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(p.course.get(), 'f', 1).toUtf8());
      row << new QByteArray(num.setNum(p.elevation.get(), 'f', 1).toUtf8());
      row << new QByteArray("");
      QByteArray data = m_formater->serialize(row);
      qDeleteAll(row);
      data.append("\r\n");
      m_file->write(data);
      m_segment.pop_front();
    }
    m_file->close();
  }
}

void TrackerModule::record()
{
  if (m_segment.count() >= SEGMENT_SIZE)
    onFlushRecording();
  osmscout::gpx::TrackPoint point(m_lastPosition.coord);
  point.time = osmscout::gpx::Optional<osmscout::Timestamp>::of(m_lastPosition.time);
  point.course = osmscout::gpx::Optional<double>::of(m_lastPosition.bearing.AsDegrees());
  point.elevation = osmscout::gpx::Optional<double>::of(m_lastPosition.elevation);
  m_segment.append(point);
}
