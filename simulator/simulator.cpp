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

#include "simulator.h"
#include <utils.h>

#include <QCoreApplication>
#include <QDebug>
#include <cmath>
#include <cstdio>

Simulator::Simulator()
    : QObject(nullptr), _azimuth(), _position()
{
  // setup the simulated compass sensor
  _compassPlugin = new SimulatedSensorPlugin(_azimuth);
  _compassPlugin->registerSensors();
  _compassSource = new SimulatedSensor();
  // setup the simulated position source
  _positionSource = new SimulatedPositionSource(_position);

  _compassSource->start();
  _positionSource->startUpdates();

  _gpxrunner = new GPXRunner(_position, _azimuth);
  connect(_gpxrunner, SIGNAL(pointChanged(int)), this, SLOT(onPointChanged(int)));
  connect(_gpxrunner, SIGNAL(finished()), this, SLOT(onRunFinished()));
}

Simulator::~Simulator()
{
  delete _gpxrunner;
  delete _positionSource;
  delete _compassPlugin;
  delete _compassSource;
}

void Simulator::enableCLI(CommandLine * cmd)
{
  if (cmd != nullptr)
  {
    _cmd = cmd;
    connect(_cmd, SIGNAL(newCommand(QStringList)), this, SLOT(onCommand(QStringList)));
    connect(_cmd, SIGNAL(eof()), this, SLOT(onQuit()));
    prompt();
  }
}

void Simulator::disableCLI()
{
  if (_cmd)
  {
    disconnect(_cmd, SIGNAL(newCommand(QStringList)), this, SLOT(onCommand(QStringList)));
    disconnect(_cmd, SIGNAL(eof()), this, SLOT(onQuit()));
    _cmd = nullptr;
  }
}

bool Simulator::onKeyBreak()
{
  if (!_gpxrunner->isRunning())
    return true;
  _gpxrunner->requestInterruption();
  return false;
}

void Simulator::onCommand(QStringList tokens)
{
  QString token;
  while (!tokens.isEmpty())
  {
    token = tokens.front();
    tokens.pop_front();
    // break on token not empty
    if (!token.isEmpty())
      break;
  }

  if (token.compare("EXIT", Qt::CaseInsensitive) == 0)
  {
    onQuit();
    return;
  }

  // when runner is running, accept only the commands below
  if (_gpxrunner->isRunning())
  {
    if (token.compare("BREAK", Qt::CaseInsensitive) == 0)
    {
      if (_gpxrunner->isRunning())
        _gpxrunner->requestInterruption();
    }
    // do not prompt here
    return;
  }

  // else the following commands are welcome
  if (token.compare("HELP", Qt::CaseInsensitive) == 0)
  {
    fprintf(stdout, "Commands:\n"
                    "HELP                       Print this help\n"
                    "EXIT                       Quit the simulator\n"
                    "STATUS                     Print current state\n"
                    "GOTO lat lon [alt]         Move to position\n"
                    "LEFT                       Rotate left\n"
                    "RIGHT                      Rotate right\n"
                    "ANGLE deg                  Rotate at angle of deg\n"
                    "MOVE [dm]                  Move forward dm or 0 (meters)\n"
                    "PAUSE [sec]                Pause for a tick or duration (1..59 seconds)\n"
                    "LOAD gpx                   Load the GPX file\n"
                    "LIST                       List all tracks contained in the loaded file\n"
                    "RUN trkid [speed [pts]]    Run the identified track of the loaded file\n"
                    "RUN                        Resume the stopped run\n"
                    "\n");
  }
  else if (token.compare("STATUS", Qt::CaseInsensitive) == 0)
  {
    onStatusRequested();
  }
  else if (token.compare("LEFT", Qt::CaseInsensitive) == 0)
  {
    _azimuth.resetData(normalizeAzimuth(_azimuth.data() - 90));
  }
  else if (token.compare("RIGHT", Qt::CaseInsensitive) == 0)
  {
    _azimuth.resetData(normalizeAzimuth(_azimuth.data() + 90));
  }
  else if (token.compare("ANGLE", Qt::CaseInsensitive) == 0 && !tokens.isEmpty())
  {
    qreal angle = tokens.front().toFloat();
    _azimuth.resetData(normalizeAzimuth(angle));
  }
  else if (token.compare("GOTO", Qt::CaseInsensitive) == 0 && tokens.length() >= 2)
  {
    double lat = tokens.front().toDouble();
    tokens.pop_front();
    double lon = tokens.front().toDouble();
    tokens.pop_front();
    double alt = _position.data().coordinate().altitude();
    if (!tokens.isEmpty())
      alt = tokens.front().toDouble();
    _position.resetData(lat, lon, alt);
  }
  else if (token.compare("MOVE", Qt::CaseInsensitive) == 0)
  {
    double dist = 0.0;
    if (!tokens.isEmpty())
      dist = tokens.front().toDouble();
    QGeoCoordinate coord = _position.data().coordinate();
    double bearing = M_PI * _azimuth.data() / 180.0;
    double lat, lon;
    osmin::Utils::sphericalTarget(coord.latitude(), coord.longitude(), bearing, dist,
                                  &lat, &lon);
    _position.resetData(lat, lon, coord.altitude());
  }
  else if (token.compare("PAUSE", Qt::CaseInsensitive) == 0)
  {
    if (tokens.isEmpty())
      QThread::msleep(_positionSource->updateInterval());
    else
    {
      int sec = tokens.front().toInt();
      if (sec > 0 && sec < 60)
        QThread::sleep(sec);
      else
        fprintf(stdout, "Invalid duration specified. A valid range is 1 to 59\n");
    }
  }
  else if (token.compare("LOAD", Qt::CaseInsensitive) == 0 && !tokens.isEmpty())
  {
    token = tokens.front();
    if (_gpxrunner->loadGPX(token))
    {
      fprintf(stdout, "\nFile load succeeded.\n");
      onListGPXRequested();
    }
  }
  else if (token.compare("LIST", Qt::CaseInsensitive) == 0)
  {
    if (_gpxrunner->file() == nullptr)
      fprintf(stdout, "No file loaded. Type LOAD, to load a file GPX\n");
    else
      onListGPXRequested();
  }
  else if (token.compare("RUN", Qt::CaseInsensitive) == 0 && !tokens.isEmpty())
  {
    token = tokens.front();
    int trackid = token.toInt();
    if (trackid > 0)
    {
      tokens.pop_front();
      double speed = 1.0;
      int pts = 0;
      if (!tokens.isEmpty())
      {
        token = tokens.front();
        speed = token.toDouble();
        if (speed <= 0)
          speed = 1.0;
        tokens.pop_front();
        if (!tokens.isEmpty())
        {
          token = tokens.front();
          pts = token.toInt();
          if (pts < 0)
            pts = 0;
        }
      }
      if (_gpxrunner->configureRun(trackid, _positionSource->updateInterval(), speed, pts))
      {
        fprintf(stdout, "Run is starting on track %d, interval=%d speed=%3f\n"
                        "Type CTRL+C to stop\n", trackid, _positionSource->updateInterval(), speed);
        _gpxrunner->start();
        return;
      }
      else
      {
        fprintf(stdout, "Run cannot be processed\n");
      }
    }
  }
  else if (token.compare("RUN", Qt::CaseInsensitive) == 0 && tokens.isEmpty())
  {
    if (_gpxrunner->isRunAborted())
    {
      fprintf(stdout, "Run resumes\nType CTRL+C to stop\n");
      _gpxrunner->start();
      return;
    }
    else
    {
      fprintf(stdout, "No run to resume\n");
    }
  }
  else if (!token.isEmpty())
  {
    fprintf(stdout, "Invalid command: Type HELP, for a list of commands\n");
  }

  prompt();
}

