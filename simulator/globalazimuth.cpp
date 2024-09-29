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

#include "globalazimuth.h"

void GlobalAzimuth::resetData(qreal azimuth)
{
  std::lock_guard<std::mutex> g(_mutex);
  _azimuth = azimuth;
}

qreal GlobalAzimuth::data() const
{
  std::lock_guard<std::mutex> g(_mutex);
  return _azimuth;
}
