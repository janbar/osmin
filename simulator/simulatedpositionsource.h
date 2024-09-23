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

#include <mutex>
#include <QtPositioning/QGeoPositionInfoSource>
#include <QTimer>

class SimulatedPositionSource : public QGeoPositionInfoSource
{
  Q_OBJECT
public:
  explicit SimulatedPositionSource(QObject *parent = nullptr);
  ~SimulatedPositionSource() = default;

  void setUpdateInterval(int msec) override;
  QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;
  PositioningMethods supportedPositioningMethods() const override;
  int minimumUpdateInterval() const override;
  Error error() const override;

  static const QGeoPositionInfo& data() { return _info; };
  static void resetData(double lat, double lon, double alt);

public Q_SLOTS:
  void startUpdates() override;
  void stopUpdates() override;
  void requestUpdate(int timeout = 0) override;

private slots:
  void onTimeout();

private:
  Q_DISABLE_COPY(SimulatedPositionSource)

  static std::mutex _mutex;
  static QGeoPositionInfo _info;
  QTimer _updateTimer;
  QDateTime _lastUpdate;
};

#endif // SIMULATEDPOSITIONSOURCE_H
