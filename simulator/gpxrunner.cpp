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

#include "gpxrunner.h"

#include <utils.h>

#include <cmath>
#include <chrono>

#define PERIOD_DEFAULT  1000  /* millisec */
#define PERIOD_MAX      30000 /* millisec */

GPXRunner::~GPXRunner()
{
  if (this->isRunning())
    this->requestInterruption();
  this->wait();
  if (_running)
    delete _running;
  if (_gpxfile)
    delete _gpxfile;
}

bool GPXRunner::loadGPX(const QString &filepath)
{
  if (_gpxfile)
  {
    if (this->isRunning())
      this->requestInterruption();
    this->wait();
    if (_running)
      delete _running;
    delete _gpxfile;
    _running = nullptr;
    _gpxfile = nullptr;
  }
  GPXFile * file = new GPXFile();
  osmscout::gpx::ProcessCallbackRef cb(new Callback());
  if (!file->parse(filepath, cb))
  {
    delete file;
    return false;
  }
  _gpxfile = file;
  return true;
}

bool GPXRunner::configureRun(int trackid, int tick, double speed, int startpts)
{
  if (_gpxfile)
  {
    for (GPXObjectTrack& t : _gpxfile->tracks())
    {
      if (t.id() == trackid)
      {
        if (t.data().GetPointCount() <= (unsigned)startpts)
          return false;
        if (_running)
        {
          if (this->isRunning())
            this->requestInterruption();
          this->wait();
          delete _running;
        }
        _running = new Running(t, tick, speed, startpts);
        return true;
      }
    }
  }
  return false;
}

bool GPXRunner::processNextPoint(int * waitfor)
{
  if (!_running)
    return false;

  if (!_running->midway.isEmpty())
  {
    Point point = _running->midway.front();
    _running->midway.pop_front();
    fprintf(stdout, "[TRANSITION %d] %3.6f %3.6f %3.0f\n", _running->pts, point.lat, point.lon, point.alt);
    _position.resetData(point.lat, point.lon, point.alt);
    *waitfor = point.duration;
    return true;
  }

  quint64 ts0 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count();

  const osmscout::gpx::TrackPoint * point = _running->findPoint();
  if (point == nullptr)
    return false;

  // apply current point
  _position.resetData(
      point->coord.GetLat(),
      point->coord.GetLon(),
      point->elevation.value_or(0.0));
  emit pointChanged(_running->pts);

  quint64 tsP1 = 0;
  if (point->timestamp.has_value())
    tsP1 = std::chrono::duration_cast<std::chrono::milliseconds>(point->timestamp.value().time_since_epoch()).count();

  const osmscout::gpx::TrackPoint * nextpoint = _running->findPoint(_running->pts + 1);
  if (nextpoint == nullptr)
    return false;
  _running->pts += 1;

  quint64 tsP2 = 0;
  if (nextpoint->timestamp.has_value())
    tsP2 = std::chrono::duration_cast<std::chrono::milliseconds>(nextpoint->timestamp.value().time_since_epoch()).count();

  int diff = 0.0;
  if (tsP2 > tsP1)
    diff = (int) (tsP2 - tsP1);
  else
    diff = PERIOD_DEFAULT;

  // if diff exceed the max then truncate
  diff = (diff < PERIOD_MAX ? diff / _running->speed : PERIOD_MAX / _running->speed);

  // provide more points when the period is longer than update interval
  if (diff > _running->tick)
  {
    // the required count of transition
    int n = (diff / _running->tick);
    // the new period per transition
    diff /= (n + 1);
    // delta of one transition
    double distance = 0.0;
    double elevation = point->elevation.value_or(0.0);
    double vdist = (nextpoint->elevation.value_or(0.0) - elevation) / (n + 1);
    double hdist = osmin::Utils::sphericalDistance(point->coord.GetLat(),
                                                      point->coord.GetLon(),
                                                      nextpoint->coord.GetLat(),
                                                      nextpoint->coord.GetLon()
                                                   ) / (n + 1);
    // the direction to go
    double theta = osmin::Utils::sphericalBearingFinal(point->coord.GetLat(),
                                                         point->coord.GetLon(),
                                                         nextpoint->coord.GetLat(),
                                                         nextpoint->coord.GetLon());
    // add transitions to midway
    while (n > 0)
    {
      n -= 1;
      distance += hdist;
      elevation += vdist;
      Point mid;
      mid.alt = elevation;
      mid.duration = diff;
      osmin::Utils::sphericalTarget(point->coord.GetLat(), point->coord.GetLon(), theta, distance, &mid.lat, &mid.lon);
      _running->midway << mid;
    }
  }

  quint64 ts1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
  if (ts1 > ts0)
    *waitfor = diff - (int)(ts1 - ts0);
  else
    *waitfor = diff;
  return true;
}

void GPXRunner::run()
{
  if (!_running)
    return;
  _running->aborted = false;
  int wait = 0;
  while (!_running->aborted && processNextPoint(&wait))
  {
    while (wait > 0)
    {
      if (isInterruptionRequested())
      {
        _running->aborted = true;
        break;
      }
      msleep(10);
      wait -= 10;
    }
  }
}

void GPXRunner::Callback::Progress(double p)
{
  double n = round(p * 50.0);
  if (n > _p)
  {
    fputc('.', stdout);
    fflush(stdout);
    _p = n;
  }
}

void GPXRunner::Callback::Error(const std::string &error)
{
  fprintf(stdout, "Error: %s\n", error.c_str());
}

const osmscout::gpx::TrackPoint * GPXRunner::Running::findPoint(int pts) const
{
  int _seg = 0;
  int _idx = 0;
  for (auto& s : track.data().segments)
  {
    int _p = _idx + s.points.size();
    if (pts < _p)
      break;
    _idx = _p;
    _seg += 1;
  }
  _idx = pts - _idx;
  if (track.data().segments.size() > (size_t)_seg)
    return &track.data().segments[_seg].points[_idx];
  return nullptr;
}
