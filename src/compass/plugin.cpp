/*
 * Copyright (C) 2020
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

#include "plugin.h"
#include "genericcompass.h"
#include <QtSensors/qsensorplugin.h>
#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qsensormanager.h>
#include <QFile>
#include <QDebug>

void BuiltInSensorPlugin::registerSensors()
{
  if (!QSensorManager::isBackendRegistered(QCompass::type, GenericCompass::id)) {
    qInfo("Register sensor backend: %s", GenericCompass::id);
    QSensorManager::registerBackend(QCompass::type, GenericCompass::id, this);
  }
}

QSensorBackend *BuiltInSensorPlugin::createBackend(QSensor *sensor)
{
  if (sensor->identifier() == GenericCompass::id) {
    return new GenericCompass(sensor);
  }
  return 0;
}

BuiltInCompass::BuiltInCompass(QObject* parent) : QSensor("QCompass", parent)
{
  this->setIdentifier("builtin.compass");
}

BuiltInCompass::~BuiltInCompass()
{
}

//#include "plugin.moc"
