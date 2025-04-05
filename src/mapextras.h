/*
 * Copyright (C) 2022-2023
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
#ifndef MAP_EXTRAS_H
#define MAP_EXTRAS_H

#include <osmscoutclient/DBThread.h>

#include "locked.h" // for qt compat

#include <QObject>
#include <QQmlEngine>
#include <QMap>
#include <QList>

class MapExtras : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool dayLight READ getDayLight NOTIFY dayLightChanged)

public:
  explicit MapExtras(QObject *parent = nullptr);
  ~MapExtras();

  // Define singleton provider functions
  static QObject* createMapExtras(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new MapExtras;
  }


signals:
  void dayLightChanged();

public:
  Q_INVOKABLE void flushCaches(int seconds);

  /**
   * @brief Return the array of flags with properties { name: string, value: bool }
   * @return array of object
   */
  Q_INVOKABLE QVariantList getStyleFlags();

  /**
   * @brief Reload the style with the given flags
   * @param flags the array of flags { name: string, value: bool }
   */
  Q_INVOKABLE void reloadStyle(QVariantList flags);

  /**
   * @brief Set the given flag to the desired value
   * @param name the flag name
   * @param value the flag value
   */
  Q_INVOKABLE void setStyleFlag(const QString& name, bool value);

  /**
   * @brief Enable/Disable the flag 'daylight'
   * @param enable true to enable, else false
   */
  Q_INVOKABLE void setDaylight(bool enable);

  bool getDayLight() const { return m_dayLigth; }

  /**
   * @brief Add overlay for type and key
   * @param type
   * @param key
   * @return new id
   */
  Q_INVOKABLE int addOverlay(const QString& type, int key);

  /**
   * @brief Find overlays by type and key
   * @param type
   * @param key
   * @return list of registered ids
   */
  Q_INVOKABLE QList<int> findOverlays(const QString& type, int key);

  /**
   * @brief Find overlay keys by type
   * @param type
   * @return list of keys
   */
  Q_INVOKABLE QList<int> findOverlayKeys(const QString& type);

  /**
   * @brief Clear an overlay. The returned list of ids should be released
   * once cleanup is complete.
   * @see releaseOverlayIds()
   * @param type
   * @param key
   * @return list of cleared ids
   */
  Q_INVOKABLE QList<int> clearOverlays(const QString& type, int key);

  /**
   * @brief Release ids of a previously cleared overlay
   * @param ids list of id to be released
   */
  Q_INVOKABLE void releaseOverlayIds(const QList<int>& ids);

private:
  QMutex* m_overlayLock;
  typedef QMap<int, QList<int> > Overlay;
  QMap<QString, Overlay> m_overlays;
  QList<int> m_freedIds;
  int m_newId = 0;

  int getOverlayId();

  // style flags
  bool m_dayLigth = true;
};

#endif // MAP_EXTRAS_H
