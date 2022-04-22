/*
 * Copyright (C) 2020
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
#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <qsocketnotifier.h>
#include <signal.h>

class SignalHandler : public QObject
{
    Q_OBJECT
public:
  SignalHandler(QObject *parent = 0);
  virtual ~SignalHandler();

  bool catchSignal(int signal);
  void omitSignal(int signal);

signals:
  void catched(int signal);

private slots:
  void forward();

private:
  SignalHandler(const SignalHandler&);
  SignalHandler& operator=(const SignalHandler&);

  QSocketNotifier *m_notifier;

  static int m_pipe[2];
  static void handler(int signal, siginfo_t * info, void * data);
};

#endif // SIGNALHANDLER_H
