#ifndef REMOTEPOSITIONSOURCE_H
#define REMOTEPOSITIONSOURCE_H

#include "remote.h"

#include <QObject>

class RemotePosition : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool valid READ getValid CONSTANT)
  Q_PROPERTY(double latitude READ getLatitude CONSTANT)
  Q_PROPERTY(double longitude READ getLongitude CONSTANT)
  Q_PROPERTY(double altitude READ getAltitude CONSTANT)
  Q_PROPERTY(bool horizontalAccuracyValid READ getHorizontalAccuracyValid CONSTANT)
  Q_PROPERTY(float horizontalAccuracy READ getHorizontalAccuracy CONSTANT)

public:
  explicit RemotePosition(QObject * parent = nullptr) : QObject(parent) { }

  bool getValid() { return m_valid; }
  double getLatitude() { return m_latitude; }
  double getLongitude() { return m_longitude; }
  double getAltitude() { return m_altitude; }
  bool getHorizontalAccuracyValid() { return m_horizontalAccuracyValid; }
  float getHorizontalAccuracy() { return m_horizontalAccuracy; }
  void set(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt)
  {
    m_valid = valid;
    m_latitude = lat;
    m_longitude = lon;
    m_horizontalAccuracyValid = haccvalid;
    m_horizontalAccuracy = hacc;
    m_altitude = alt;
  }
private:
  bool m_valid;
  double m_latitude;
  double m_longitude;
  bool m_horizontalAccuracyValid;
  float m_horizontalAccuracy;
  double m_altitude;
};

class RemotePositionSource: public QObject, public Remote
{
  Q_OBJECT
  Q_PROPERTY(RemotePosition* position READ getPosition NOTIFY positionChanged)
  Q_PROPERTY(bool active READ getActive WRITE setActive NOTIFY activeChanged)
  Q_PROPERTY(int updateInterval READ getUpdateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
  Q_PROPERTY(int preferredPositioningMethods READ getPositioningMethods WRITE setPositioningMethods NOTIFY preferredPositioningMethodsChanged)

public:
  explicit RemotePositionSource(QObject * parent = nullptr) : QObject(parent), m_position(this) { }

  RemotePosition* getPosition() { return &m_position; }
  bool getActive() { return m_active; }
  int getUpdateInterval() { return m_updateInterval; }
  int getPositioningMethods() { return m_positioningMethods; }
  void setActive(bool active);
  void setUpdateInterval(int interval) { (void)interval; /*disabled*/ }

  // for testing
  void set(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt)
  {
    _positionUpdated(valid, lat, lon, haccvalid, hacc, alt);
  }

  Q_INVOKABLE void connectToService(QVariant service) override;
  void connectToService(ServiceFrontendPtr& service) override;

signals:
  // operations
  void setPositioningMethods(int methods);
  // callbacks
  void positionChanged();
  void activeChanged();
  void updateIntervalChanged();
  void preferredPositioningMethodsChanged();

private slots:
  void _positionUpdated(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt);
  void _updateIntervalChanged(int interval);
  void _preferredPositioningMethodsChanged(int methods);

private:
  ServiceFrontendPtr m_service;
  RemotePosition m_position;
  bool m_active = false;
  int m_updateInterval = 0;
  int m_positioningMethods;
};

#endif // REMOTEPOSITIONSOURCE_H
