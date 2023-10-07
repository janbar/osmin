/*
 * Copyright (C) 2020
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tracker.h"
#include "utils.h"
#include "csvparser.h"

#include <osmscoutgpx/Export.h>
#include <cmath>
#include <QDebug>

#define SEGMENT_SIZE    20          /* when reaching this limit it will flush data to disk */
#define MOVING_SPEED    1.0         /* motion detection */
#define COURSE_SPEED    3.0         /* below this speed the direction is determined with sensor */
#define BASE_DIRECTORY  "TRACKER"
#define RECORD_INTERVAL 50.0        /* max distance interval between recorded positions */

#define TAG_TRKPT       "TRKPT"
#define TAG_WAYPT       "WAYPT"
#define MARK_SYMBOL     "Pin, Red"

Tracker::Tracker(QObject* parent)
: QObject(parent)
, m_p(nullptr)
, m_vehicle(osmscout::Vehicle::vehicleCar)
, m_vehicleState(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_vehicleCoord(0, 0)
, m_vehicleBearing(osmscout::Bearing())
, m_elevation(0)
, m_currentSpeed(0)
, m_distance(0)
, m_duration(0)
, m_ascent(0)
, m_descent(0)
, m_maxSpeed(0)
, m_busy(false)
//, m_nextRouteStep()
//, m_remainingDistance()
{
}

Tracker::~Tracker()
{
  if (m_p)
    delete m_p;
  qInfo("%s", __FUNCTION__);
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
  connect(this, &Tracker::doMarkPosition, m_p, &TrackerModule::onMarkPosition, Qt::QueuedConnection);
  connect(this, &Tracker::doDumpRecording, m_p, &TrackerModule::onDumpRecording, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::recordingChanged, this, &Tracker::onRecordingChanged, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::recordingFailed, this, &Tracker::recordingFailed, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::processing, this, &Tracker::onProcessingChanged, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::positionRecorded, this, &Tracker::onPositionRecorded, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::positionMarked, this, &Tracker::onPositionMarked, Qt::QueuedConnection);

  return true;
}

osmscout::VehiclePosition* Tracker::getTrackerPosition() const
{
  std::optional<osmscout::GeoCoord> nextPosition;
  //if (!m_nextRouteStep.getType().isEmpty())
  //  nextPosition = std::make_shared<osmscout::GeoCoord>(m_nextRouteStep.GetCoord());
  return new osmscout::VehiclePosition(m_vehicle, m_vehicleState, m_vehicleCoord, m_vehicleBearing, nextPosition);
}

void Tracker::setRecording(const QString& filename)
{
  emit doResumeRecording(filename);
}

bool Tracker::getIsRecording() const
{
  return (m_p ? m_p->isRecording() : false);
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
  // the reset is only effective if there is no recording in progress
  if (!getIsRecording())
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

void Tracker::pinPosition()
{
  if (m_p && m_p->isRecording()) {
    m_p->pinPosition();
  }
}

void Tracker::markPosition(const QString &symbol, const QString &name, const QString &description)
{
  emit doMarkPosition(symbol, name, description);
}

void Tracker::dumpRecording()
{
  emit doDumpRecording();
}

void Tracker::onPositionChanged(const osmscout::PositionAgent::PositionState state,
                       const osmscout::GeoCoord coord,
                       const std::optional<osmscout::Bearing> bearing)
{
  m_vehicleState = state;
  m_vehicleCoord = coord;
  m_vehicleBearing = bearing;
  emit trackerPositionChanged();
}

void Tracker::onDataChanged(double kmph, double meters, double seconds, double ascent, double descent, double maxkmph)
{
  m_currentSpeed = kmph;
  m_distance = meters;
  m_duration = seconds;
  m_ascent = ascent;
  m_descent = descent;
  m_maxSpeed = maxkmph;
  emit trackerDataChanged();
}

void Tracker::onRecordingChanged(const QString& filename)
{
  m_recording = filename;
  emit trackerRecordingChanged();
}

void Tracker::onProcessingChanged(bool busy)
{
  m_busy = busy;
  emit trackerProcessingChanged();
}

void Tracker::onPositionRecorded(const osmscout::GeoCoord coord)
{
  emit trackerPositionRecorded(coord.GetLat(), coord.GetLon());
}

void Tracker::onPositionMarked(const osmscout::GeoCoord coord, const QString& symbol, const QString& name)
{
  emit trackerPositionMarked(coord.GetLat(), coord.GetLon(), symbol, name);
}

TrackerModule::TrackerModule(QThread* thread, const QString& root)
: m_t(thread)
, m_baseDir()
, m_timer()
, m_state(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_azimuth(0)
, m_currentSpeed(0)
, m_maxSpeed(0)
, m_lastPosition()
, m_pinnedPosition()
, m_lastRecord()
, m_distance(0)
, m_duration(0)
, m_ascent(0)
, m_descent(0)
, m_recording(false)
, m_segment()
, m_lock()
, m_file(nullptr)
, m_log(nullptr)
, m_formater(nullptr)
, m_mark(nullptr)
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
  qInfo("%s", __FUNCTION__);
}

void TrackerModule::onTimeout()
{
}

void TrackerModule::onLocationChanged(bool positionValid, double lat, double lon, double alt)
{
  if (!positionValid)
    m_state = osmscout::PositionAgent::PositionState::NoGpsSignal;
  else
    m_state = osmscout::PositionAgent::PositionState::OffRoute;

  osmscout::Timestamp now = std::chrono::system_clock::now();
  osmscout::Bearing bearing;

  if (m_lastPosition)
  {
    double d = osmin::Utils::sphericalDistance(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon);
    auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(now - m_lastPosition.time);
    if (sec.count() > 0)
    {
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
          m_descent -= a;
        if (m_currentSpeed > m_maxSpeed)
          m_maxSpeed = m_currentSpeed;
      }
      emit dataChanged(m_currentSpeed, m_distance, m_duration, m_ascent, m_descent, m_maxSpeed);
    }
    // when we are stationary the direction is calculated according to the azimuth of the device
    // otherwise it is estimated according to the progress compared to the previous position
    if (m_currentSpeed < COURSE_SPEED)
      bearing = osmscout::Bearing::Degrees(m_azimuth);
    else
      bearing = osmscout::Bearing::Radians(osmin::Utils::sphericalBearingFinal(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon));

    if (m_recording)
    {
      if (!m_lastRecord)
      {
        record();
        m_lastRecord.time = m_lastPosition.time;
        m_lastRecord.coord = m_lastPosition.coord;
        m_lastRecord.bearing = m_lastPosition.bearing;
        emit positionRecorded(m_lastRecord.coord);
      }
      else
      {
        d = osmin::Utils::sphericalDistance(m_lastRecord.coord.GetLat(), m_lastRecord.coord.GetLon(), lat, lon);
        double da = std::fabs(bearing.AsRadians() - m_lastRecord.bearing.AsRadians());
        if (da > M_PI_2)
          da = fabs(da - M_PI);
        // 0.17rad ~ 20deg
        if (d >= RECORD_INTERVAL || (da > 0.17 && d > 5.0))
        {
          record();
          m_lastRecord.time = m_lastPosition.time;
          m_lastRecord.coord = m_lastPosition.coord;
          m_lastRecord.bearing = m_lastPosition.bearing;
          emit positionRecorded(m_lastRecord.coord);
        }
      }
    }
  }
  else
  {
    bearing = osmscout::Bearing::Degrees(m_azimuth);
  }

  m_lastPosition.bearing = bearing;
  m_lastPosition.coord.Set(lat, lon);
  m_lastPosition.elevation = alt;
  m_lastPosition.time = now;
  emit positionChanged(m_state, m_lastPosition.coord, std::optional<osmscout::Bearing>(m_lastPosition.bearing));
}

void TrackerModule::onAzimuthChanged(double degrees)
{
  m_azimuth = degrees;
  if (m_currentSpeed < COURSE_SPEED)
  {
    m_lastPosition.bearing = osmscout::Bearing::Degrees(m_azimuth);
    emit positionChanged(m_state, m_lastPosition.coord, std::optional<osmscout::Bearing>(m_lastPosition.bearing));
  }
}

void TrackerModule::onReset()
{
  m_distance = 0;
  m_duration = 0;
  m_ascent = 0;
  m_descent = 0;
  m_maxSpeed = 0;
  emit dataChanged(m_currentSpeed, m_distance, m_duration, m_ascent, m_descent, m_maxSpeed);
}

void TrackerModule::onStartRecording()
{
  qDebug("%s", __FUNCTION__);
  if (m_recording)
    onStopRecording();
  // prepare the new file to record data
  QString recordingName = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
  // log contains current state of the tracker, and this data will be restored upon recovery
  m_log.reset(new QFile(m_baseDir.absoluteFilePath(QString(recordingName).append(".log"))));
  // main file contains track data
  m_file.reset(new QFile(m_baseDir.absoluteFilePath(QString(recordingName).append(".csv"))));
  if (!m_log->open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text) &&
      !m_file->open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text))
  {
    emit recordingFailed();
  }
  else
  {
    onReset(); // reset state starting new recording
    m_log->close();
    m_file->close();
    m_recording = true;
    emit recordingChanged(recordingName);
  }
}

