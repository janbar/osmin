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

#include "memorymanager.h"
#include "csvparser.h"

#include <osmscoutclientqt/OSMScoutQt.h>

#include <QDebug>

#include <unistd.h>
#ifdef HAVE_MALLOC_TRIM
#include <malloc.h>
#endif

MemoryManager::MemoryManager(uint16_t rss_target_mb)
    : m_t(new QThread())
{
  m_t->setObjectName("memorymanager");
  this->moveToThread(m_t);
  connect(m_t, &QThread::finished, this, &MemoryManager::onFinished);
  connect(m_t, &QThread::started, this, &MemoryManager::run);
  m_dbThread = osmscout::OSMScoutQt::GetInstance().GetDBThread();
  m_page_size = sysconf(_SC_PAGESIZE);
  m_rss_target = ((size_t)rss_target_mb << 20) / m_page_size;
  m_rss_warn = m_rss_target - (m_rss_target >> 3); // 0.88
  m_t->start();

  qInfo("Memory target: %u MB", (unsigned)((m_rss_target * m_page_size) >> 20));
  qInfo("Memory flushing threshold: %u MB", (unsigned)((m_rss_warn * m_page_size) >> 20));
}

MemoryManager::~MemoryManager()
{
  if (!m_t->isFinished())
    terminate();
  m_t->deleteLater();
  qInfo("%s", __FUNCTION__);
}

void MemoryManager::terminate()
{
  m_t->quit();
}

void MemoryManager::run()
{
  /*
   * collection of memory stat is performed every cycle
   * the possible states are low, target, or high, and the control cycle is
   * updated accordingly such as 10, 5 or 1 second.
   */
  unsigned cycle = 10;
  while (!m_t->isInterruptionRequested())
  {
    if (!statMemoryUsage())
      break;

    if (m_rss_usage > m_rss_target)
    {
      cycle = 1;
      qWarning("Flusing caches: rss=%u MB", (unsigned)((m_rss_usage * m_page_size) >> 20));
      m_dbThread->FlushCaches(std::chrono::seconds(cycle));
#ifdef HAVE_MALLOC_TRIM
      if (::malloc_trim(0) == 0)
      {
        qWarning("No memory can be released back to the system");
      }
#endif
    }
    else if (m_rss_usage > m_rss_warn)
    {
      cycle = 5;
      qDebug("Flusing caches: rss=%u MB", (unsigned)((m_rss_usage * m_page_size) >> 20));
      m_dbThread->FlushCaches(std::chrono::seconds(cycle));
    }
    else
    {
      cycle = 10;
    }

    // wait for a cycle or any interruption
    m_t->sleep(cycle);
  }
}

void MemoryManager::onFinished()
{
}

bool MemoryManager::statMemoryUsage()
{
  QFile inputFile("/proc/self/statm");
  if (!inputFile.open(QIODevice::ReadOnly))
  {
    qWarning("Can't open statm file");
    return false;
  }
  QString line = QTextStream(&inputFile).readLine();
  inputFile.close();

  if (line.isEmpty())
  {
    qWarning("Can't parse statm");
    return false;
  }
  osmin::CSVParser parser(0x20, 0x00);
  osmin::CSVParser::container values;
  parser.deserialize(values, line.toStdString());
  if (values.size() < 7)
  {
    qWarning("Can't parse statm");
    return false;
  }

  size_t res = ::strtoul (values[1].c_str(), NULL, 10);
  size_t shr = ::strtoul (values[2].c_str(), NULL, 10);
  m_rss_usage = (res - shr);
  //qDebug("statm: res=%u shr=%u use=%u", (unsigned)res, (unsigned)shr, (unsigned)m_rss_usage);
  return true;
}
