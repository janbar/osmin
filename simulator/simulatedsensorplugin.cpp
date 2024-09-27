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

#include "simulatedsensorplugin.h"

#include <QFile>
#include <QDebug>

void SimulatedSensorPlugin::registerSensors()
{
#if QT_VERSION < 0x060000 //QT_VERSION_CHECK(6, 0, 0)
  if (!QSensorManager::isBackendRegistered(QCompass::type, SimulatedCompass::id)) {
    qInfo("Register sensor backend: %s", SimulatedCompass::id);
    QSensorManager::registerBackend(QCompass::type, SimulatedCompass::id, this);
  }
#else
  if (!QSensorManager::isBackendRegistered(QCompass::sensorType, SimulatedCompass::id)) {
    qInfo("Register sensor backend: %s", SimulatedCompass::id);
    QSensorManager::registerBackend(QCompass::sensorType, SimulatedCompass::id, this);
  }
#endif
}

SimulatedCompass *SimulatedSensorPlugin::createBackend(QSensor *sensor)
{
  if (sensor->identifier() == SimulatedCompass::id) {
    return new SimulatedCompass(_compass, sensor);
  }
  return 0;
}

SimulatedSensor::SimulatedSensor(QObject *parent) : QSensor("QCompass", parent)
{
  this->setIdentifier(SimulatedCompass::id);
}

SimulatedSensor::~SimulatedSensor()
{
}

//#include "plugin.moc"
