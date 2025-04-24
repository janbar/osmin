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
#define ASC_THRESHOLD   (10.0)      /* threshold in meter */
#define DES_THRESHOLD   (-10.0)     /* threshold in meter */
#define MAX_ACCEL       12.0        /* accel in meter per sec^2 */

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

double Tracker::getBearing() const
{
  if (m_vehicleBearing.has_value())
    return m_vehicleBearing->AsRadians();
  return 0.0;
}

void Tracker::setRecording(const QString& filename)
{
  emit doResumeRecording(filename);
}

bool Tracker::getIsRecording() const
{
  return (m_p ? m_p->isRecording() : false);
}

double Tracker::getMagneticDip() const
{
  return (m_p ? m_p->magneticDip() : 0.0);
}

void Tracker::setMagneticDip(double magneticDip)
{
  if (m_p)
    m_p->setMagneticDip(magneticDip);
  emit trackerMagneticDipChanged();
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
, m_state(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_magneticDip(0.0)
, m_azimuth(0.0)
, m_currentAlt(0)
, m_currentSpeed(0)
, m_maxSpeed(0)
, m_lastPosition()
, m_pinnedPosition()
, m_lastRecord()
, m_distance(0)
, m_duration(0)
, m_originAlt(0)
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
}

TrackerModule::~TrackerModule()
{
  onFlushRecording();
  if (m_t)
    m_t->quit();
  delete m_formater;
  qInfo("%s", __FUNCTION__);
}

void TrackerModule::setMagneticDip(double magneticDip)
{
  // rollback dip before signal
  double degrees = ::remainder(m_azimuth - m_magneticDip, 360.0);
  m_magneticDip = magneticDip;
  onAzimuthChanged((degrees < 0 ? degrees + 360.0 : degrees));
}

void TrackerModule::onLocationChanged(bool positionValid, double lat, double lon, double alt)
{
  if (!positionValid)
    m_state = osmscout::PositionAgent::PositionState::NoGpsSignal;
  else
    m_state = osmscout::PositionAgent::PositionState::OffRoute;

  position_t now = {
    std::chrono::system_clock::now(),
    osmscout::GeoCoord(lat, lon),
    osmscout::Bearing(),
    alt,
    0.0
  };

  // compute data for the last interval,
  // starting with the last known position and up to now
  if (!m_lastPosition || !positionValid)
  {
    // On the first run, initialize the starting data
    now.bearing = osmscout::Bearing::Degrees(m_azimuth);
    m_currentAlt = alt; // could be nan
    m_originAlt = NAN;
    m_currentSpeed = 0.0;
  }
  else
  {
    // the duration should be non zero
    auto duration = std::chrono::duration_cast<std::chrono::duration<double> >(now.time - m_lastPosition.time);
    if (duration.count() > 0)
    {
      // i use accel to validate the new interval: data won't be updated when value is excessive
      // the vertical accuracy isn't good enough, therefore i estimate the move using horizontal data only
      double dh = osmin::Utils::sphericalDistance(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon);
      double sh = dh / duration.count();
      double accelh = std::fabs((sh - (m_lastPosition.speed / 3.6)) / duration.count());
      now.speed = 3.6 * sh;

      if (accelh < MAX_ACCEL)
      {
        // update tracking speed when moving
        m_currentSpeed = now.speed;
        if (now.speed > MOVING_SPEED)
        {
          m_distance += dh;
          m_duration += duration.count();
          if (now.speed > m_maxSpeed)
            m_maxSpeed = now.speed;
        }

        // update tracking elevation when moving
        if (!std::isnan(alt) && !std::isnan(m_lastPosition.elevation))
        {
          // hack to validate vertical deviation
          if (dh > std::fabs(alt - m_lastPosition.elevation))
          {
            // update current elevation
            m_currentAlt = alt;

            // initialize origin as needed
            if (std::isnan(m_originAlt))
              m_originAlt = alt;
            else
            {
              // update statistics when threshold is reached compared to origin
              double dv = alt - m_originAlt;
              if (dv > ASC_THRESHOLD)
              {
                m_originAlt = alt;
                m_ascent += dv;
              }
              else if (dv < DES_THRESHOLD)
              {
                m_originAlt = alt;
                m_descent -= dv;
              }
            }
          }
        }

        emit dataChanged(m_currentSpeed, m_distance, m_duration, m_ascent, m_descent, m_maxSpeed);

        // when we are stationary the direction is calculated according to the azimuth of the device
        // otherwise it is estimated according to the progress compared to the previous position
        if (now.speed < COURSE_SPEED)
          now.bearing = osmscout::Bearing::Degrees(m_azimuth);
        else
          now.bearing = osmscout::Bearing::Radians(osmin::Utils::sphericalBearingFinal(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon));
      }
      else
      {
        // deviation is out of bounds !!!
        // keep previous direction
        now.bearing = m_lastPosition.bearing;
      }
    }
    else
    {
      // on zero duration, the deviation is null
      now.speed = m_lastPosition.speed;
      now.bearing = m_lastPosition.bearing;
    }

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
        double dh = osmin::Utils::sphericalDistance(m_lastRecord.coord.GetLat(), m_lastRecord.coord.GetLon(), lat, lon);
        double da = std::fabs(now.bearing.AsRadians() - m_lastRecord.bearing.AsRadians());
        if (da > M_PI_2)
          da = std::fabs(da - M_PI);
        // 0.17rad ~ 20deg
        if (dh >= RECORD_INTERVAL || (da > 0.17 && dh > 5.0))
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

  // save last position
  m_lastPosition = now;
  emit positionChanged(m_state, now.coord, std::optional<osmscout::Bearing>(now.bearing));
}

void TrackerModule::onAzimuthChanged(double degrees)
{
  double angle = ::remainder(degrees + m_magneticDip, 360.0);
  m_azimuth = (angle < 0 ? angle + 360.0 : angle);
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
      osmin::CSVParser::container row;
      m_formater->deserialize(row, m_log->readLine(0x3ff).toStdString());
      if (row.size() == 0)
        break;
      else if (row.size() >= 5)
      {
        m_distance = QString::fromUtf8(row[0].c_str()).toDouble();
        m_duration = QString::fromUtf8(row[1].c_str()).toDouble();
        m_ascent = QString::fromUtf8(row[2].c_str()).toDouble();
        m_descent = QString::fromUtf8(row[3].c_str()).toDouble();
        m_maxSpeed = QString::fromUtf8(row[4].c_str()).toDouble();
      }
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
    osmin::CSVParser::container row;
    bool next = m_formater->deserialize(row, file->readLine(0x3ff).toStdString());
    // paranoia: on corruption the last field could overflow
    while (next && !file->atEnd() && row.back().size() < 0x3ff)
    {
      // the row continue with next line
      next = m_formater->deserialize_next(row, file->readLine(0x3ff).toStdString());
    }
    if (row.size() == 0)
      break;
    else if (row.size() >= 6 && row[0] == TAG_WAYPT)
    {
      if (++cwp > waypoints.size())
        waypoints.reserve(waypoints.size() + 20);
      osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1].c_str()).toDouble() * 1000))));
      double lat = QString::fromUtf8(row[2].c_str()).toDouble();
      double lon = QString::fromUtf8(row[3].c_str()).toDouble();
      double cse = QString::fromUtf8(row[4].c_str()).toDouble();
      double alt = QString::fromUtf8(row[5].c_str()).toDouble();
      osmscout::gpx::Waypoint waypoint(osmscout::GeoCoord(lat, lon));
      waypoint.timestamp = std::optional<osmscout::Timestamp>(ts);
      if (!std::isnan(cse))
        waypoint.course = std::optional<double>(cse);
      if (!std::isnan(alt))
        waypoint.elevation = std::optional<double>(alt);
      waypoint.symbol = std::optional<std::string>(row[6].c_str());
      waypoint.name = std::optional<std::string>(row[7].c_str());
      waypoint.description = std::optional<std::string>(row[8].c_str());
      waypoints.push_back(waypoint);
    }
    else if (row.size() >= 6 && row[0] == TAG_TRKPT)
    {
      if (++cp > segment.points.size())
        segment.points.reserve(segment.points.size() + 1000);
      osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1].c_str()).toDouble() * 1000))));
      double lat = QString::fromUtf8(row[2].c_str()).toDouble();
      double lon = QString::fromUtf8(row[3].c_str()).toDouble();
      double cse = QString::fromUtf8(row[4].c_str()).toDouble();
      double alt = QString::fromUtf8(row[5].c_str()).toDouble();
      osmscout::gpx::TrackPoint point(osmscout::GeoCoord(lat, lon));
      point.timestamp = std::optional<osmscout::Timestamp>(ts);
      if (!std::isnan(cse))
        point.course = std::optional<double>(cse);
      if (!std::isnan(alt))
        point.elevation = std::optional<double>(alt);
      segment.points.emplace_back(point);
    }
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
    gpx.timestamp = segment.points[0].timestamp;
    osmscout::gpx::Waypoint beg(segment.points[0].coord);
    beg.elevation = segment.points[0].elevation;
    beg.symbol = std::optional<std::string>("Flag, Red");
    beg.timestamp = segment.points[0].timestamp;
    beg.name = std::optional<std::string>("[START]");
    osmscout::gpx::Waypoint end(segment.points[segment.points.size() - 1].coord);
    end.elevation = segment.points[segment.points.size() - 1].elevation;
    end.symbol = std::optional<std::string>("Flag, Red");
    end.timestamp = segment.points[segment.points.size() - 1].timestamp;
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
      osmin::CSVParser::container row;
      QString num;
      auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(p.timestamp.value().time_since_epoch());
      row.push_back(std::string(TAG_TRKPT));
      row.push_back(num.setNum(sec.count(), 'f', 3).toStdString());
      row.push_back(num.setNum(p.coord.GetLat(), 'f', 6).toStdString());
      row.push_back(num.setNum(p.coord.GetLon(), 'f', 6).toStdString());
      row.push_back(num.setNum(p.course.value(), 'f', 1).toStdString());
      row.push_back(num.setNum(p.elevation.value(), 'f', 1).toStdString());
      row.push_back(std::string(""));
      std::string data;
      m_formater->serialize(data, row);
      data.append("\r\n");
      m_file->write(data.c_str());
      m_segment.pop_front();
    }
    // flush mark
    QScopedPointer<osmscout::gpx::Waypoint> mark(nullptr);
    mark.swap(m_mark);
    if (!mark.isNull())
    {
      osmin::CSVParser::container row;
      QString num;
      auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(mark->timestamp.value().time_since_epoch());
      row.push_back(std::string(TAG_WAYPT));
      row.push_back(num.setNum(sec.count(), 'f', 3).toStdString());
      row.push_back(num.setNum(mark->coord.GetLat(), 'f', 6).toStdString());
      row.push_back(num.setNum(mark->coord.GetLon(), 'f', 6).toStdString());
      row.push_back(num.setNum(mark->course.value(), 'f', 1).toStdString());
      row.push_back(num.setNum(mark->elevation.value(), 'f', 1).toStdString());
      row.push_back(mark->symbol.value_or(MARK_SYMBOL).data());
      row.push_back(mark->name.value_or("").data());
      row.push_back(mark->description.value_or("").data());
      row.push_back(std::string(""));
      std::string data;
      m_formater->serialize(data, row);
      data.append("\r\n");
      m_file->write(data.c_str());
    }
    // all data are flushed
    m_file->close();
  }
  // flush log
  if (m_log->open(QIODevice::Truncate| QIODevice::WriteOnly | QIODevice::Text))
  {
    osmin::CSVParser::container row;
    QString num;
    row.push_back(num.setNum(m_distance, 'f', 3).toStdString());
    row.push_back(num.setNum(m_duration, 'f', 3).toStdString());
    row.push_back(num.setNum(m_ascent, 'f', 3).toStdString());
    row.push_back(num.setNum(m_descent, 'f', 3).toStdString());
    row.push_back(num.setNum(m_maxSpeed, 'f', 3).toStdString());
    row.push_back("");
    std::string data;
    m_formater->serialize(data, row);
    data.append("\r\n");
    m_log->write(data.c_str());
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
      osmin::CSVParser::container row;
      bool next = m_formater->deserialize(row, file->readLine(0x3ff).toStdString());
      // paranoia: on corruption the last field could overflow
      while (next && !file->atEnd() && row.back().size() < 0x3ff)
      {
        // the row continue with next line
        next = m_formater->deserialize_next(row, file->readLine(0x3ff).toStdString());
      }
      if (row.size() == 0)
        break;
      else if (row.size() >= 6 && row[0] == TAG_WAYPT)
      {
        osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1].c_str()).toDouble() * 1000))));
        double lat = QString::fromUtf8(row[2].c_str()).toDouble();
        double lon = QString::fromUtf8(row[3].c_str()).toDouble();
        QString symbol(row[6].c_str());
        QString name(row[7].c_str());
        emit positionMarked(osmscout::GeoCoord(lat, lon), symbol, name);
      }
      else if (row.size() >= 6 && row[0] == TAG_TRKPT)
      {
        osmscout::Timestamp ts(std::chrono::milliseconds(static_cast<qint64>(std::round(QString::fromUtf8(row[1].c_str()).toDouble() * 1000))));
        double lat = QString::fromUtf8(row[2].c_str()).toDouble();
        double lon = QString::fromUtf8(row[3].c_str()).toDouble();
        emit positionRecorded(osmscout::GeoCoord(lat, lon));
      }
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
