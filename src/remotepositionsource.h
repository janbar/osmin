/*
 * Copyright (C) 2022
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
#ifndef REMOTEPOSITIONSOURCE_H
#define REMOTEPOSITIONSOURCE_H

#include "remote.h"

#include <QObject>

class RemotePosition : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool valid READ getValid CONSTANT)
  Q_PROPERTY(double latitude READ getLatitude CONSTANT)
  Q_PROPERTY(double longitude READ getLongitude CONSTANT)
  Q_PROPERTY(double altitude READ getAltitude CONSTANT)
  Q_PROPERTY(bool horizontalAccuracyValid READ getHorizontalAccuracyValid CONSTANT)
  Q_PROPERTY(float horizontalAccuracy READ getHorizontalAccuracy CONSTANT)

public:
  explicit RemotePosition(QObject * parent = nullptr) : QObject(parent) { }

  bool getValid() { return m_valid; }
  double getLatitude() { return m_latitude; }
  double getLongitude() { return m_longitude; }
  double getAltitude() { return m_altitude; }
  bool getHorizontalAccuracyValid() { return m_horizontalAccuracyValid; }
  float getHorizontalAccuracy() { return m_horizontalAccuracy; }
  void set(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt)
  {
    m_valid = valid;
    m_latitude = lat;
    m_longitude = lon;
    m_horizontalAccuracyValid = haccvalid;
    m_horizontalAccuracy = hacc;
    m_altitude = alt;
  }
private:
  bool m_valid;
  double m_latitude;
  double m_longitude;
  bool m_horizontalAccuracyValid;
  float m_horizontalAccuracy;
  double m_altitude;
};

class RemotePositionSource: public QObject, public Remote
{
  Q_OBJECT
  Q_PROPERTY(RemotePosition* position READ getPosition NOTIFY positionChanged)
  Q_PROPERTY(bool active READ getActive WRITE setActive NOTIFY activeChanged)

public:

  explicit RemotePositionSource(QObject * parent = nullptr) : QObject(parent), m_position(this) { }

  RemotePosition* getPosition() { return &m_position; }
  bool getActive() { return m_active; }
  void setActive(bool active);

  // for testing
  void set(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt)
  {
    _positionUpdated(valid, lat, lon, haccvalid, hacc, alt);
  }

  Q_INVOKABLE void connectToService(QVariant service) override;
  void connectToService(ServiceFrontendPtr& service) override;

signals:
  // callbacks
  void positionChanged();
  void activeChanged();

private slots:
  void _positionUpdated(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt);

private:
  ServiceFrontendPtr m_service;
  RemotePosition m_position;
  bool m_active = false;
};

#endif // REMOTEPOSITIONSOURCE_H
