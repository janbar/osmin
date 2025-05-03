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

#include "converter.h"
#include <cstdlib>
#include <cmath>

#define STR_SYSTEM_SI       "SI"
#define STR_SYSTEM_IMPERIAL "Imperial"

Converter::Converter(QObject* parent)
: QObject(parent)
, m_system(SYSTEM_SI)
, m_meters("meters")
, m_km("km")
, m_feet("feet")
, m_miles("miles")
, m_north("north")
, m_south("south")
, m_west("west")
, m_east("east")
, m_northwest("northwest")
, m_northeast("northeast")
, m_southwest("southwest")
, m_southeast("southeast")
{
}

Converter::~Converter()
{
}

QStringList Converter::systems() const
{
  QStringList list;
  list.push_back(STR_SYSTEM_SI);
  list.push_back(STR_SYSTEM_IMPERIAL);
  return list;
}

QString Converter::readableDistance(double distance) const
{
  if (distance != distance)
    return QString("-");
  switch (m_system)
  {
  case SYSTEM_IMPERIAL:
    {
      double ft = distance * 3.2808;
      if (ft < 1000 )
        return QString("%1 %2").arg(std::round(ft), 0 ,'f', 0).arg(m_feet);
      double mi = distance / 1609.344;
      if (mi < 20)
        return QString("%1 %2").arg(std::round(mi * 10.0) / 10.0, 0 ,'f', 1).arg(m_miles);
      return QString("%1 %2").arg(std::round(mi), 0 ,'f', 0).arg(m_miles);
    }
  default:
    if (distance < 1000)
      return QString("%1 %2").arg(std::round(distance), 0, 'f', 0).arg(m_meters);
    if (distance < 10000)
      return QString("%1 %2").arg(std::round((distance / 1000.0) * 10.0) / 10.0, 0, 'f', 1).arg(m_km);
    return QString("%1 %2").arg(std::round(distance / 1000.0), 0, 'f', 0).arg(m_km);
  }
}

QString Converter::panelDistance(double distance) const
{
  if (distance != distance)
    return QString("-");
  switch (m_system)
  {
  case SYSTEM_IMPERIAL:
    {
      double ft = distance * 3.2808;
      if (ft < 1000 )
        return QString("%1 ft").arg(std::round(ft), 0 ,'f', 0);
      double mi = distance / 1609.344;
      if (mi < 20)
        return QString("%1 mi").arg(std::round(mi * 10.0) / 10.0, 0 ,'f', 1);
      return QString("%1 mi").arg(std::round(mi), 0 ,'f', 0);
    }
  default:
    if (distance < 1000)
      return QString("%1 m").arg(std::round(distance), 0, 'f', 0);
    if (distance < 10000)
      return QString("%1 km").arg(std::round((distance / 1000.0) * 10.0) / 10.0, 0, 'f', 1);
    return QString("%1 km").arg(std::round(distance / 1000.0), 0, 'f', 0);
  }
}

QString Converter::readableSpeed(double speed) const
{
  if (speed != speed)
    return QString("-");
  switch (m_system)
  {
  case SYSTEM_IMPERIAL:
    return QString("%1 mph").arg(std::round(speed * 1000.0 / 1609.344), 0, 'f', 0);
  default:
    return QString("%1 km/h").arg(std::round(speed), 0, 'f', 0);
  }
}

QString Converter::readableBearing(const QString& bearing) const
{
  if (bearing == "N")
    return m_north;
  if (bearing == "S")
    return m_south;
  if (bearing == "W")
    return m_west;
  if (bearing == "E")
    return m_east;
  if (bearing == "NW")
    return m_northwest;
  if (bearing == "NE")
    return m_northeast;
  if (bearing == "SW")
    return m_southwest;
  if (bearing == "SE")
    return m_southeast;
  return bearing;
}

QString Converter::readableElevation(double elevation) const
{
  if (elevation != elevation)
    return QString("-");
  switch (m_system)
  {
  case SYSTEM_IMPERIAL:
    {
      double ft = elevation * 3.2808;
      return QString("%1 %2").arg(std::round(ft), 0 ,'f', 0).arg(m_feet);
    }
  default:
    return QString("%1 %2").arg(std::round(elevation), 0, 'f', 0).arg(m_meters);
  }
}

QString Converter::panelElevation(double elevation) const
{
  if (elevation != elevation)
    return QString("-");
  switch (m_system)
  {
  case SYSTEM_IMPERIAL:
    {
      double ft = elevation * 3.2808;
      return QString("%1 ft").arg(std::round(ft), 0 ,'f', 0);
    }
  default:
    return QString("%1 m").arg(std::round(elevation), 0, 'f', 0);
  }
}

QString Converter::panelDurationHM(int seconds) const
{
  if (seconds < 0)
    return "?";
  uint m = (uint) std::floor((seconds % 3600) / 60.0);
  uint h = (uint) std::floor(seconds / 3600.0);
  return QString("%1:%2").arg((uint)h, (int)2, (int)10, QChar('0')).arg((uint)m, (int)2, (int)10, QChar('0'));
}

