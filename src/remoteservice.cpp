#include "remoteservice.h"
#include "servicefrontend.h"

RemoteService::RemoteService(QObject * parent)
: QObject(parent)
{
}

QVariant RemoteService::getServiceHandle()
{
  QVariant var;
  var.setValue<ServiceFrontendPtr>(m_service);
  return var;
}

void RemoteService::connectToService(QVariant service)
{
  ServiceFrontendPtr _service = service.value<ServiceFrontendPtr>();
}

void RemoteService::connectToService(ServiceFrontendPtr& service)
{
  if (m_service)
  {
    disconnect(m_service.data(), &ServiceFrontend::serviceConnected, this, &RemoteService::_serviceConnected);
    disconnect(m_service.data(), &ServiceFrontend::serviceDisconnected, this, &RemoteService::_serviceDisconnected);
    disconnect(this, &RemoteService::ping, m_service.data(), &ServiceFrontend::ping);
  }
  m_service = service;
  if (m_service)
  {
    connect(m_service.data(), &ServiceFrontend::serviceConnected, this, &RemoteService::_serviceConnected);
    connect(m_service.data(), &ServiceFrontend::serviceDisconnected, this, &RemoteService::_serviceDisconnected);
    connect(this, &RemoteService::ping, m_service.data(), &ServiceFrontend::ping);
  }
}

void RemoteService::_serviceConnected()
{
  m_status = ServiceConnected;
  emit statusChanged();
}

void RemoteService::_serviceDisconnected()
{
  m_status = ServiceDisconnected;
  emit statusChanged();
}
