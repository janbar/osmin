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
#ifdef QTSENSORS_GENERICCOMPASS
#include "genericcompass.h"
#endif
#include <QtSensors/qsensorplugin.h>
#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qsensormanager.h>
#include <QFile>
#include <QDebug>

void BuiltInSensorPlugin::registerSensors()
{
#ifdef QTSENSORS_GENERICCOMPASS
  if (!QSensorManager::isBackendRegistered(QCompass::type, GenericCompass::id)) {
    qInfo("Register sensor backend: %s", GenericCompass::id);
    QSensorManager::registerBackend(QCompass::type, GenericCompass::id, this);
  }
#endif
}

QSensorBackend *BuiltInSensorPlugin::createBackend(QSensor *sensor)
{
#ifdef QTSENSORS_GENERICCOMPASS
  if (sensor->identifier() == GenericCompass::id) {
    return new GenericCompass(sensor);
  }
#endif
  return 0;
}

BuiltInCompass::BuiltInCompass(QObject* parent) : QSensor("QCompass", parent)
{
#ifdef QTSENSORS_GENERICCOMPASS
  this->setIdentifier("builtin.compass");
#endif
}

BuiltInCompass::~BuiltInCompass()
{
}

//#include "plugin.moc"
