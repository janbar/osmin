#ifndef REMOTESERVICE_H
#define REMOTESERVICE_H

#include "remote.h"

#include <QObject>

class RemoteService : public QObject, public Remote
{
  Q_OBJECT

  Q_PROPERTY(ServiceStatus status READ getStatus NOTIFY statusChanged)
  Q_PROPERTY(int positionUpdateInterval READ getPositionUpdateInterval WRITE setPositionUpdateInterval NOTIFY positionUpdateIntervalChanged)
  Q_PROPERTY(int preferredPositioningMethods READ getPreferredPositioningMethods WRITE setPreferredPositioningMethods NOTIFY preferredPositioningMethodsChanged)

public:
  enum ServiceStatus {
    ServiceDisconnected   = 0,
    ServiceConnected      = 1,
  };
  Q_ENUM(ServiceStatus)

  enum PositioningMethods
  {
    NoPositioningMethods            = 0,
    SatellitePositioningMethods     = 1,
    NonSatellitePositioningMethods  = 2,
    AllPositioningMethods           = 3,
  };
  Q_ENUM(PositioningMethods)

  explicit RemoteService(QObject* parent = nullptr);

  ServiceStatus getStatus() const { return m_status; }

  Q_INVOKABLE QVariant getServiceHandle();
  Q_INVOKABLE void connectToService(QVariant service) override;
  void connectToService(ServiceFrontendPtr& service) override;

  int getPositionUpdateInterval() { return m_positionUpdateInterval; }
  int getPreferredPositioningMethods() { return m_preferredPositioningMethods; }


signals:
  // operations
  void ping(const QString& message);
  void setPositionUpdateInterval(int interval);
  void setPreferredPositioningMethods(int methods);
  // callbacks
  void statusChanged();
  void positionUpdateIntervalChanged();
  void preferredPositioningMethodsChanged();

private slots:
  void _serviceConnected();
  void _serviceDisconnected();
  void _positionUpdateIntervalChanged(int interval);
  void _preferredPositioningMethodsChanged(int methods);

private:
  ServiceFrontendPtr m_service;
  ServiceStatus m_status = ServiceDisconnected;

  int m_positionUpdateInterval = 0;
  int m_preferredPositioningMethods = NoPositioningMethods;
};

#endif // REMOTESERVICE_H
