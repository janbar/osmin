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

#include <QDebug>

#define INTERVAL_MIN  100   /* millisec */

std::mutex SimulatedPositionSource::_mutex;
QGeoPositionInfo SimulatedPositionSource::_info;

SimulatedPositionSource::SimulatedPositionSource(QObject *parent)
    : QGeoPositionInfoSource(parent)
{
  QGeoPositionInfoSource::setUpdateInterval(INTERVAL_MIN);

  connect(&_updateTimer, &QTimer::timeout,
          this, &SimulatedPositionSource::onTimeout);
}

void SimulatedPositionSource::setUpdateInterval(int msec)
{
  if (msec >= INTERVAL_MIN)
  {
    // the base class store the value
    QGeoPositionInfoSource::setUpdateInterval(msec);
    if (_updateTimer.isActive())
      _updateTimer.setInterval(msec);
  }
}

QGeoPositionInfo SimulatedPositionSource::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
  std::lock_guard<std::mutex> g(_mutex);
  Q_UNUSED(fromSatellitePositioningMethodsOnly);
  return _info;
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
  if (!_updateTimer.isActive())
  {
    _updateTimer.setInterval(updateInterval());
    _updateTimer.start();
  }
}

void SimulatedPositionSource::stopUpdates()
{
  if (_updateTimer.isActive())
  {
    _updateTimer.stop();
  }
}

void SimulatedPositionSource::requestUpdate(int timeout)
{
  std::lock_guard<std::mutex> g(_mutex);
  Q_UNUSED(timeout);
  emit positionUpdated(_info);
}

void SimulatedPositionSource::onTimeout()
{
  std::lock_guard<std::mutex> g(_mutex);
  if (_info.timestamp() != _lastUpdate)
  {
    _lastUpdate = _info.timestamp();
    emit positionUpdated(_info);
  }
}

void SimulatedPositionSource::resetData(double lat, double lon, double alt)
{
  QGeoCoordinate coord(lat, lon, alt);
  std::lock_guard<std::mutex> g(_mutex);
  _info = QGeoPositionInfo(coord, QDateTime::currentDateTime());
}
