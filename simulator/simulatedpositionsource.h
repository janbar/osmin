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
#ifndef SIMULATEDPOSITIONSOURCE_H
#define SIMULATEDPOSITIONSOURCE_H

#include "globalposition.h"

#include <QtPositioning/QGeoPositionInfoSource>

class SimulatedPositionSource : public QGeoPositionInfoSource
{
  Q_OBJECT
public:
  explicit SimulatedPositionSource(GlobalPosition& gp, QObject *parent = nullptr);
  ~SimulatedPositionSource() = default;

  void setUpdateInterval(int msec) override;
  QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;
  PositioningMethods supportedPositioningMethods() const override;
  int minimumUpdateInterval() const override;
  Error error() const override;

public slots:
  void startUpdates() override;
  void stopUpdates() override;
  void requestUpdate(int timeout = 0) override;

private slots:
  void onDataUpdated();

private:
  Q_DISABLE_COPY(SimulatedPositionSource)

  GlobalPosition& _position;
  bool _active = false;
};

#endif // SIMULATEDPOSITIONSOURCE_H
