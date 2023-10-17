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

class PlatformExtras : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool preventBlanking READ getPreventBlanking WRITE setPreventBlanking NOTIFY preventBlanking)

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

  static QString getHomeDir();
  static QString getDataDir(const char* appId);
  static QStringList getStorageDirs();

signals:
  void preventBlanking();

private:
  bool getPreventBlanking() const { return m_preventBlanking; };

  void setPreventBlanking(bool on);

  bool m_preventBlanking;

#ifdef Q_OS_LINUX
  uint m_cookie;
#endif
};

#endif // PLATFORM_EXTRAS_H
