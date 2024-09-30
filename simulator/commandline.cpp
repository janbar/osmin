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

#include "commandline.h"

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define PROMPT_STRING ">>> "
#define LINE_MAXSIZE  1024

CommandLine::CommandLine()
{
#ifdef HAVE_READLINE
  stifle_history(512);
#endif
  _buffer = new char[LINE_MAXSIZE];
  memset(_buffer, '\0', LINE_MAXSIZE);
}

CommandLine::~CommandLine()
{
  delete [] _buffer;
#ifdef HAVE_READLINE
  if (rl_line)
    free(rl_line);
  clear_history();
#endif
}

void CommandLine::forceInterruption() {
    this->terminate();
#ifdef HAVE_READLINE
    // cleanup the state of the terminal
    rl_cleanup_after_signal();
#endif
}

void CommandLine::run()
{
  int r = readstdin(_buffer, LINE_MAXSIZE);
  if (r > 0)
  {
    // a line must be terminated by nl
    _buffer[r - 1] = '\0';
    emit newCommand(QString(_buffer));
  }
  else
  {
    emit eof();
  }
}

#ifdef HAVE_READLINE
int CommandLine::readstdin(char *buf, size_t maxlen)
{
  unsigned n = 0;
  if (rl_pos)
  {
    if (*rl_pos)
    {
      /* fill a chunk from previous read */
      while (*rl_pos && n < maxlen)
      {
        char c = *rl_pos;
        buf[n] = c;
        ++rl_pos;
        ++n;
        if (c == '\n')
          return n;
      }
      if (n >= maxlen)
      {
        /* buffer is full */
        return maxlen;
      }
    }
    /* free old buffer */
    free(rl_line);
    rl_line = rl_pos = nullptr;
    buf[n++] = '\n';
    return n;
  }

  /* get a new line */
  rl_catch_signals = 0;
  rl_line = readline(PROMPT_STRING);
  /* if the line has any text in it */
  if (rl_line)
  {
    if (*rl_line)
    {
      /* save it on the history */
      add_history(rl_line);
      rl_pos = rl_line;
      while (*rl_pos && n < maxlen)
      {
        char c = *rl_pos;
        buf[n] = c;
        ++rl_pos;
        ++n;
        if (c == '\n')
          return n;
      }
      if (n >= maxlen)
      {
        /* buffer is full */
        return maxlen;
      }
    }
    /* free old buffer */
    free(rl_line);
    rl_line = rl_pos = nullptr;
    buf[n++] = '\n';
    return n;
  }
  /* EOF */
  return -1;
}
#else

#include <stdio.h>

#if (defined(_WIN32) || defined(_WIN64))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#include <WinSock2.h>
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef __WINDOWS__
#define LASTERROR WSAGetLastError()
#define ERRNO_INTR WSAEINTR
#else
#define LASTERROR errno
#define ERRNO_INTR EINTR
#endif

int CommandLine::readstdin(char *buf, size_t maxlen)
{
  size_t len = 0;
#ifndef __WINDOWS__
  fd_set fds;
#endif
  fputs(PROMPT_STRING, stdout);
  fflush(stdout);
  for (;;)
  {
#ifndef __WINDOWS__
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    int r = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (r > 0 && FD_ISSET(STDIN_FILENO, &fds))
#endif
    {
      int chr;
      while ((chr = getchar()) != EOF)
      {
        if (len == maxlen)
          break;
        buf[len++] = (char) chr;
        if (chr == '\n')
          break;
      }
      return len;
    }
#ifndef __WINDOWS__
    else if (r < 0)
    {
      if (LASTERROR == ERRNO_INTR)
        continue;
      else
        break;
    }
#endif
  }
  return -1;
}
#endif // HAVE_READLINE
