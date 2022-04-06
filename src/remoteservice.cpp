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
    disconnect(m_service.data(), &ServiceFrontend::positionUpdateIntervalChanged, this, &RemoteService::_positionUpdateIntervalChanged);
    disconnect(m_service.data(), &ServiceFrontend::positionPreferredPositioningMethodsChanged, this, &RemoteService::_preferredPositioningMethodsChanged);
    disconnect(this, &RemoteService::ping, m_service.data(), &ServiceFrontend::ping);
    disconnect(this, &RemoteService::setPositionUpdateInterval, m_service.data(), &ServiceFrontend::setPositionUpdateInterval);
    disconnect(this, &RemoteService::setPreferredPositioningMethods, m_service.data(), &ServiceFrontend::setPreferedPositioningMethods);
  }
  m_service = service;
  if (m_service)
  {
    connect(m_service.data(), &ServiceFrontend::serviceConnected, this, &RemoteService::_serviceConnected);
    connect(m_service.data(), &ServiceFrontend::serviceDisconnected, this, &RemoteService::_serviceDisconnected);
    connect(m_service.data(), &ServiceFrontend::positionUpdateIntervalChanged, this, &RemoteService::_positionUpdateIntervalChanged);
    connect(m_service.data(), &ServiceFrontend::positionPreferredPositioningMethodsChanged, this, &RemoteService::_preferredPositioningMethodsChanged);
    connect(this, &RemoteService::ping, m_service.data(), &ServiceFrontend::ping);
    connect(this, &RemoteService::setPositionUpdateInterval, m_service.data(), &ServiceFrontend::setPositionUpdateInterval);
    connect(this, &RemoteService::setPreferredPositioningMethods, m_service.data(), &ServiceFrontend::setPreferedPositioningMethods);
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

void RemoteService::_positionUpdateIntervalChanged(int interval)
{
  m_positionUpdateInterval = interval;
  emit positionUpdateIntervalChanged();
}

void RemoteService::_preferredPositioningMethodsChanged(int methods)
{
  m_preferredPositioningMethods = methods;
  emit preferredPositioningMethodsChanged();
}
