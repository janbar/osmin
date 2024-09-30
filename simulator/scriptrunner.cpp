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

#include "scriptrunner.h"
#include "simulator.h"

#include <QByteArray>
#include <QEventLoop>
#include <cassert>

#define LINE_MAXSIZE  1024

ScriptRunner::~ScriptRunner()
{
  if (isRunning())
  {
    this->requestInterruption();
    this->wait();
  }
  if (_file)
    delete _file;
}

bool ScriptRunner::configureScript(const QString &filepath)
{
  QFile file(filepath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    file.close();
    if (_file)
      delete _file;
    _file = new QFile(filepath);
    return true;
  }
  return false;
}

QString ScriptRunner::filepath() const
{
  if (_file)
    return _file->fileName();
  return QString();
}

void ScriptRunner::run()
{
  if (_file == nullptr)
    return;
  if (!_file->isOpen() && !_file->open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  _aborted = false;
  while (processCommand())
  {
    // check for thread interrupted, not this
    if (QThread::currentThread()->isInterruptionRequested())
    {
      _aborted = true;
      break;
    }
    // now the command is completed, therefore clear recover
    _redo = false;
  }
  if (!_aborted)
    _file->close();
}

void ScriptRunner::recover()
{
  qDebug("rollback %s : %s", filepath().toStdString().c_str(), _command.constData());
  _redo = true;
}

bool ScriptRunner::processCommand()
{
  assert(_file->isOpen() == true);
  // redo last aborted command or process next
  if (!_redo)
    _command = _file->readLine(LINE_MAXSIZE);

  if (_command.isEmpty() || _command.back() != '\n')
    return false;
  fprintf(stdout, "PLAY: %s", _command.constData());
  _simulator.onCommand(QString(_command.mid(0, _command.size()-1)));
  return true;
}
