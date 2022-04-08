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
    disconnect(m_service.data(), &ServiceFrontend::positionActiveChanged, this, &RemoteService::_positionActiveChanged);
    disconnect(m_service.data(), &ServiceFrontend::positionUpdateIntervalChanged, this, &RemoteService::_positionUpdateIntervalChanged);
    disconnect(m_service.data(), &ServiceFrontend::positionPreferredPositioningMethodsChanged, this, &RemoteService::_preferredPositioningMethodsChanged);
    disconnect(m_service.data(), &ServiceFrontend::compassDataRateChanged, this, &RemoteService::_compassDataRateChanged);
    disconnect(this, &RemoteService::ping, m_service.data(), &ServiceFrontend::ping);
    disconnect(this, &RemoteService::setPositionUpdateInterval, m_service.data(), &ServiceFrontend::setPositionUpdateInterval);
    disconnect(this, &RemoteService::setPreferredPositioningMethods, m_service.data(), &ServiceFrontend::setPreferedPositioningMethods);
    disconnect(this, &RemoteService::setCompassDataRate, m_service.data(), &ServiceFrontend::setCompassDataRate);
    disconnect(this, &RemoteService::positionStartUpdates, m_service.data(), &ServiceFrontend::positionStartUpdates);
    disconnect(this, &RemoteService::positionStopUpdates, m_service.data(), &ServiceFrontend::positionStopUpdates);
  }
  m_service = service;
  if (m_service)
  {
    connect(m_service.data(), &ServiceFrontend::serviceConnected, this, &RemoteService::_serviceConnected);
    connect(m_service.data(), &ServiceFrontend::serviceDisconnected, this, &RemoteService::_serviceDisconnected);
    connect(m_service.data(), &ServiceFrontend::positionActiveChanged, this, &RemoteService::_positionActiveChanged);
    connect(m_service.data(), &ServiceFrontend::positionUpdateIntervalChanged, this, &RemoteService::_positionUpdateIntervalChanged);
    connect(m_service.data(), &ServiceFrontend::positionPreferredPositioningMethodsChanged, this, &RemoteService::_preferredPositioningMethodsChanged);
    connect(m_service.data(), &ServiceFrontend::compassDataRateChanged, this, &RemoteService::_compassDataRateChanged);
    connect(this, &RemoteService::ping, m_service.data(), &ServiceFrontend::ping);
    connect(this, &RemoteService::setPositionUpdateInterval, m_service.data(), &ServiceFrontend::setPositionUpdateInterval);
    connect(this, &RemoteService::setPreferredPositioningMethods, m_service.data(), &ServiceFrontend::setPreferedPositioningMethods);
    connect(this, &RemoteService::setCompassDataRate, m_service.data(), &ServiceFrontend::setCompassDataRate);
    connect(this, &RemoteService::positionStartUpdates, m_service.data(), &ServiceFrontend::positionStartUpdates);
    connect(this, &RemoteService::positionStopUpdates, m_service.data(), &ServiceFrontend::positionStopUpdates);
  }
}

void RemoteService::setPositionActive(bool active)
{
  if (active)
    emit positionStartUpdates();
  else
    emit positionStopUpdates();
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

void RemoteService::_positionActiveChanged(bool active)
{
  m_positionActive = active;
  emit positionActiveChanged();
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

void RemoteService::_compassDataRateChanged(int dataRate)
{
  m_compassDataRate = dataRate;
  emit compassDataRateChanged();
}
