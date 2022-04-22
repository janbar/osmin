/*
 * Copyright (C) 2022
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
