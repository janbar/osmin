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

#ifndef BUILTINSENSORPLUGIN_H
#define BUILTINSENSORPLUGIN_H

#include "genericcompass.h"

#include <QtSensors/qsensorplugin.h>
#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qsensormanager.h>
#include <QFile>
#include <QDebug>

class BuiltInSensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
  Q_OBJECT
  Q_INTERFACES(QSensorPluginInterface)
public:
  void registerSensors();

  GenericCompass *createBackend(QSensor *sensor);
};

class BuiltInCompass : public QSensor
{
  Q_OBJECT
public:
  BuiltInCompass(QObject *parent = nullptr);
  virtual ~BuiltInCompass();
};

#endif // BUILTINSENSORPLUGIN_H