void TrackerModule::onResumeRecording(const QString& filename)
{
  if (m_recording)
    return;
  // register files of the previous session
  m_log.reset(new QFile(m_baseDir.absoluteFilePath(QString(filename).append(".log"))));
  m_file.reset(new QFile(m_baseDir.absoluteFilePath(QString(filename).append(".csv"))));
  if (!m_log->open(QIODevice::ReadWrite | QIODevice::Text) &&
      !m_file->open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text))
  {
    emit recordingChanged(QString());
    emit recordingFailed();
  }
  else
  {
    // restoring state from log
    for (;;)
    {
      QList<QByteArray*> row = m_formater->deserialize(m_log->readLine(0x3ff));
      if (row.length() == 0)
        break;
      else if (row.length() >= 5)
      {
        m_distance = QString::fromUtf8(row[0]->constData()).toDouble();
        m_duration = QString::fromUtf8(row[1]->constData()).toDouble();
        m_ascent = QString::fromUtf8(row[2]->constData()).toDouble();
        m_descent = QString::fromUtf8(row[3]->constData()).toDouble();
        m_maxSpeed = QString::fromUtf8(row[4]->constData()).toDouble();
      }
      qDeleteAll(row);
    }
    m_log->close();
    m_file->close();
    m_recording = true;
    emit recordingChanged(filename);
  }
}

