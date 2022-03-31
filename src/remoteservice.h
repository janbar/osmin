#ifndef REMOTESERVICE_H
#define REMOTESERVICE_H

#include "remote.h"

#include <QObject>

class RemoteService : public QObject, public Remote
{
  Q_OBJECT

  Q_PROPERTY(ServiceStatus status READ getStatus NOTIFY statusChanged)

public:
  enum ServiceStatus {
    ServiceDisconnected   = 0,
    ServiceConnected      = 1,
  };

  Q_ENUM(ServiceStatus)

  explicit RemoteService(QObject* parent = nullptr);

  ServiceStatus getStatus() const { return m_status; }

  Q_INVOKABLE QVariant getServiceHandle();
  Q_INVOKABLE void connectToService(QVariant service) override;
  void connectToService(ServiceFrontendPtr& service) override;

signals:
  void ping(const QString& message);
  void statusChanged();

private slots:
  void _serviceConnected();
  void _serviceDisconnected();

private:
  ServiceFrontendPtr m_service;
  ServiceStatus m_status = ServiceDisconnected;
};

#endif // REMOTESERVICE_H
