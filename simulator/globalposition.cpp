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

#include "globalposition.h"

void GlobalPosition::resetData(double lat, double lon, double alt)
{
  {
    QGeoCoordinate coord(lat, lon, alt);
    std::lock_guard<std::mutex> g(_mutex);
    _info = QGeoPositionInfo(coord, QDateTime::currentDateTime());
  }
  emit dataUpdated();
}

QGeoPositionInfo GlobalPosition::data() const
{
  std::lock_guard<std::mutex> g(_mutex);
  return _info;
}