void TrackerModule::onStopRecording()
{
  qDebug("%s", __FUNCTION__);
  onFlushRecording();
  QSharedPointer<QFile> file = m_file;
  m_recording = false;
  emit recordingChanged(QString());
  if (file.isNull() || !file->open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  emit processing(true);
  // fill gpx data
  std::vector<osmscout::gpx::Waypoint> waypoints;
  waypoints.reserve(20);
  osmscout::gpx::TrackSegment segment;
  segment.points.reserve(1000);
  size_t cp = 0, cwp = 0;
  for (;;)
  {
    QList<QByteArray*> row = m_formater->deserialize(file->readLine(0x3ff));
    if (row.length() == 0)
      break;
    else if (row.length() >= 6 && *row[0] == TAG_WAYPT)
    {
      if (++cwp > waypoints.size())
        waypoints.reserve(waypoints.size() + 20);
      osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1]->constData()).toDouble() * 1000))));
      double lat = QString::fromUtf8(row[2]->constData()).toDouble();
      double lon = QString::fromUtf8(row[3]->constData()).toDouble();
      double cse = QString::fromUtf8(row[4]->constData()).toDouble();
      double alt = QString::fromUtf8(row[5]->constData()).toDouble();
      osmscout::gpx::Waypoint waypoint(osmscout::GeoCoord(lat, lon));
      waypoint.timestamp = std::optional<osmscout::Timestamp>(ts);
      waypoint.course = std::optional<double>(cse);
      waypoint.elevation = std::optional<double>(alt);
      waypoint.symbol = std::optional<std::string>(row[6]->constData());
      waypoint.name = std::optional<std::string>(row[7]->constData());
      waypoint.description = std::optional<std::string>(row[8]->constData());
      waypoints.push_back(waypoint);
    }
    else if (row.length() >= 6 && *row[0] == TAG_TRKPT)
    {
      if (++cp > segment.points.size())
        segment.points.reserve(segment.points.size() + 1000);
      osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1]->constData()).toDouble() * 1000))));
      double lat = QString::fromUtf8(row[2]->constData()).toDouble();
      double lon = QString::fromUtf8(row[3]->constData()).toDouble();
      double cse = QString::fromUtf8(row[4]->constData()).toDouble();
      double alt = QString::fromUtf8(row[5]->constData()).toDouble();
      osmscout::gpx::TrackPoint point(osmscout::GeoCoord(lat, lon));
      point.timestamp = std::optional<osmscout::Timestamp>(ts);
      point.course = std::optional<double>(cse);
      point.elevation = std::optional<double>(alt);
      segment.points.emplace_back(point);
    }
    qDeleteAll(row);
  }
  file->close();
  // generate a simple GPX file
  if (cp > 0)
  {
    osmscout::gpx::Track track;
    track.segments.push_back(segment);
    track.name = std::optional<std::string>("track");
    osmscout::gpx::GpxFile gpx;
    gpx.tracks.push_back(track);
    gpx.desc = std::optional<std::string>("Generated by Osmin Traker");
    gpx.timestamp = std::optional<osmscout::Timestamp>(segment.points[0].timestamp.value());
    osmscout::gpx::Waypoint beg(segment.points[0].coord);
    beg.elevation = std::optional<double>(segment.points[0].elevation.value());
    beg.symbol = std::optional<std::string>("Flag, Red");
    beg.timestamp = std::optional<osmscout::Timestamp>(segment.points[0].timestamp.value());
    beg.name = std::optional<std::string>("[START]");
    osmscout::gpx::Waypoint end(segment.points[segment.points.size() - 1].coord);
    end.elevation = std::optional<double>(segment.points[segment.points.size() - 1].elevation.value());
    end.symbol = std::optional<std::string>("Flag, Red");
    end.timestamp = std::optional<osmscout::Timestamp>(segment.points[segment.points.size() - 1].timestamp.value());
    end.name = std::optional<std::string>("[END]");
    gpx.waypoints.push_back(beg);
    gpx.waypoints.push_back(end);
    gpx.waypoints.insert(gpx.waypoints.end(), waypoints.begin(), waypoints.end());
    QDateTime fdate = QDateTime();
    fdate.setMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(beg.timestamp.value().time_since_epoch()).count());
    gpx.name = std::optional<std::string>(fdate.toString(Qt::ISODate).toUtf8().toStdString());
    QString fname = fdate.toString("yyyy_MM_dd_hh_mm_ss").append(".gpx");
    if (!osmscout::gpx::ExportGpx(gpx, m_baseDir.absoluteFilePath(fname).toUtf8().toStdString()))
    {
      emit processing(false);
      emit recordingFailed();
      return;
    }
  }
  file->remove();
  if (!m_log.isNull())
    m_log->remove();
  emit processing(false);
}

