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
  Q_PROPERTY(int compassDataRate READ getCompassDataRate WRITE setCompassDataRate NOTIFY compassDataRateChanged)
  Q_PROPERTY(bool positionActive READ getPositionActive WRITE setPositionActive NOTIFY positionActiveChanged)

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
  int getCompassDataRate() { return m_compassDataRate; }
  bool getPositionActive() { return m_positionActive; }
  void setPositionActive(bool active);

signals:
  // operations
  void ping(const QString& message);
  void setPositionUpdateInterval(int interval);
  void setPreferredPositioningMethods(int methods);
  void setCompassDataRate(int datarate);
  void positionStartUpdates();
  void positionStopUpdates();
  // callbacks
  void statusChanged();
  void positionActiveChanged();
  void positionUpdateIntervalChanged();
  void preferredPositioningMethodsChanged();
  void compassDataRateChanged();

private slots:
  void _serviceConnected();
  void _serviceDisconnected();
  void _positionActiveChanged(bool active);
  void _positionUpdateIntervalChanged(int interval);
  void _preferredPositioningMethodsChanged(int methods);
  void _compassDataRateChanged(int dataRate);

private:
  ServiceFrontendPtr m_service;
  ServiceStatus m_status = ServiceDisconnected;

  bool m_positionActive = false;
  int m_positionUpdateInterval = 0;
  int m_preferredPositioningMethods = NoPositioningMethods;
  int m_compassDataRate = 0;
};

#endif // REMOTESERVICE_H