QString Converter::panelDurationHMS(int seconds) const
{
  if (seconds < 0)
    return "?";
  uint s = (uint) (seconds % 60);
  uint m = (uint) std::floor((seconds % 3600) / 60.0);
  uint h = (uint) std::floor(seconds / 3600.0);
  return QString("%1:%2:%3").arg((uint)h, (int)2, (int)10, QChar('0')).arg((uint)m, (int)2, (int)10, QChar('0')).arg((uint)s, (int)2, (int)10, QChar('0'));
}

QString Converter::readableDegreeGeocaching(double degree) const
{
  if (degree != degree)
    return QString("-");
  double minutes = (degree - std::floor(degree)) * 60.0;
  return QString("%1°%2'")
      .arg(std::floor(degree), 0, 'f', 0)
      .arg(minutes, 6, 'f', 3, '0');
}

QString Converter::readableDegreeDMS(double degree) const
{
  if (degree != degree)
    return QString("-");
  double minutes = (degree - std::floor(degree)) * 60.0;
  double seconds = (minutes - std::floor(minutes)) * 60.0;
  return QString("%1°%2'%3\"")
      .arg(std::floor(degree), 0, 'f', 0)
      .arg(std::floor(minutes), 2, 'f', 0, '0')
      .arg(seconds, 5, 'f', 2, '0');
}

QString Converter::readableDegree(double degree) const
{
  if (degree != degree)
    return QString("-");
  return QString("%1°").arg(std::round(degree), 0, 'f', 0);
}

QString Converter::readableCardinal(double degree) const
{
  if (degree != degree)
    return QString("-");
  double n = std::round(std::remainder(degree, 360.0) / 22.5);
  int c = (n < 0 ? int(n)+16 : int(n));
  switch(c)
  {
  case 0:
    return readableBearing("N");
  case 1:
    return readableBearing("N") + " " + readableBearing("NE");
  case 2:
    return readableBearing("NE");
  case 3:
    return readableBearing("E") + " " + readableBearing("NE");
  case 4:
    return readableBearing("E");
  case 5:
    return readableBearing("E") + " " + readableBearing("SE");
  case 6:
    return readableBearing("SE");
  case 7:
    return readableBearing("S") + " " + readableBearing("SE");
  case 8:
    return readableBearing("S");
  case 9:
    return readableBearing("S") + " " + readableBearing("SW");
  case 10:
    return readableBearing("SW");
  case 11:
    return readableBearing("W") + " " + readableBearing("SW");
  case 12:
    return readableBearing("W");
  case 13:
    return readableBearing("W") + " " + readableBearing("NW");
  case 14:
    return readableBearing("NW");
  case 15:
    return readableBearing("N") + " " + readableBearing("NW");
  default:
    return QString();
  }
}

QString Converter::readableCoordinatesGeocaching(double lat, double lon) const
{
  if (lat != lat || lon != lon)
    return QString("-");
  QString str;
  str.append(lat > 0 ? "N" : "S").append(" ").append(readableDegreeGeocaching(std::fabs(lat))).append(" ")
      .append(lon > 0 ? "E" : "W").append(" ").append(readableDegreeGeocaching(std::fabs(lon)));
  return str;
}

QString Converter::readableCoordinatesNumeric(double lat, double lon) const
{
  if (lat != lat || lon != lon)
    return QString("-");
  return QString("%1 %2").arg(lat, 0, 'f', 5, '0').arg(lon, 0, 'f', 5, '0');
}

QString Converter::readableCoordinates(double lat, double lon) const
{
  if (lat != lat || lon != lon)
    return QString("-");
  return QString("%1 %2 %3 %4")
      .arg(std::fabs(lat), 0, 'f', 5, '0').arg(lat > 0 ? "N" : "S")
      .arg(std::fabs(lon), 0, 'f', 5, '0').arg(lon > 0 ? "E" : "W");
}

QString Converter::readableBytes(quint64 bytes) const
{
  if (bytes < 0x400LL)
    return QString("%1 B").arg(bytes);
  double b = 1.0 * bytes;
  if (bytes < 0x100000LL)
    return QString("%1 KB").arg(std::round((b / 0x400LL) * 10.0) / 10.0, 0, 'f', 1);
  if (bytes < 0x40000000LL)
    return QString("%1 MB").arg(std::round((b / 0x100000LL) * 10.0) / 10.0, 0, 'f', 1);
  if (bytes < 0x10000000000LL)
    return QString("%1 GB").arg(std::round((b / 0x40000000LL) * 10.0) / 10.0, 0, 'f', 1);
  return QString("%1 TB").arg(std::round((b / 0x10000000000LL) * 10.0) / 10.0, 0, 'f', 1);
}

QString Converter::getSystem() const
{
  switch (m_system)
  {
  case SYSTEM_IMPERIAL: return STR_SYSTEM_IMPERIAL;
  case SYSTEM_SI: return STR_SYSTEM_SI;
  default: return STR_SYSTEM_SI;
  }
}

void Converter::setSystem(const QString& system)
{
  if (system == STR_SYSTEM_IMPERIAL)
    m_system = SYSTEM_IMPERIAL;
  else if (system == STR_SYSTEM_SI)
    m_system = SYSTEM_SI;
}