void TrackerModule::onMarkPosition(const QString& symbol, const QString& name, const QString& description)
{
  qDebug("%s", __FUNCTION__);
  if (m_pinnedPosition.isNull())
    pinPosition();
  QScopedPointer<position_t> pos;
  pos.swap(m_pinnedPosition);
  // default an empty name
  QString _name(name);
  if (_name.isEmpty())
    _name.append('[').append(QString::fromStdString(pos->coord.GetDisplayText())).append(']');
  // save the mark
  if (!m_mark.isNull())
    onFlushRecording();
  osmscout::gpx::Waypoint* waypoint = new osmscout::gpx::Waypoint(pos->coord);
  waypoint->timestamp = std::optional<osmscout::Timestamp>(pos->time);
  waypoint->course = std::optional<double>(pos->bearing.AsDegrees());
  waypoint->elevation = std::optional<double>(pos->elevation);
  waypoint->symbol = std::optional<std::string>(symbol.toUtf8());
  waypoint->name = std::optional<std::string>(_name.toUtf8());
  waypoint->description = std::optional<std::string>(description.toUtf8());
  m_mark.reset(waypoint);
  onFlushRecording();
  emit positionMarked(pos->coord, symbol, name);
}

void TrackerModule::onFlushRecording()
{
  qDebug("%s", __FUNCTION__);
  QSharedPointer<QFile> file = m_file;
  if (!m_recording || file.isNull())
  {
    m_segment.clear();
    return;
  }
  // start critical section
  m_lock.lock();
  if (m_file->open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text))
  {
    // flush segment
    while (!m_segment.empty())
    {
      const auto p = m_segment.front();
      QList<QByteArray*> row;
      QString num;
      auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(p.timestamp.value().time_since_epoch());
      row << new QByteArray(TAG_TRKPT);
      row << new QByteArray(num.setNum(sec.count(), 'f', 3).toUtf8());
      row << new QByteArray(num.setNum(p.coord.GetLat(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(p.coord.GetLon(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(p.course.value(), 'f', 1).toUtf8());
      row << new QByteArray(num.setNum(p.elevation.value(), 'f', 1).toUtf8());
      row << new QByteArray("");
      QByteArray data = m_formater->serialize(row);
      qDeleteAll(row);
      data.append("\r\n");
      m_file->write(data);
      m_segment.pop_front();
    }
    // flush mark
    QScopedPointer<osmscout::gpx::Waypoint> mark(nullptr);
    mark.swap(m_mark);
    if (!mark.isNull())
    {
      QList<QByteArray*> row;
      QString num;
      auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(mark->timestamp.value().time_since_epoch());
      row << new QByteArray(TAG_WAYPT);
      row << new QByteArray(num.setNum(sec.count(), 'f', 3).toUtf8());
      row << new QByteArray(num.setNum(mark->coord.GetLat(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(mark->coord.GetLon(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(mark->course.value(), 'f', 1).toUtf8());
      row << new QByteArray(num.setNum(mark->elevation.value(), 'f', 1).toUtf8());
      row << new QByteArray(mark->symbol.value_or(MARK_SYMBOL).data());
      row << new QByteArray(mark->name.value_or("").data());
      row << new QByteArray(mark->description.value_or("").data());
      row << new QByteArray("");
      QByteArray data = m_formater->serialize(row);
      qDeleteAll(row);
      data.append("\r\n");
      m_file->write(data);
    }
    // all data are flushed
    m_file->close();
  }
  // flush log
  if (m_log->open(QIODevice::Truncate| QIODevice::WriteOnly | QIODevice::Text))
  {
    QList<QByteArray*> row;
    QString num;
    row << new QByteArray(num.setNum(m_distance, 'f', 3).toUtf8());
    row << new QByteArray(num.setNum(m_duration, 'f', 3).toUtf8());
    row << new QByteArray(num.setNum(m_ascent, 'f', 3).toUtf8());
    row << new QByteArray(num.setNum(m_descent, 'f', 3).toUtf8());
    row << new QByteArray(num.setNum(m_maxSpeed, 'f', 3).toUtf8());
    row << new QByteArray("");
    QByteArray data = m_formater->serialize(row);
    qDeleteAll(row);
    data.append("\r\n");
    m_log->write(data);
    // log is flushed
    m_log->close();
  }
  m_lock.unlock();
  // end critical section
}

void TrackerModule::onDumpRecording()
{
  qDebug("%s", __FUNCTION__);
  QSharedPointer<QFile> file = m_file;
  if (file.isNull())
    return;
  // start critical section
  m_lock.lock();
  if (file->open(QIODevice::ReadOnly | QIODevice::Text))
  {
    // dump stored data
    for (;;)
    {
      QList<QByteArray*> row = m_formater->deserialize(file->readLine(0x3ff));
      if (row.length() == 0)
        break;
      else if (row.length() >= 6 && *row[0] == TAG_WAYPT)
      {
        osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1]->constData()).toDouble() * 1000))));
        double lat = QString::fromUtf8(row[2]->constData()).toDouble();
        double lon = QString::fromUtf8(row[3]->constData()).toDouble();
        QString symbol(row[6]->constData());
        QString name(row[7]->constData());
        emit positionMarked(osmscout::GeoCoord(lat, lon), symbol, name);
      }
      else if (row.length() >= 6 && *row[0] == TAG_TRKPT)
      {
        osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1]->constData()).toDouble() * 1000))));
        double lat = QString::fromUtf8(row[2]->constData()).toDouble();
        double lon = QString::fromUtf8(row[3]->constData()).toDouble();
        emit positionRecorded(osmscout::GeoCoord(lat, lon));
      }
      qDeleteAll(row);
    }
    file->close();
  }
  // dump in memory data
  for (auto& p : m_segment)
    emit positionRecorded(p.coord);
  if (!m_mark.isNull())
    emit positionMarked(m_mark->coord, QString::fromUtf8(m_mark->symbol.value_or(MARK_SYMBOL).data()), QString::fromUtf8(m_mark->name.value_or("").data()));
  m_lock.unlock();
  // end critical section
}

void TrackerModule::record()
{
  if (m_segment.count() >= SEGMENT_SIZE)
    onFlushRecording();
  osmscout::gpx::TrackPoint point(m_lastPosition.coord);
  point.timestamp = std::optional<osmscout::Timestamp>(m_lastPosition.time);
  point.course = std::optional<double>(m_lastPosition.bearing.AsDegrees());
  point.elevation = std::optional<double>(m_lastPosition.elevation);
  m_segment.append(point);
}

void TrackerModule::pinPosition()
{
  m_pinnedPosition.reset(new position_t(m_lastPosition));
}
