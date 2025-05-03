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
#ifndef PLATFORM_EXTRAS_H
#define PLATFORM_EXTRAS_H

//#define Q_OS_ANDROID

#include <QObject>
#include <QQmlEngine>
#ifdef HAVE_DBUS
#include <QtDBus>
#include <QMap>
#endif
class PlatformExtras : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool preventBlanking READ getPreventBlanking NOTIFY preventBlanking)

public:
  explicit PlatformExtras(QObject *parent = nullptr);
  ~PlatformExtras();

  // Define singleton provider functions
  static QObject* createPlatformExtras(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new PlatformExtras;
  }

  /**
   * @brief Returns base path of user data.
   * The path is readable and writable by the user and the instance.
   * @return path
   */
  static QString getDataDir();

  /**
   * @brief Returns base path of application data
   * The path is readable and writable by the instance.
   * @return path
   */
  static QString getAppDir();

  /**
   * @brief Returns base path of installed asset.
   * The path is readable by the instance and should contain all resources
   * required by the initial run.
   * @param appId
   * @return path
   */
  static QString getAssetDir(const char* appId);

  /**
   * @brief Returns the list of storage volumes.
   * All are candidates for storing map databases.
   * @return list of path
   */
  static QStringList getStorageDirs();

  Q_INVOKABLE void setPreventBlanking(bool on, int mask);
  Q_INVOKABLE bool shareContent(const QString &text, const QString &path, const QString &mimeType);
  Q_INVOKABLE bool shareData(const QString &text, const QString &data, const QString &mimeType);

signals:
  void preventBlanking();

private:
  bool getPreventBlanking() const { return m_preventBlanking; };

  void doPreventBlanking(bool on);

  volatile bool m_preventBlanking;
  volatile int m_preventBlankingMask;

#ifdef HAVE_DBUS
  struct RemoteService {
    RemoteService(const QString& svc, const QString& path, const QString& iface)
    : interface(svc, path, iface), cookie(0) { }
    QDBusInterface interface;
    uint cookie;
  };
  QMap<QString, RemoteService*> m_remoteServices;
#endif
};

#endif // PLATFORM_EXTRAS_H
