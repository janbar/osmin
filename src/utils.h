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
#ifndef OSMINUTILS_H
#define OSMINUTILS_H

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace osmin
{

  class Utils : public QObject
  {
    Q_OBJECT

  public:
    explicit Utils(QObject *parent = nullptr);
    ~Utils() { }

    Q_INVOKABLE static QString normalizedInputString(const QString& str);

    Q_INVOKABLE static quint64 storageBytesFree(const QString& path);

    Q_INVOKABLE static double sphericalDistance(double aLat, double aLon, double bLat, double bLon);

    Q_INVOKABLE static double sphericalBearingFinal(double aLat, double aLon, double bLat, double bLon);

    static void sphericalTarget(double aLat, double aLon, double bearing, double distance, double * bLat, double * bLon);

    Q_INVOKABLE static QVariantMap sphericalTarget(double aLat, double aLon, double bearing, double distance);
  };

}
#endif /* OSMINUTILS_H */
