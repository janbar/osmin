/*
 * Copyright (C) 2025
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
#ifndef MEMORYMAANGER_H
#define MEMORYMAANGER_H

#include <osmscoutclient/DBThread.h>

#include <QObject>

class MemoryManagerThread;

class MemoryManager : public QObject
{
  Q_OBJECT

  MemoryManagerThread * m_t;

public:
  MemoryManager(const MemoryManager&) = delete;

  static void createInstance(uint16_t rss_target_mb);
  static MemoryManager * getInstance();
  static void freeInstance();

  void start();
  void terminate();
  void flushCaches(unsigned keep, bool trim = false);

private:
  MemoryManager(uint16_t rss_target_mb);
  ~MemoryManager();
  static MemoryManager * _instance;
};

#endif // MEMORYMANAGER_H
