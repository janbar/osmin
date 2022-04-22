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
#ifndef CONVERTER_H
#define CONVERTER_H

#include <QObject>
#include <QString>
#include <QQmlEngine>

class Converter : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString system READ getSystem WRITE setSystem NOTIFY systemChanged)
  Q_PROPERTY(QString meters READ getMeters WRITE setMeters NOTIFY metersChanged)
  Q_PROPERTY(QString km READ getKm WRITE setKm NOTIFY kmChanged)
  Q_PROPERTY(QString feet READ getFeet WRITE setFeet NOTIFY feetChanged)
  Q_PROPERTY(QString miles READ getMiles WRITE setMiles NOTIFY milesChanged)
  Q_PROPERTY(QString north READ getNorth WRITE setNorth NOTIFY northChanged)
  Q_PROPERTY(QString south READ getSouth WRITE setSouth NOTIFY southChanged)
  Q_PROPERTY(QString west READ getWest WRITE setWest NOTIFY westChanged)
  Q_PROPERTY(QString east READ getEast WRITE setEast NOTIFY eastChanged)
  Q_PROPERTY(QString northwest READ getNorthwest WRITE setNorthwest NOTIFY northwestChanged)
  Q_PROPERTY(QString northeast READ getNortheast WRITE setNortheast NOTIFY northeastChanged)
  Q_PROPERTY(QString southwest READ getSouthwest WRITE setSouthwest NOTIFY southwestChanged)
  Q_PROPERTY(QString southeast READ getSoutheast WRITE setSoutheast NOTIFY southeastChanged)

public:
  explicit Converter(QObject *parent = nullptr);
  ~Converter();

  Q_INVOKABLE QStringList systems() const;
  Q_INVOKABLE QString readableDistance(double distance) const;
  Q_INVOKABLE QString panelDistance(double distance) const;
  Q_INVOKABLE QString readableSpeed(double speed) const;
  Q_INVOKABLE QString readableBearing(const QString& bearing) const;
  Q_INVOKABLE QString readableElevation(double elevation) const;
  Q_INVOKABLE QString panelElevation(double elevation) const;
  Q_INVOKABLE QString panelDurationHM(int seconds) const;
  Q_INVOKABLE QString panelDurationHMS(int seconds) const;
  Q_INVOKABLE QString readableDegreeGeocaching(double degree) const;
  Q_INVOKABLE QString readableDegree(double degree) const;
  Q_INVOKABLE QString readableCoordinatesGeocaching(double lat, double lon) const;
  Q_INVOKABLE QString readableCoordinatesNumeric(double lat, double lon) const;
  Q_INVOKABLE QString readableCoordinates(double lat, double lon) const;
  Q_INVOKABLE QString readableBytes(quint64 bytes) const;

  // Define singleton provider functions
  static QObject* createConverter(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Converter;
  }

signals:
  void systemChanged();
  void metersChanged();
  void kmChanged();
  void feetChanged();
  void milesChanged();
  void northChanged();
  void southChanged();
  void westChanged();
  void eastChanged();
  void northwestChanged();
  void northeastChanged();
  void southwestChanged();
  void southeastChanged();

private:
  QString getSystem() const;
  void setSystem(const QString& system);

  QString getMeters() const { return m_meters; }
  void setMeters(const QString& meters) { m_meters = meters; }
  QString getKm() const { return m_km; }
  void setKm(const QString& km) { m_km = km; }
  QString getFeet() const { return m_feet; }
  void setFeet(const QString& feet) { m_feet = feet; }
  QString getMiles() const { return m_miles; }
  void setMiles(const QString& miles) { m_miles = miles; }
  QString getNorth() const { return m_north; }
  void setNorth(const QString& north) { m_north = north; }
  QString getSouth() const { return m_south; }
  void setSouth(const QString& south) { m_south = south; }
  QString getWest() const { return m_west; }
  void setWest(const QString& west) { m_west = west; }
  QString getEast() const { return m_east; }
  void setEast(const QString& east) { m_east = east; }
  QString getNorthwest() const { return m_northwest; }
  void setNorthwest(const QString& northwest) { m_northwest = northwest; }
  QString getNortheast() const { return m_northeast; }
  void setNortheast(const QString& northeast) { m_northeast = northeast; }
  QString getSouthwest() const { return m_southwest; }
  void setSouthwest(const QString& southwest) { m_southwest = southwest; }
  QString getSoutheast() const { return m_southeast; }
  void setSoutheast(const QString& southeast) { m_southeast = southeast; }

  enum {
    SYSTEM_SI,
    SYSTEM_IMPERIAL,
  } m_system;

  QString m_meters;
  QString m_km;
  QString m_feet;
  QString m_miles;
  QString m_north;
  QString m_south;
  QString m_west;
  QString m_east;
  QString m_northwest;
  QString m_northeast;
  QString m_southwest;
  QString m_southeast;
};

#endif // CONVERTER_H
