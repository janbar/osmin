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
#include <cassert>

#define PAUSE_TICK          10 /* millisec */
#define TOKEN_SEPARATOR     0x20
#define TOKEN_ENCAPSULATOR  0x22

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
  connect(_gpxrunner, SIGNAL(finished()), this, SLOT(onGPXFinished()));
}

Simulator::~Simulator()
{
  // running cmd must be terminated
  if (_cmd && _cmd->isRunning())
  {
    _prompt = false;
    _cmd->forceInterruption();
    _cmd->wait();
  }
  qDeleteAll(_scripts);
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
    connect(_cmd, SIGNAL(newCommand(QString)), this, SLOT(onCommand(QString)));
    connect(_cmd, SIGNAL(eof()), this, SLOT(onQuit()));
    _prompt = true; // enable prompt
    prompt();
  }
}

void Simulator::disableCLI()
{
  if (_cmd)
  {
    _prompt = false; // disable prompt
    disconnect(_cmd, SIGNAL(newCommand(QString)), this, SLOT(onCommand(QString)));
    disconnect(_cmd, SIGNAL(eof()), this, SLOT(onQuit()));
    _cmd = nullptr;
  }
}

bool Simulator::onKeyBreak()
{
  bool accepted = true;
  if (_gpxrunner->isRunning())
  {
    // stop gpx runner
    _gpxrunner->requestInterruption();
    accepted = false;
  }
  if (scriptRunning())
  {
    _scripts.front()->requestInterruption();
    accepted = false;
  }
  return accepted;
}

