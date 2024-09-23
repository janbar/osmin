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
#ifndef SIMULATEDSENSORPLUGIN_H
#define SIMULATEDSENSORPLUGIN_H

#include "simulatedcompass.h"

#include <QtSensors/qsensorplugin.h>
#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qsensormanager.h>
#include <QFile>
#include <QDebug>

class SimulatedSensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
  Q_OBJECT
  Q_INTERFACES(QSensorPluginInterface)
public:
  void registerSensors() override;

  SimulatedCompass *createBackend(QSensor *sensor) override;
};

class SimulatedSensor : public QSensor
{
  Q_OBJECT
public:
  SimulatedSensor(QObject *parent = nullptr);
  virtual ~SimulatedSensor();
};

#endif // SIMULATEDSENSORPLUGIN_H
