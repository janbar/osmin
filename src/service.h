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
#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QThread>
#include <QAtomicInt>
#include <QSettings>
#include <QGeoPositionInfoSource>
#include <QElapsedTimer>

#include "rep_servicemessenger_source.h"
#include "tracker.h"

#define SETTING_RECORDING_FILENAME      "trackerRecording"
#define COMPASS_DATARATE          2     // default
#define COMPASS_MIN_INTERVAL      250   // 250 ms
#define POSITION_UPDATE_INTERVAL  1000  // 1 sec

QT_BEGIN_NAMESPACE
class QSensor;
class QSensorBackend;
QT_END_NAMESPACE

class BuiltInSensorPlugin;

class Service : public ServiceMessengerSource
{
  Q_OBJECT
public:
  /**
   * @brief Create Service with builtin data source
   * @param url
   * @param rootDir
   */
  Service(const QString& url, const QString& rootDir);

  /**
   * @brief Create Service using the given data source
   * @param url
   * @param rootDir
   * @param compassSource
   * @param positionSource
   */
  Service(const QString& url, const QString& rootDir
          , QSensor * compassSource
          , QGeoPositionInfoSource * positionSource);

  virtual ~Service();

  enum PositioningMethods
  {
    NoPositioningMethods            = 0,
    SatellitePositioningMethods     = 1,
    NonSatellitePositioningMethods  = 2,
    AllPositioningMethods           = 3,
  };

signals:
  void compass_azimuthChanged(double azimuth);

public slots:
  void run();

  void ping(const QString &message) override;

  void compass_setActive(bool active) override;
  void compass_setDataRate(int dataRate) override;

  void position_setUpdateInterval(int interval) override;
  void position_setPreferredPositioningMethods(int methods) override;
  void position_startUpdates() override;
  void position_stopUpdates() override;

  void tracker_setRecording(const QString& filename) override;
  void tracker_startRecording() override;
  void tracker_stopRecording() override;
  void tracker_pinPosition() override;
  void tracker_markPosition(const QString& symbol, const QString& name, const QString& description) override;
  void tracker_resetData() override;
  void tracker_setMagneticDip(double magneticDip) override;

private slots:
  void onCompassReadingChanged();
  void onCompassActiveChanged();
  void onCompassDataRateChanged();

  void onPositionPositionUpdated(QGeoPositionInfo info);
  void onPositionUpdateIntervalChanged();
  void onPositionPreferredPositioningMethodsChanged();

  void onTrackerRecordingChanged();
  void onTrackerProcessingChanged();
  void onTrackerPositionRecorded(double lat, double lon);
  void onTrackerPositionMarked(double lat, double lon, const QString& symbol, const QString& name);
  void onTrackerPositionChanged();
  void onTrackerRecordingFailed();
  void onTrackerDataChanged();
  void onTrackerMagneticDipChanged();

private:
  QAtomicInt m_run;
  QString m_url;
  QString m_rootDir;
  QSettings m_settings;
  QSensor * m_compass;
  BuiltInSensorPlugin * m_sensor;

  QGeoPositionInfoSource * m_position;
  bool m_positionActive = false;

  QRemoteObjectHost * m_node = nullptr;
  Tracker * m_tracker = nullptr;

  QElapsedTimer m_pollTimeout;
};

#endif // SERVICE_H
