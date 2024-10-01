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
#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QFile>
#include <QByteArray>

class Simulator;

class ScriptRunner : public QThread
{
  Q_OBJECT
public:
  explicit ScriptRunner(Simulator& s)
      : _simulator(s) { }
  ~ScriptRunner();

  bool configureScript(const QString& filepath);
  bool isRunAborted() { return (_file ? _aborted : false); }
  QString filepath() const;

  void run() override;

  void rollback();
  bool recoverable() const { return _redo; }

private:
  bool processCommand();

  bool _aborted = false;
  bool _redo = false;
  bool _finished = false;
  Simulator& _simulator;
  QFile * _file = nullptr;
  QByteArray _command;
};

#endif // SCRIPTRUNNER_H
