#ifndef TRACKER_H
#define TRACKER_H

#include <osmscout/VehiclePosition.h>
#include <osmscout/RouteStep.h>

#include <QObject>
#include <QDateTime>
#include <QThread>
#include <QTimer>

class TrackerModule;

class Tracker : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QObject* trackerPosition   READ getTrackerPosition     NOTIFY trackerPositionChanged)
  Q_PROPERTY(double currentSpeed        READ getCurrentSpeed        NOTIFY trackerDataChanged)
  Q_PROPERTY(double distance            READ getDistance            NOTIFY trackerDataChanged)
  Q_PROPERTY(double duration            READ getDuration            NOTIFY trackerDataChanged)
  //Q_PROPERTY(double remainingDistance   READ getRemainingDistance   NOTIFY remainingDistanceChanged)
  //Q_PROPERTY(QObject* nextRouteStep     READ getNextRoutStep        NOTIFY nextStepChanged)

public:
  Tracker();
  virtual ~Tracker();

  osmscout::VehiclePosition* getTrackerPosition() const;

  double getCurrentSpeed() const { return m_currentSpeed; }
  double getDistance() const { return m_distance; }
  double getDuration() const { return m_duration; }

  Q_INVOKABLE void locationChanged(bool positionValid, double lat, double lon, bool horizontalAccuracyValid, double horizontalAccuracy);
  Q_INVOKABLE void azimuthChanged(double azimuth);

  Q_INVOKABLE void reset();

signals:
  void trackerPositionChanged();
  void trackerDataChanged();
  //void remainingDistanceChanged();
  //void nextStepChanged();

  void locationUpdate(bool positionValid, double lat, double lon);
  void azimuthUpdate(double degrees);
  void needReset();

public slots:
  void onPositionChanged(const osmscout::PositionAgent::PositionState state,
                         const osmscout::GeoCoord coord,
                         const std::shared_ptr<osmscout::Bearing> bearing);
  void onDataChanged(double kmph, double meters, double seconds);

private:
  TrackerModule* m_p;

  osmscout::Vehicle m_vehicle;
  osmscout::PositionAgent::PositionState m_vehicleState;
  osmscout::GeoCoord m_vehicleCoord;
  std::shared_ptr<osmscout::Bearing> m_vehicleBearing;
  double m_currentSpeed;
  double m_distance;
  double m_duration;

  //std::vector<osmscout::RouteStep> m_routeSteps;
  //osmscout::RouteStep m_nextRouteStep;
  //osmscout::Distance m_remainingDistance;
};

class TrackerModule : public QObject
{
  Q_OBJECT
public:
  TrackerModule(QThread* thread);
  virtual ~TrackerModule();

signals:
  void dataChanged(double kmph, double distance, double duration);
  void positionChanged(const osmscout::PositionAgent::PositionState state,
                       const osmscout::GeoCoord coord,
                       const std::shared_ptr<osmscout::Bearing> bearing);

public slots:
  void onTimeout();
  void onLocationChanged(bool positionValid, double lat, double lon);
  void onAzimuthChanged(double degrees);
  void onReset();

private:
  QThread* m_t;
  QTimer m_timer;
  osmscout::PositionAgent::PositionState m_state;
  double m_azimuth;
  double m_currentSpeed;
  osmscout::Bearing m_bearing;
  struct {
    osmscout::GeoCoord coord;
    osmscout::Timestamp time;
    inline operator bool() const { return time.time_since_epoch() != osmscout::Timestamp::duration::zero(); }
  } m_lastPosition;

  double m_distance;
  double m_duration;
};

#endif // TRACKER_H
