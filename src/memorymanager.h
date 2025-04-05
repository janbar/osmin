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
#include <QThread>
#include <QSharedPointer>

class MemoryManager;

typedef QSharedPointer<MemoryManager> MemoryManagerPtr;

class MemoryManager : public QObject
{
  Q_OBJECT

public:
  MemoryManager(uint16_t rss_target_mb);
  virtual ~MemoryManager();
  MemoryManager(const MemoryManager&) = delete;

  void terminate();

private slots:
  void run();
  void onFinished();

private:
  QThread * m_t;
  osmscout::DBThreadRef m_dbThread;
  size_t m_page_size;
  size_t m_rss_target;
  size_t m_rss_warn;
  size_t m_rss_usage;


  bool statMemoryUsage();
};

Q_DECLARE_METATYPE(MemoryManagerPtr)

#endif // MEMORYMANAGER_H