void Simulator::onQuit()
{
  QCoreApplication::quit();
}

void Simulator::onListGPXRequested()
{
  GPXFile * f = _gpxrunner->file();
  if (f == nullptr)
    return;
  fprintf(stdout, "Path: %s\nName: %s\n",
          f->path().toStdString().c_str(),
          f->name().toStdString().c_str()
          );
  for (GPXObjectTrack& t : f->tracks())
  {
    fprintf(stdout, "Track %d: %u pts, %s [%s]\n",
            t.id(),
            (unsigned) t.data().GetPointCount(),
            _converter.readableDistance(t.length()).toStdString().c_str(),
            t.name().toStdString().c_str()
            );
  }
}

void Simulator::onStatusRequested()
{
  // read info from sources
  QGeoCoordinate coord = _positionSource->lastKnownPosition().coordinate();
  QString pos = _converter.readableCoordinatesNumeric(coord.latitude(), coord.longitude());
  QString alt = _converter.readableElevation(coord.altitude());
  QString ang = _converter.readableDegree(_compassSource->reading()->value(0).toDouble());
  fprintf(stdout, "Pos %s Alt %s Ang %s\n",
          pos.toStdString().c_str(),
          alt.toStdString().c_str(),
          ang.toStdString().c_str()
          );
}

void Simulator::onPointChanged(int pts)
{
  // read info from backend
  QGeoCoordinate coord = _position.data().coordinate();
  QString pos = _converter.readableCoordinatesNumeric(coord.latitude(), coord.longitude());
  QString alt = _converter.readableElevation(coord.altitude());
  QString ang = _converter.readableDegree(_azimuth.data());
  fprintf(stdout, "%d: Pos %s Alt %s Ang %s\n",
          pts,
          pos.toStdString().c_str(),
          alt.toStdString().c_str(),
          ang.toStdString().c_str()
          );
}

void Simulator::onRunFinished()
{
  if (_gpxrunner->isRunAborted())
    fprintf(stdout, "Run is stopped\n");
  else
    fprintf(stdout, "Run has finished\n");
  prompt();
}

void Simulator::prompt()
{
  // it must wait the thread has finished or not started yet
  if (_cmd && _cmd->wait())
    _cmd->start();
}

qreal Simulator::normalizeAzimuth(qreal azimuth)
{
  azimuth = std::remainder(azimuth, 360);
  return (azimuth < 0 ? azimuth + 360 : azimuth);
}
