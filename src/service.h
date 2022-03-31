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

class BuiltInCompass;
class BuiltInSensorPlugin;
class QSensorBackend;

class Service : public ServiceMessengerSource
{
  Q_OBJECT
public:
  Service(const QString& url, const QString& rootDir);
  virtual ~Service();

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

private slots:
  void onCompassReadingChanged();
  void onCompassActiveChanged();
  void onCompassDataRateChanged();

  void onPositionPositionUpdated(QGeoPositionInfo info);
  void onPositionUpdateIntervalChanged();
  void onPositionSupportedPositioningMethodsChanged();

  void onTrackerRecordingChanged();
  void onTrackerProcessingChanged();
  void onTrackerPositionRecorded(double lat, double lon);
  void onTrackerPositionMarked(double lat, double lon, const QString& symbol, const QString& name);
  void onTrackerPositionChanged();
  void onTrackerRecordingFailed();
  void onTrackerDataChanged();

private:
  QAtomicInt m_run;
  QString m_url;
  QString m_rootDir;
  QSettings m_settings;
  BuiltInCompass * m_compass;
  BuiltInSensorPlugin * m_sensor;
  QSensorBackend * m_SB;

  QGeoPositionInfoSource * m_position;
  bool m_positionActive = false;

  QRemoteObjectHost * m_node = nullptr;
  Tracker * m_tracker = nullptr;

  QElapsedTimer m_pollTimeout;
};

#endif // SERVICE_H
