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

#include "simulatedcompass.h"

#include <QDebug>

#include <time.h>

#define RADIANS_TO_DEGREES 57.2957795

char const * const SimulatedCompass::id("simulated.compass");
QCompassReading SimulatedCompass::_compassReading;

SimulatedCompass::SimulatedCompass(QSensor *sensor)
    : QSensorBackend(sensor)
{
  setReading<QCompassReading>(&_compassReading);
  addDataRate(1.0, 10.0);

  connect(&_updateTimer, &QTimer::timeout,
          this, &SimulatedCompass::onTimeout);
}

SimulatedCompass::~SimulatedCompass()
{
}

quint64 SimulatedCompass::produceTimestamp()
{
  struct timespec tv;
  int ok;

#ifdef CLOCK_MONOTONIC_RAW
  ok = clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
  if (ok != 0)
#endif
    ok = clock_gettime(CLOCK_MONOTONIC, &tv);
  Q_ASSERT(ok == 0);

  quint64 result = (tv.tv_sec * 1000000ULL) + (tv.tv_nsec * 0.001); // scale to microseconds
  return result;
}

void SimulatedCompass::start()
{
  if (!_updateTimer.isActive())
  {
    int interval = 1000;
    if (sensor()->dataRate() > 0)
    {
      interval /= sensor()->dataRate(); // rate is hertz
    }
    _updateTimer.setInterval(interval);
    _updateTimer.start();
  }
}

void SimulatedCompass::stop()
{
  if (_updateTimer.isActive())
  {
    _updateTimer.stop();
  }
}

void SimulatedCompass::resetData(qreal azimuth)
{
  _compassReading.setCalibrationLevel(1.0);
  _compassReading.setAzimuth(azimuth);
  _compassReading.setTimestamp(produceTimestamp());
}

void SimulatedCompass::onTimeout()
{
  newReadingAvailable();
}
