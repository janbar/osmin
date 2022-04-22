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

#include <unistd.h>
#include <sys/socket.h>

#include "signalhandler.h"
#include <qdebug.h>

int SignalHandler::m_pipe[] = {0,0};

SignalHandler::SignalHandler(QObject *parent)
: QObject(parent)
{
  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_pipe))
     qFatal("Couldn't create signal socketpair.");
  else
  {
    m_notifier = new QSocketNotifier(m_pipe[1], QSocketNotifier::Read, this);
    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(forward()));
    qDebug("Signal handler is enabled.");
  }
}

SignalHandler::~SignalHandler()
{
  if (m_notifier)
  {
    disconnect(m_notifier, SIGNAL(activated(int)), this, SLOT(forward()));
    delete m_notifier;
  }
  qDebug("Signal handler is destroyed.");
}

bool SignalHandler::catchSignal(int signal)
{
  struct sigaction act;
  memset(&act, '\0', sizeof(struct sigaction));
  act.sa_sigaction = &handler;
  act.sa_flags |= SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  return (sigaction(signal, &act, 0) == 0);
}

void SignalHandler::omitSignal(int signal)
{
  struct sigaction act;
  memset(&act, '\0', sizeof(struct sigaction));
  act.sa_handler = SIG_DFL;
  sigemptyset(&act.sa_mask);
  sigaction(signal, &act, 0);
}

void SignalHandler::forward()
{
  int signal;
  m_notifier->setEnabled(false);
  size_t r = ::read(m_pipe[1], &signal, sizeof(signal));
  (void)r;
  emit catched(signal);
  m_notifier->setEnabled(true);
}

void SignalHandler::handler(int signal, siginfo_t * info, void * data)
{
  size_t w = ::write(m_pipe[0], &signal, sizeof(signal));
  (void)w;
  (void)info;
  (void)data;
}

