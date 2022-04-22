/*
 * Copyright (C) 2022
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

#include "service.h"
#include "compass/plugin.h"

#include <QCompass>
#include <QGeoPositionInfoSource>

Service::Service(const QString& url, const QString& rootDir)
: m_url(url)
, m_rootDir(rootDir)
{
  m_pollTimeout.start();
  // register the generic compass
  m_compass = new BuiltInCompass();
  m_sensor = new BuiltInSensorPlugin();
  m_sensor->registerSensors();
  m_SB = m_sensor->createBackend(m_compass);
  // register the position source
  m_position = QGeoPositionInfoSource::createDefaultSource(this);
}

Service::~Service()
{
  if (m_node)
  {
    m_node->disableRemoting(this);
    delete m_tracker;
    delete m_node;
  }
  if (m_position)
    delete m_position;
  delete m_SB;
  delete m_sensor;
  delete m_compass;
  qInfo("%s", __FUNCTION__);
}

void Service::run()
{
  if (m_run.fetchAndAddAcquire(1) != 0)
    return;

  m_tracker = new Tracker();
  m_tracker->init(m_rootDir);
  connect(m_tracker, &Tracker::trackerProcessingChanged, this, &Service::onTrackerProcessingChanged);
  connect(m_tracker, &Tracker::trackerRecordingChanged, this, &Service::onTrackerRecordingChanged);
  connect(m_tracker, &Tracker::trackerDataChanged, this, &Service::onTrackerDataChanged);
  connect(m_tracker, &Tracker::trackerPositionRecorded, this, &Service::onTrackerPositionRecorded);
  connect(m_tracker, &Tracker::trackerPositionMarked, this, &Service::onTrackerPositionMarked);
  connect(m_tracker, &Tracker::trackerPositionChanged, this, &Service::onTrackerPositionChanged);
  connect(m_tracker, &Tracker::recordingFailed, this, &Service::onTrackerRecordingFailed);

  m_node = new QRemoteObjectHost(QUrl(m_url));
  m_node->enableRemoting(this);

  connect(m_compass, &BuiltInCompass::readingChanged, this, &Service::onCompassReadingChanged, Qt::QueuedConnection);
  connect(this, &Service::compass_azimuthChanged, m_tracker, &Tracker::azimuthChanged);
  m_compass->setDataRate(COMPASS_DATARATE);
  m_SB->start();

  if (m_position)
  {
    qDebug("Position info source is available: %s", m_position->sourceName().toUtf8().constData());
    connect(m_position, &QGeoPositionInfoSource::positionUpdated, this, &Service::onPositionPositionUpdated);
    connect(this, &Service::position_positionUpdated, m_tracker, &Tracker::locationChanged);
    QGeoPositionInfoSource::PositioningMethods m;
    m.setFlag(QGeoPositionInfoSource::SatellitePositioningMethods);
    m_position->setPreferredPositioningMethods(m);
    m_position->setUpdateInterval(POSITION_UPDATE_INTERVAL);
    m_positionActive = true;
    emit position_activeChanged(m_positionActive);
    m_position->startUpdates();
  }

  // resume current recording
  QString setting = m_settings.value(SETTING_RECORDING_FILENAME).toString();
  if (setting.length() > 0)
  {
    emit tracker_resumeRecording();
    m_tracker->resumeRecording(setting);
  }
}

void Service::ping(const QString &message)
{
  onTrackerRecordingChanged();
  onTrackerProcessingChanged();
  onTrackerDataChanged();
  onPositionUpdateIntervalChanged();
  onPositionPreferredPositioningMethodsChanged();
  onCompassActiveChanged();
  onCompassDataRateChanged();
  onCompassReadingChanged();
  emit position_activeChanged(m_positionActive);
  onPositionPositionUpdated(m_position->lastKnownPosition());
  if (message == "ALL")
    m_tracker->dumpRecording();
  emit pong(message);
}

void Service::compass_setActive(bool active)
{
  m_compass->setActive(active);
}

void Service::compass_setDataRate(int rate)
{
  m_compass->setDataRate(rate);
  onCompassDataRateChanged();
}

void Service::position_setUpdateInterval(int interval)
{
  if (m_position == nullptr)
    return;
  if (interval < m_position->minimumUpdateInterval())
    interval = m_position->minimumUpdateInterval();
  m_position->setUpdateInterval(interval);
  onPositionUpdateIntervalChanged();
}

void Service::position_setPreferredPositioningMethods(int methods)
{
  if (m_position == nullptr)
    return;
  QGeoPositionInfoSource::PositioningMethods m;
  switch (methods)
  {
  case NoPositioningMethods:
    m.setFlag(QGeoPositionInfoSource::NoPositioningMethods);
    break;
  case SatellitePositioningMethods:
    m.setFlag(QGeoPositionInfoSource::SatellitePositioningMethods);
    break;
  case NonSatellitePositioningMethods:
    m.setFlag(QGeoPositionInfoSource::NonSatellitePositioningMethods);
    break;
  case AllPositioningMethods:
    m.setFlag(QGeoPositionInfoSource::AllPositioningMethods);
    break;
  }
  m_position->setPreferredPositioningMethods(m);
  onPositionPreferredPositioningMethodsChanged();
}

void Service::position_startUpdates()
{
  if (m_position != nullptr)
  {
    m_position->startUpdates();
    m_positionActive = true;
    emit position_activeChanged(m_positionActive);
  }
}

void Service::position_stopUpdates()
{
  if (m_position != nullptr)
  {
    m_position->stopUpdates();
    m_positionActive = false;
    emit position_activeChanged(m_positionActive);
  }
}

void Service::tracker_setRecording(const QString &filename)
{
  m_tracker->setRecording(filename);
}

void Service::tracker_startRecording()
{
  m_tracker->startRecording();
}

void Service::tracker_stopRecording()
{
  m_tracker->stopRecording();
}

void Service::tracker_pinPosition()
{
  m_tracker->pinPosition();
}

void Service::tracker_markPosition(const QString &symbol, const QString &name, const QString &description)
{
  m_tracker->markPosition(symbol, name, description);
}

void Service::tracker_resetData()
{
  m_tracker->reset();
}

void Service::onCompassReadingChanged()
{
  if (!m_pollTimeout.hasExpired(COMPASS_MIN_INTERVAL))
    return;
  m_pollTimeout.restart();
  if (m_compass->reading()->valueCount() == 2)
  {
    emit compass_readingChanged(
          m_compass->reading()->value(0).toFloat(),
          m_compass->reading()->value(1).toFloat()
    );
    // signal azimuth
    emit compass_azimuthChanged(m_compass->reading()->value(0).toDouble());
  }
}

void Service::onCompassActiveChanged()
{
  emit compass_activeChanged(m_compass->isActive());
}

void Service::onCompassDataRateChanged()
{
  emit compass_dataRateChanged(m_compass->dataRate());
}

void Service::onPositionPositionUpdated(QGeoPositionInfo info)
{
  bool pvalid = info.coordinate().isValid();
  double lat = info.coordinate().latitude();
  double lon = info.coordinate().longitude();
  double alt = info.coordinate().altitude();
  bool hvalid = info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy);
  float hacc = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
  emit position_positionUpdated(pvalid, lat, lon, hvalid, hacc, alt);
}

void Service::onPositionUpdateIntervalChanged()
{
  emit position_updateIntervalChanged(m_position->updateInterval());
}

void Service::onPositionPreferredPositioningMethodsChanged()
{
  QGeoPositionInfoSource::PositioningMethods m = m_position->preferredPositioningMethods();
  int methods = NoPositioningMethods;
  if (m.testFlag(QGeoPositionInfoSource::AllPositioningMethods))
    methods = AllPositioningMethods;
  else if (m.testFlag(QGeoPositionInfoSource::SatellitePositioningMethods))
    methods = SatellitePositioningMethods;
  else if (m.testFlag(QGeoPositionInfoSource::NonSatellitePositioningMethods))
    methods = NonSatellitePositioningMethods;
  emit position_preferredPositioningMethodsChanged(methods);
}

void Service::onTrackerRecordingChanged()
{
  QString recording = m_tracker->getRecording();
  QString setting = m_settings.value(SETTING_RECORDING_FILENAME).toString();
  if (setting != recording)
    m_settings.setValue(SETTING_RECORDING_FILENAME, QVariant::fromValue(recording));
  emit tracker_recordingChanged(recording);
  emit tracker_isRecordingChanged(m_tracker->getIsRecording());
}

void Service::onTrackerProcessingChanged()
{
  emit tracker_processingChanged(m_tracker->getProcessing());
}

void Service::onTrackerPositionRecorded(double lat, double lon)
{
  emit tracker_positionRecorded(lat, lon);
}

void Service::onTrackerPositionMarked(double lat, double lon, const QString &symbol, const QString &name)
{
  emit tracker_positionMarked(lat, lon, symbol, name);
}

void Service::onTrackerPositionChanged()
{
  osmscout::VehiclePosition * pos = m_tracker->getTrackerPosition();
  bool valid = (pos->getState() == osmscout::PositionAgent::PositionState::NoGpsSignal ? false : true);
  emit tracker_positionChanged(valid, pos->getLat(), pos->getLon(), pos->getBearingRadians());
  delete pos;
}

void Service::onTrackerRecordingFailed()
{
  emit tracker_recordingFailed();
}

void Service::onTrackerDataChanged()
{
  emit tracker_dataChanged(
        m_tracker->getElevation(),
        m_tracker->getCurrentSpeed(),
        m_tracker->getDistance(),
        m_tracker->getDuration(),
        m_tracker->getAscent(),
        m_tracker->getDescent(),
        m_tracker->getMaxSpeed());
}
