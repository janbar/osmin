#ifndef GPXRUNNER_H
#define GPXRUNNER_H

#include <gpxfilemodel.h>
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
#include <QObject>
#include <QThread>
#include <QList>

class GPXRunner : public QThread
{
  Q_OBJECT

public:
  GPXRunner() { }
  ~GPXRunner();

  bool loadGPX(const QString& fileptah);
  GPXFile * file() { return _gpxfile; }
  bool configureRun(int trackid, int tick, double speed, int startpts);
  bool isRunAborted() { return (_running ? _running->aborted : false); }

signals:
  void pointChanged(int pts);

private slots:

private:

  struct Point
  {
    Point() { }
    Point(double _lat, double _lon, double _alt, double ms)
        : lat(_lat), lon(_lon), alt(_alt), duration(ms) { }
    double lat = 0.0;
    double lon = 0.0;
    double alt = 0.0;
    double duration = 0.0;
  };

  struct Running {
    explicit Running(GPXObjectTrack& _track, int _tick, double _speed, int _startpts)
        : track(_track), tick(_tick), speed(_speed), pts(_startpts), aborted(false) { }
    const osmscout::gpx::TrackPoint * findPoint(int pts) const;
    const osmscout::gpx::TrackPoint * findPoint() const { return findPoint(pts); }

    GPXObjectTrack track;
    int tick;
    double speed;
    int pts;
    QList<Point> midway;
    bool aborted;
  };

  GPXFile * _gpxfile = nullptr;
  Running * _running = nullptr;

  void run() override;
  bool processNextPoint(int * waitfor);

  /* file loader progress callback
   */
  class Callback : public osmscout::gpx::ProcessCallback
  {
  public:
    void Progress(double p) override;
    void Error(const std::string& error) override;
  private:
    double _p = 0.0;
  };
};

#endif // GPXRUNNER_H
