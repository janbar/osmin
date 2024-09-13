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

#include "platformextras.h"

#include <QDir>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QDebug>

#if defined(SAILFISHOS)
#define AUTO_MOUNT        "/run/media/"

#elif defined(Q_OS_ANDROID)
#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>
#include <QCoreApplication>
#define AUTO_MOUNT        "/storage/"

#else
#define AUTO_MOUNT        "/media/"
#endif

PlatformExtras::PlatformExtras(QObject* parent)
: QObject(parent)
, m_preventBlanking(false)
, m_preventBlankingMask(0)
{
#ifdef HAVE_DBUS
  // register dbus service for inhibitor
  RemoteService* inhibitor = new RemoteService("org.freedesktop.ScreenSaver",
                                               "/org/freedesktop/ScreenSaver",
                                               "org.freedesktop.ScreenSaver");
  if (inhibitor->interface.isValid())
    m_remoteServices.insert("inhibitor", inhibitor);
  else
  {
    qWarning("Failed to register DBus interface for inhibitor");
    delete inhibitor;
  }
#endif
}

PlatformExtras::~PlatformExtras()
{
  // clear inhibitor lock
  doPreventBlanking(false);
#ifdef HAVE_DBUS
  // free registered DBus services
  for (const RemoteService* svc : std::as_const(m_remoteServices))
    delete svc;
  m_remoteServices.clear();
#endif
}

QString PlatformExtras::getDataDir()
{
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject nullstr = QJniObject::fromString("");
  QJniObject file = activity.callObjectMethod("getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;", nullstr.object<jstring>());
  QJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
  return path.toString();
#else
  return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
}

QString PlatformExtras::getAppDir()
{
#ifdef Q_OS_ANDROID
  // from Android 14, only internal storage can be used for resources
  // file descriptors can be hold during the life of the instance
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject file = activity.callObjectMethod("getFilesDir", "()Ljava/io/File;");
  QJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
  return path.toString();
#else
  return getDataDir();
#endif
}

QString PlatformExtras::getAssetDir(const char* appId)
{
#ifdef Q_OS_ANDROID
  return "assets:";
#else
  return QStandardPaths::locate(QStandardPaths::GenericDataLocation, appId, QStandardPaths::LocateDirectory);
#endif
}

QStringList PlatformExtras::getStorageDirs()
{
  QStringList dirs;
#ifdef Q_OS_ANDROID
  // from Android 14, only internal storage can be used for databases
  // file descriptors can be hold during the life of the instance
  dirs.push_back(getAppDir());
#else
  // search for mounted volumes
  for (const QStorageInfo& storage : QStorageInfo::mountedVolumes())
  {
    QString path = storage.rootPath();
    if (storage.isValid() && storage.isReady() && path.startsWith(AUTO_MOUNT))
    {
      qInfo("Found storage: %s", path.toUtf8().constData());
      dirs.push_back(path);
    }
  }
#endif
  return dirs;
}

void PlatformExtras::setPreventBlanking(bool on, int mask)
{
  if (on)
  {
    if (!m_preventBlanking)
      doPreventBlanking(true);
    m_preventBlankingMask |= mask;
  }
  else
  {
    m_preventBlankingMask &= (~ mask);
    if (m_preventBlankingMask == 0 && m_preventBlanking)
      doPreventBlanking(false);
  }
}

bool PlatformExtras::shareContent(const QString &text, const QString &path, const QString &mimeType)
{
#if defined(Q_OS_ANDROID)
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject jsText = QJniObject::fromString(text);
  QJniObject jsPath = QJniObject::fromString(path);
  QJniObject jsType = QJniObject::fromString(mimeType);
  jboolean ok = QJniObject::callStaticMethod<jboolean>("io/github/janbar/osmin/QtAndroidHelper",
                                                              "shareContent",
                                                              "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
                                                              activity.object(), jsText.object<jstring>(), jsPath.object<jstring>(), jsType.object<jstring>());
  if(ok) {
    return true;
  } else {
    return false;
  }
#else
  // not implemented
  (void)text;
  (void)path;
  (void)mimeType;
  return false;
#endif
}

void PlatformExtras::doPreventBlanking(bool on)
{
  if (m_preventBlanking == on)
    return;
  m_preventBlanking = on;
#if defined(Q_OS_ANDROID)
  {
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([on]
    {
      auto activity = QJniObject(QNativeInterface::QAndroidApplication::context());
      QJniObject::callStaticMethod<void>("io/github/janbar/osmin/QtAndroidHelper",
                                         "preventBlanking",
                                         "(Landroid/content/Context;Z)V",
                                         activity.object(), (on ? JNI_TRUE : JNI_FALSE));
    });
  }
#elif defined(HAVE_DBUS)
  {
    auto inhibitor = m_remoteServices.find("inhibitor");
    if (inhibitor == m_remoteServices.end())
    {
      // rollback
      m_preventBlanking = !on;
      return;
    }
    else if(on)
    {
      QDBusReply<uint> reply = inhibitor.value()->interface.call("Inhibit", "osmin", "navigation enabled");
      if (reply.isValid())
        inhibitor.value()->cookie = reply.value();
      else
      {
        qWarning("Inhibitor failed: %s", reply.error().message().toUtf8().constData());
        // rollback
        m_preventBlanking = !on;
        return;
      }
    }
    else
      inhibitor.value()->interface.call("UnInhibit", inhibitor.value()->cookie);
  }
#else
  qWarning("Inhibitor isn't implemented for this platform");
#endif
  qInfo("PreventBlanking: %s", (m_preventBlanking ? "true" : "false"));
  emit preventBlanking();
}
