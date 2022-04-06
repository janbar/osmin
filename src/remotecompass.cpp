#include "remotecompass.h"
#include "servicefrontend.h"

void RemoteCompass::setActive(bool active)
{
  if (m_service)
  {
    if (!m_active && active)
    {
      connect(m_service.data(), &ServiceFrontend::compassReadingChanged, this, &RemoteCompass::_readingChanged);
      m_active = true;
      emit activeChanged();
    }
    else if (m_active && !active)
    {
      disconnect(m_service.data(), &ServiceFrontend::compassReadingChanged, this, &RemoteCompass::_readingChanged);
      m_active = false;
      emit activeChanged();
    }
  }
}


void RemoteCompass::connectToService(QVariant service)
{
  ServiceFrontendPtr _service = service.value<ServiceFrontendPtr>();
  connectToService(_service);
}

void RemoteCompass::connectToService(ServiceFrontendPtr& service)
{
  if (m_service)
  {
    setActive(false);
  }
  m_service = service;
  if (m_service)
  {
    setActive(true);
  }
}

void RemoteCompass::_readingChanged(float azimuth, float calibration)
{
  m_reading.set(azimuth, calibration);
  emit readingChanged();
}
