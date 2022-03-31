#ifndef REMOTETRACKER_H
#define REMOTETRACKER_H

#include "remote.h"
#include <osmscoutclientqt/VehiclePosition.h>
#include <osmscoutclientqt/RouteStep.h>

#include <QObject>

class RemoteTracker : public QObject, public Remote
{
  Q_OBJECT

  Q_PROPERTY(QObject* trackerPosition READ getTrackerPosition NOTIFY trackerPositionChanged)
  Q_PROPERTY(double elevation READ getElevation NOTIFY trackerDataChanged)
  Q_PROPERTY(double currentSpeed READ getCurrentSpeed NOTIFY trackerDataChanged)
  Q_PROPERTY(double distance READ getDistance NOTIFY trackerDataChanged)
  Q_PROPERTY(double duration READ getDuration NOTIFY trackerDataChanged)
  Q_PROPERTY(double ascent READ getAscent NOTIFY trackerDataChanged)
  Q_PROPERTY(double descent READ getDescent NOTIFY trackerDataChanged)
  Q_PROPERTY(double maxSpeed READ getMaxSpeed NOTIFY trackerDataChanged)
  Q_PROPERTY(QString recording READ getRecording WRITE setRecording NOTIFY trackerRecordingChanged)
  Q_PROPERTY(bool processing READ getProcessing NOTIFY trackerProcessingChanged)
  Q_PROPERTY(bool isRecording READ getIsRecording NOTIFY trackerIsRecordingChanged)
  //Q_PROPERTY(double remainingDistance READ getRemainingDistance NOTIFY remainingDistanceChanged)
  //Q_PROPERTY(QObject* nextRouteStep READ getNextRoutStep NOTIFY nextStepChanged)

public:
  explicit RemoteTracker(QObject* parent = nullptr);

  osmscout::VehiclePosition* getTrackerPosition() const;
  double getElevation() const { return m_elevation; }
  double getCurrentSpeed() const { return m_currentSpeed; }
  double getDistance() const { return m_distance; }
  double getDuration() const { return m_duration; }
  double getAscent() const { return m_ascent; }
  double getDescent() const { return m_descent; }
  double getMaxSpeed() const { return m_maxSpeed; }
  double getLat() const { return m_vehicleCoord.GetLat(); }
  double getLon() const { return m_vehicleCoord.GetLon(); }
  QString getRecording() const { return m_recording; }
  void setRecording(const QString& filename);
  bool getProcessing() const { return m_busy; }
  bool getIsRecording() const { return m_isRecording; }

  Q_INVOKABLE void connectToService(QVariant service) override;
  void connectToService(ServiceFrontendPtr& service) override;

signals:
  // operations
  void reset();
  void startRecording();
  void stopRecording();
  void pinPosition();
  void markPosition(const QString& symbol, const QString& name, const QString& description);
  // callbacks
  void trackerPositionChanged();
  void trackerDataChanged();
  void trackerRecordingChanged();
  void trackerIsRecordingChanged();
  void trackerProcessingChanged();
  void trackerPositionRecorded(double lat, double lon);
  void trackerPositionMarked(double lat, double lon, const QString& symbol, const QString& name);
  void recordingFailed();
  void resumeRecording();

private slots:
  // internal events
  void _trackerIsRecordingChanged(bool recording);
  void _trackerRecordingChanged(const QString& filename);
  void _trackerProcessingChanged(bool processing);
  void _trackerPositionChanged(bool valid, double lat, double lon, double bearing);
  void _trackerDataChanged(
                  double elevation,
                  double currentSpeed,
                  double distance,
                  double duration,
                  double ascent,
                  double descent,
                  double maxSpeed);

private:
  ServiceFrontendPtr m_service;
  osmscout::Vehicle m_vehicle;
  osmscout::PositionAgent::PositionState m_vehicleState;
  osmscout::GeoCoord m_vehicleCoord;
  std::optional<osmscout::Bearing> m_vehicleBearing;
  double m_elevation;
  double m_currentSpeed;
  double m_distance;
  double m_duration;
  double m_ascent;
  double m_descent;
  double m_maxSpeed;
  bool m_busy;
  QString m_recording;
  bool m_isRecording;
};

#endif // REMOTETRACKER_H
