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
#ifndef GLOBALAZIMUTH_H
#define GLOBALAZIMUTH_H

#include <mutex>
#include <QCompassReading>

class GlobalAzimuth
{
public:
  GlobalAzimuth() { }
  ~GlobalAzimuth() = default;

  void resetData(qreal azimuth);
  qreal data() const;

private:
  mutable std::mutex _mutex;
  qreal _azimuth = 0.0;
};

#endif // GLOBALAZIMUTH_H
