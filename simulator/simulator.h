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
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "simulatorbreaker.h"
#include "simulatedsensorplugin.h"
#include "simulatedpositionsource.h"
#include "commandline.h"
#include "gpxrunner.h"

#include <converter.h>

#include <QObject>

class Simulator : public QObject, public SimulatorBreaker
{
  Q_OBJECT

public:
  Simulator();
  ~Simulator();

  void enableCLI(CommandLine * cmd);
  void disableCLI();

  QSensor * compassSensor() { return _compassSource; }
  QGeoPositionInfoSource * positionSource() { return _positionSource; }

  bool onKeyBreak() override;

public slots:
  void onCommand(QStringList tokens);
  void onQuit();
  void onListGPXRequested();
  void onStatusRequested();
  void onPointChanged(int pts);
  void onRunFinished();

private:
  QSensor * _compassSource = nullptr;
  SimulatedSensorPlugin * _compassPlugin = nullptr;
  SimulatedPositionSource * _positionSource = nullptr;
  CommandLine * _cmd = nullptr;
  Converter _converter;
  GPXRunner _gpxrunner;

  void prompt();
  static qreal normalizeAzimuth(qreal azimuth);
};

#endif // SIMULATOR_H
