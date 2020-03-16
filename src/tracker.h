#ifndef TRACKER_H
#define TRACKER_H

#include "csvparser.h"

#include <osmscout/VehiclePosition.h>
#include <osmscout/RouteStep.h>
#include <osmscout/gpx/TrackPoint.h>

#include <QObject>
#include <QDateTime>
#include <QThread>
#include <QTimer>
#include <QSharedPointer>
#include <QDir>
#include <QFile>

class TrackerModule;

class Tracker : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QObject* trackerPosition READ getTrackerPosition NOTIFY trackerPositionChanged)
  Q_PROPERTY(double currentSpeed READ getCurrentSpeed NOTIFY trackerDataChanged)
  Q_PROPERTY(double distance READ getDistance NOTIFY trackerDataChanged)
  Q_PROPERTY(double duration READ getDuration NOTIFY trackerDataChanged)
  Q_PROPERTY(QString recording READ getRecording WRITE setRecording NOTIFY trackerRecordingChanged)
  //Q_PROPERTY(double remainingDistance READ getRemainingDistance NOTIFY remainingDistanceChanged)
  //Q_PROPERTY(QObject* nextRouteStep READ getNextRoutStep NOTIFY nextStepChanged)

public:
  Tracker(QObject* parent = nullptr);
  virtual ~Tracker();

  bool init(const QString& root);

  osmscout::VehiclePosition* getTrackerPosition() const;

  double getCurrentSpeed() const { return m_currentSpeed; }
  double getDistance() const { return m_distance; }
  double getDuration() const { return m_duration; }
  QString getRecording() const { return m_recording; }
  void setRecording(const QString& filename);

  Q_INVOKABLE void locationChanged(bool positionValid, double lat, double lon, bool horizontalAccuracyValid, double horizontalAccuracy);
  Q_INVOKABLE void azimuthChanged(double azimuth);
  Q_INVOKABLE void reset();
  Q_INVOKABLE void startRecording();
  Q_INVOKABLE void resumeRecording(const QString& filename);
  Q_INVOKABLE void stopRecording();

signals:
  void trackerPositionChanged();
  void trackerDataChanged();
  void trackerRecordingChanged();
  void recordingFailed();
  //void remainingDistanceChanged();
  //void nextStepChanged();

  void locationUpdate(bool positionValid, double lat, double lon);
  void azimuthUpdate(double degrees);
  void doReset();
  void doStartRecording();
  void doResumeRecording(const QString& filename);
  void doStopRecording();

private slots:
  void onPositionChanged(const osmscout::PositionAgent::PositionState state,
                         const osmscout::GeoCoord coord,
                         const std::shared_ptr<osmscout::Bearing> bearing);
  void onDataChanged(double kmph, double meters, double seconds);
  void onRecordingChanged(const QString& filename);

private:
  TrackerModule* m_p;

  osmscout::Vehicle m_vehicle;
  osmscout::PositionAgent::PositionState m_vehicleState;
  osmscout::GeoCoord m_vehicleCoord;
  std::shared_ptr<osmscout::Bearing> m_vehicleBearing;
  double m_currentSpeed;
  double m_distance;
  double m_duration;
  QString m_recording;
  //std::vector<osmscout::RouteStep> m_routeSteps;
  //osmscout::RouteStep m_nextRouteStep;
  //osmscout::Distance m_remainingDistance;
};

class TrackerModule : public QObject
{
  Q_OBJECT
public:
  TrackerModule(QThread* thread, const QString& root);
  virtual ~TrackerModule();

  void record();

signals:
  void dataChanged(double kmph, double distance, double duration);
  void positionChanged(const osmscout::PositionAgent::PositionState state,
                       const osmscout::GeoCoord coord,
                       const std::shared_ptr<osmscout::Bearing> bearing);
  void recordingFailed();
  void recordingChanged(const QString& filename);

public slots:
  void onTimeout();
  void onLocationChanged(bool positionValid, double lat, double lon);
  void onAzimuthChanged(double degrees);
  void onReset();
  void onStartRecording();
  void onResumeRecording(const QString& filename);
  void onStopRecording();
  void onFlushRecording();

private:
  QThread* m_t;
  QDir m_baseDir;
  QTimer m_timer;
  osmscout::PositionAgent::PositionState m_state;
  double m_azimuth;
  double m_currentSpeed;
  struct {
    osmscout::Timestamp time;
    osmscout::GeoCoord coord;
    osmscout::Bearing bearing;
    double elevation;
    inline operator bool() const { return time.time_since_epoch() != osmscout::Timestamp::duration::zero(); }
  } m_lastPosition;

  double m_distance;
  double m_duration;

  bool m_recording;
  QList<osmscout::gpx::TrackPoint> m_segment;
  QSharedPointer<QFile> m_file;
  osmin::CSVParser* m_formater;
};

#endif // TRACKER_H
