#include "remotepositionsource.h"
#include "servicefrontend.h"

void RemotePositionSource::setActive(bool active)
{
  if (m_service)
  {
    if (!m_active && active)
    {
      connect(m_service.data(), &ServiceFrontend::positionPositionUpdated, this, &RemotePositionSource::_positionUpdated);
      connect(m_service.data(), &ServiceFrontend::positionUpdateIntervalChanged, this, &RemotePositionSource::_updateIntervalChanged);
      connect(m_service.data(), &ServiceFrontend::positionPreferredPositioningMethodsChanged, this, &RemotePositionSource::_preferredPositioningMethodsChanged);
      m_active = true;
      emit activeChanged();
    }
    else if (m_active && !active)
    {
      disconnect(m_service.data(), &ServiceFrontend::positionPositionUpdated, this, &RemotePositionSource::_positionUpdated);
      disconnect(m_service.data(), &ServiceFrontend::positionUpdateIntervalChanged, this, &RemotePositionSource::_updateIntervalChanged);
      disconnect(m_service.data(), &ServiceFrontend::positionPreferredPositioningMethodsChanged, this, &RemotePositionSource::_preferredPositioningMethodsChanged);
      m_active = false;
      emit activeChanged();
    }
  }
}

void RemotePositionSource::connectToService(QVariant service)
{
  ServiceFrontendPtr _service = service.value<ServiceFrontendPtr>();
  connectToService(_service);
}

void RemotePositionSource::connectToService(ServiceFrontendPtr& service)
{
  if (m_service)
  {
    setActive(false);
    //disconnect(this, &ServicePositionSource::setUpdateInterval, m_service.data(), &ServiceRemote::setPositionUpdateInterval);
    disconnect(this, &RemotePositionSource::setPositioningMethods, m_service.data(), &ServiceFrontend::setPreferedPositioningMethods);
  }
  m_service = service;
  if (m_service)
  {
    //connect(this, &ServicePositionSource::setUpdateInterval, m_service.data(), &ServiceRemote::setPositionUpdateInterval);
    connect(this, &RemotePositionSource::setPositioningMethods, m_service.data(), &ServiceFrontend::setPreferedPositioningMethods);
    setActive(true);
  }
}

void RemotePositionSource::_positionUpdated(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt)
{
  m_position.set(valid, lat, lon, haccvalid, hacc, alt);
  emit positionChanged();
}

void RemotePositionSource::_updateIntervalChanged(int interval)
{
  m_updateInterval = interval;
  emit updateIntervalChanged();

}

void RemotePositionSource::_preferredPositioningMethodsChanged(int methods)
{
  m_positioningMethods = methods;
  emit preferredPositioningMethodsChanged();
}

