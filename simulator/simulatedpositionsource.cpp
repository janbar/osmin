/*
 * Copyright (C) 2024
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

#include "simulatedpositionsource.h"

#define INTERVAL_MIN  100   /* millisec */

SimulatedPositionSource::SimulatedPositionSource(GlobalPosition& gp, QObject *parent)
    : QGeoPositionInfoSource(parent), _position(gp)
{
  QGeoPositionInfoSource::setUpdateInterval(INTERVAL_MIN);
}

void SimulatedPositionSource::setUpdateInterval(int msec)
{
  if (msec >= INTERVAL_MIN)
  {
    // the base class store the value
    QGeoPositionInfoSource::setUpdateInterval(msec);
  }
}

QGeoPositionInfo SimulatedPositionSource::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
  Q_UNUSED(fromSatellitePositioningMethodsOnly);
  return _position.data();
}

QGeoPositionInfoSource::PositioningMethods SimulatedPositionSource::supportedPositioningMethods() const
{
  QGeoPositionInfoSource::PositioningMethods methods;
  methods.setFlag(SatellitePositioningMethods);
  return methods;
}

int SimulatedPositionSource::minimumUpdateInterval() const
{
  return INTERVAL_MIN;
}

QGeoPositionInfoSource::Error SimulatedPositionSource::error() const
{
  return QGeoPositionInfoSource::NoError;
}

void SimulatedPositionSource::startUpdates()
{
  if (!_active)
  {
    connect(&_position, &GlobalPosition::dataUpdated,
            this, &SimulatedPositionSource::onDataUpdated,
            Qt::QueuedConnection);
    _active = true;
  }
}

void SimulatedPositionSource::stopUpdates()
{
  if (_active)
  {
    disconnect(&_position, &GlobalPosition::dataUpdated,
               this, &SimulatedPositionSource::onDataUpdated);
    _active = false;
  }
}

void SimulatedPositionSource::requestUpdate(int timeout)
{
  Q_UNUSED(timeout);
  emit positionUpdated(_position.data());
}

void SimulatedPositionSource::onDataUpdated()
{
  emit positionUpdated(_position.data());
}