void Simulator::onCommand(QString line)
{
  QString token;
  QStringList tokens = tokenize(line.toStdString().c_str());
  if (!tokens.empty())
  {
    token = tokens.front();
    tokens.pop_front();
  }

  if (token.compare("EXIT", Qt::CaseInsensitive) == 0)
  {
    if (_prompt && _cmd)
      onQuit();
    else
      fprintf(stdout, "Invalid command\n");
    return;
  }

  // the following commands are welcome
  if (token.compare("BREAK", Qt::CaseInsensitive) == 0)
  {
    onKeyBreak();
  }
  else if (token.compare("HELP", Qt::CaseInsensitive) == 0)
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
                    "BREAK                      Stop playback of the running script\n"
                    "LOAD gpx                   Load the GPX file\n"
                    "LIST                       List all tracks contained in the loaded file\n"
                    "RUN trkid [speed [pts]]    Run the identified track of the loaded file\n"
                    "RUN                        Resume the stopped run\n"
                    "PLAY script                Play the script file\n"
                    "PLAY                       Resume the stopped playback\n"
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
  else if (token.compare("ANGLE", Qt::CaseInsensitive) == 0 && !tokens.empty())
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
    if (!tokens.empty())
      alt = tokens.front().toDouble();
    _position.resetData(lat, lon, alt);
  }
  else if (token.compare("MOVE", Qt::CaseInsensitive) == 0)
  {
    double dist = 0.0;
    if (!tokens.empty())
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
    // by default wait for the tick of position source
    int millisec = _positionSource->updateInterval();
    if (!tokens.empty())
      millisec = 1000 * tokens.front().toInt();
    if (millisec < 1000 || millisec > 59000)
      fprintf(stdout, "Invalid duration specified. A valid range is 1 to 59\n");
    else
    {
      if (!scriptRunning())
      {
        // in interactive mode, the prompt will be delayed until a timer expires
        QTimer::singleShot(millisec, this, &Simulator::prompt);
        return;
      }
      else
      {
        // start a waiting loop in the script thread
        while (millisec > 0 && !QThread::currentThread()->isInterruptionRequested())
        {
          QThread::msleep(PAUSE_TICK);
          millisec -= PAUSE_TICK;
        }
      }
    }
  }
  else if (token.compare("LOAD", Qt::CaseInsensitive) == 0 && !tokens.empty())
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
  else if (token.compare("RUN", Qt::CaseInsensitive) == 0 && !tokens.empty())
  {
    token = tokens.front();
    int trackid = token.toInt();
    if (trackid > 0)
    {
      tokens.pop_front();
      double speed = 1.0;
      int pts = 0;
      if (!tokens.empty())
      {
        token = tokens.front();
        speed = token.toDouble();
        if (speed <= 0)
          speed = 1.0;
        tokens.pop_front();
        if (!tokens.empty())
        {
          token = tokens.front();
          pts = token.toInt();
          if (pts < 0)
            pts = 0;
        }
      }
      if (scriptRunning() && _scripts.back()->recoverable())
      {
        // the previous run is recoverable, therefore there is nothing to configure
        _gpxrunner->run(); // run in the thread of the script
        if (_gpxrunner->isRunAborted())
          _scripts.back()->rollback();
        return;
      }
      else if (_gpxrunner->configureRun(trackid, _positionSource->updateInterval(), speed, pts))
      {
        fprintf(stdout, "Run is starting on track %d, interval=%d speed=%3f\n"
                        "Type CTRL+C to stop\n", trackid, _positionSource->updateInterval(), speed);
        if (!scriptRunning())
          _gpxrunner->start();
        else
        {
          _gpxrunner->run(); // run in the thread of the script
          if (_gpxrunner->isRunAborted())
            _scripts.back()->rollback();
        }
        return;
      }
      else
      {
        fprintf(stdout, "Run cannot be processed\n");
      }
    }
  }
  else if (token.compare("RUN", Qt::CaseInsensitive) == 0 && tokens.empty())
  {
    if (scriptRunning())
    {
      // discard command in script
      fprintf(stdout, "Invalid command\n");
      return;
    }
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
  else if (token.compare("PLAY", Qt::CaseInsensitive) == 0 && !tokens.empty())
  {
    token = tokens.front();
    ScriptRunner * s;
    // playing a child script
    if (scriptRunning())
    {
      if (!scriptAborted())
      {
        s = new ScriptRunner(*this); // start a new
        if (!s->configureScript(token))
        {
          fprintf(stdout, "Script cannot be processed\n");
          delete s;
          return;
        }
        if (loopDetected(*s))
        {
          qWarning("Loop detected: %s won't be played", token.toStdString().c_str());
          delete s;
          return;
        }
      }
      else
      {
        // recover the aborted child
        s = _aborted.back();
        _aborted.pop_back();
        // a child script is recoverable, so if there is one on the abort stack
        // then request a rollback
        if (!_aborted.empty())
          s->rollback();
      }
      // processing
      _scripts.push_back(s);
      s->run();
      _scripts.pop_back();
      // an aborted script can be resumed, so keep it back
      if (s->isRunAborted())
        _aborted.push_back(s);
      return;
    }

    // playing main script
    s = new ScriptRunner(*this);
    if (s->configureScript(token))
    {
      // clear old recovery
      qDeleteAll(_aborted);
      _aborted.clear();
      // the main script owns the runner thread,
      // and it will signal when finished
      connect(s, SIGNAL(finished()), this, SLOT(onScriptFinished()));
      fprintf(stdout, "Playback is starting\nType CTRL+C to stop\n");
      _prompt = false; // disable prompt for next commands
      _scripts.push_back(s);
      s->start();
      return;
    }
    fprintf(stdout, "Script cannot be processed\n");
    delete s;
  }
  else if (token.compare("PLAY", Qt::CaseInsensitive) == 0 && tokens.empty())
  {
    if (scriptRunning())
    {
      // discard command in script
      fprintf(stdout, "Invalid command\n");
      return;
    }
    if (scriptAborted())
    {
      fprintf(stdout, "Playback resumes\nType CTRL+C to stop\n");
      _prompt = false; // disable prompt for next commands
      _scripts.push_back(_aborted.back());
      _aborted.pop_back();
      // a child script is recoverable, so if there is one on the abort stack
      // then request a rollback
      if (!_aborted.empty())
        _scripts.back()->rollback();
      // restart the main script
      _scripts.front()->start();
      return;
    }
    fprintf(stdout, "No playback to resume\n");
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
  QString ang = _converter.readableDegreeDMS(_compassSource->reading()->value(0).toDouble());
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
  QString ang = _converter.readableDegreeDMS(_azimuth.data());
  fprintf(stdout, "%d: Pos %s Alt %s Ang %s\n",
          pts,
          pos.toStdString().c_str(),
          alt.toStdString().c_str(),
          ang.toStdString().c_str()
          );
}

void Simulator::onGPXFinished()
{
  if (_gpxrunner->isRunAborted())
    fprintf(stdout, "Run is stopped\n");
  else
    fprintf(stdout, "Run has finished\n");
  prompt();
}

void Simulator::onScriptFinished()
{
  if (scriptRunning() && _scripts.front()->isRunAborted())
  {
    fprintf(stdout, "Playback is stopped\n");
    _aborted.push_back(_scripts.front());
    _scripts.pop_front();
  }
  else
  {
    fprintf(stdout, "Playback has finished\n");
    delete _scripts.front();
    _scripts.pop_front();
    assert(scriptRunning() == false);
  }
  _prompt = true; // enable prompt
  prompt();
}

void Simulator::prompt()
{
  if (_prompt)
  {
    // it must wait the thread has finished or not started yet
    if (_cmd && _cmd->wait())
      _cmd->start();
    emit prompted();
  }
}

qreal Simulator::normalizeAzimuth(qreal azimuth)
{
  azimuth = std::remainder(azimuth, 360);
  return (azimuth < 0 ? azimuth + 360 : azimuth);
}

QStringList Simulator::tokenize(const char * buf)
{
  QStringList out;
  std::string line(buf);
  std::string::const_iterator pos = line.begin();
  if (pos == line.end())
    return out;
  std::string token;
  bool first =  true;
  bool encap = false;
  while (pos != line.end())
  {
    if (*pos == TOKEN_ENCAPSULATOR)
    {
      ++pos;
      if (encap)
      {
        if (pos == line.end() || *pos != TOKEN_ENCAPSULATOR)
        {
          encap = false;
          while (pos != line.end() && *pos != TOKEN_SEPARATOR) ++pos;
        }
        else
        {
          token.push_back(*pos);
          ++pos;
        }
      }
      else if (!first)
      {
        fprintf(stdout, "Invalid character in stream\n");
        out.clear();
        return out;
      }
      else
      {
        encap = true;
      }
    }
    else if (*pos == TOKEN_SEPARATOR && !encap)
    {
      // trim null token
      if (!token.empty())
      {
        out.push_back(QString::fromStdString(token));
        token.clear();
      }
      first = true;
      ++pos;
    }
    else
    {
      first = false;
      token.push_back(*pos);
      ++pos;
    }
  }
  if (encap)
  {
    fprintf(stdout, "Quoted string not terminated\n");
    out.clear();
  }
  else if (!token.empty())
    out.push_back(QString::fromStdString(token));
  return out;
}

bool Simulator::loopDetected(const ScriptRunner &script) const
{
  for (const ScriptRunner * s : std::as_const(_scripts))
  {
    if (s->filepath() == script.filepath())
      return true;
  }
  return false;
}
