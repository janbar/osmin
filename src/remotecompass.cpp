#include "remotecompass.h"
#include "servicefrontend.h"

void RemoteCompass::setActive(bool active)
{
  if (m_service)
  {
    if (!m_active && active)
    {
      connect(m_service.data(), &ServiceFrontend::compassReadingChanged, this, &RemoteCompass::_readingChanged);
      connect(m_service.data(), &ServiceFrontend::compassDataRateChanged, this, &RemoteCompass::_dataRateChanged);
      m_active = true;
      emit activeChanged();
    }
    else if (m_active && !active)
    {
      disconnect(m_service.data(), &ServiceFrontend::compassReadingChanged, this, &RemoteCompass::_readingChanged);
      disconnect(m_service.data(), &ServiceFrontend::compassDataRateChanged, this, &RemoteCompass::_dataRateChanged);
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
    //disconnect(this, &ServiceCompass::setDataRate, m_service.data(), &ServiceRemote::setCompassDataRate);
  }
  m_service = service;
  if (m_service)
  {
    //connect(this, &ServiceCompass::setDataRate, m_service.data(), &ServiceRemote::setCompassDataRate);
    setActive(true);
  }
}

void RemoteCompass::_readingChanged(float azimuth, float calibration)
{
  m_reading.set(azimuth, calibration);
  emit readingChanged();
}

void RemoteCompass::_dataRateChanged(int datarate)
{
  m_dataRate = datarate;
  emit dataRateChanged();
}
