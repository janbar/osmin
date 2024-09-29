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
#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QThread>
#include <QList>
#include <QString>

class CommandLine : public QThread
{
  Q_OBJECT

signals:
  void newCommand(QStringList tokens);
  void eof();

public:
  CommandLine();
  ~CommandLine();

private:
  char * _buffer;

  void run();
  QStringList tokenize(const char *delimiters, bool trimnull = false);

  int readstdin(char *buf, size_t maxlen);
#ifdef HAVE_READLINE
  char * rl_line = nullptr; // hold the line
  char * rl_pos = nullptr;  // read position
#endif
};

#endif // COMMANDLINE_H
