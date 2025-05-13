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

#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QDebug>

#include <cassert>
#include <unistd.h>
#ifdef HAVE_MALLOC_TRIM
#include <malloc.h>
#endif

class MemoryManagerThread : public QThread
{
  QMutex m_lock;
  QTimer m_timer;
  osmscout::DBThreadRef m_dbThread;
  size_t m_page_size;
  size_t m_rss_target = 0;
  size_t m_rss_warn = 0;
  size_t m_rss_usage = 0;

public:
  MemoryManagerThread() : QThread()
  {
    m_timer.moveToThread(this);
    m_dbThread = osmscout::OSMScoutQt::GetInstance().GetDBThread();
    m_page_size = sysconf(_SC_PAGESIZE);
  }

  ~MemoryManagerThread() {}

  void setTarget(uint16_t rss_target_mb)
  {
    QMutexLocker g(&m_lock);
    m_rss_target = ((size_t)rss_target_mb << 20) / m_page_size;
    m_rss_warn = m_rss_target - (m_rss_target >> 3); // 0.88
  }

  void flush(int keep, bool trim)
  {
    QMutexLocker g(&m_lock);
    flush_unlocked(keep, trim);
  }

protected:
  void run() override
  {
    qInfo("Memory target: %u MB", (unsigned)((m_rss_target * m_page_size) >> 20));
    qInfo("Memory flushing threshold: %u MB", (unsigned)((m_rss_warn * m_page_size) >> 20));

    connect(&m_timer, &QTimer::timeout, this, &MemoryManagerThread::onTimeout, Qt::DirectConnection);
    m_timer.setInterval(30000);
    m_timer.setSingleShot(false);
    m_timer.start();
    this->exec();
    disconnect(&m_timer, &QTimer::timeout, this, &MemoryManagerThread::onTimeout);
    m_timer.stop();
    // wait unlocked
    QMutexLocker g(&m_lock);
  }

private slots:
  void onTimeout()
  {
    QMutexLocker g(&m_lock);
    /*
     * collection of memory stat is performed every cycle
     * the possible states are low, target, or high, and the control cycle is
     * updated accordingly such as 30, 5 or 1 second.
     */
    if (!stat_unlocked())
    {
      this->quit(); // stats not available
    }
    else if (m_rss_target > 0)
    {
      if (m_rss_usage > m_rss_target)
      {
        flush_unlocked(5, true);
        m_timer.setInterval(5000);
      }
      else if (m_rss_usage > m_rss_warn)
      {
        flush_unlocked(30, false);
        m_timer.setInterval(15000);
      }
      else
      {
        m_timer.setInterval(30000);
      }
    }
  }

private:
  bool stat_unlocked()
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

  void flush_unlocked(int keep, bool trim)
  {
    unsigned k = (keep > 0 ? (keep < 60 ? keep : 60) : 1);
    qWarning("Flushing caches: rss=%u MB", (unsigned)((m_rss_usage * m_page_size) >> 20));
    m_dbThread->FlushCaches(std::chrono::seconds(k));
#ifdef HAVE_MALLOC_TRIM
    if (trim && ::malloc_trim(0) == 0)
    {
      qWarning("No memory can be released back to the system");
    }
#endif
  }
};

MemoryManager * MemoryManager::_instance = nullptr;

MemoryManager::MemoryManager(uint16_t rss_target_mb)
    : m_t(new MemoryManagerThread)
{
  m_t->setObjectName("memorymanager");
  m_t->setTarget(rss_target_mb);
}

void MemoryManager::createInstance(uint16_t rss_target_mb)
{
  assert(_instance == nullptr);
  if (_instance == nullptr)
    _instance = new MemoryManager(rss_target_mb);
}

MemoryManager * MemoryManager::getInstance()
{
  return _instance;
}

void MemoryManager::freeInstance()
{
  assert(_instance != nullptr);
  if (_instance != nullptr)
  {
    delete _instance;
    _instance = nullptr;
  }
}

void MemoryManager::start()
{
  if (m_t->isRunning())
    return;
  m_t->start();
}

void MemoryManager::terminate()
{
  m_t->quit();
  m_t->wait();
}

MemoryManager::~MemoryManager()
{
  if (!m_t->isFinished())
    terminate();
  delete m_t;
  qInfo("%s", __FUNCTION__);
}

void MemoryManager::flushCaches(unsigned keep, bool trim /*= false*/)
{
  m_t->flush(keep, trim);
}

#if defined(Q_OS_ANDROID)
#include <jni.h>

static void onTrimMemory(JNIEnv *, jobject, jint keep)
{
  MemoryManager * mm = MemoryManager::getInstance();
  if (mm)
  {
    mm->flushCaches((unsigned)keep, true);
  }
}

static JNINativeMethod methods[] =
{
  { "onTrimMemory", "(I)V", (void *)onTrimMemory },
};

JNIEXPORT jint JNI_OnLoad(JavaVM * vm, void* /*reserved*/)
{
  JNIEnv* env;
  // get the JNIEnv pointer.
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    return JNI_ERR;

  // search for Java class which declares the native methods
  jclass javaClass = env->FindClass("io/github/janbar/osmin/JNI");
  if (!javaClass)
    return JNI_ERR;

  // register our native methods
  if (env->RegisterNatives(javaClass, methods, sizeof(methods) / sizeof(methods[0])) < 0)
    return JNI_ERR;

  return JNI_VERSION_1_6;
}
#endif
