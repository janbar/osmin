/*
 * Copyright (C) 2022
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

#include "mapextras.h"
#include <osmscoutclientqt/OSMScoutQt.h>

MapExtras::MapExtras(QObject *parent)
: QObject(parent)
, m_overlayLock(QMutex::NonRecursive)
{
}

MapExtras::~MapExtras()
{
}

QVariantList MapExtras::getStyleFlags()
{
  QVariantList list;
  osmscout::DBThreadRef dbThread = osmscout::OSMScoutQt::GetInstance().GetDBThread();
  if (dbThread->isInitialized())
  {
    QMap<QString, bool> map = dbThread->GetStyleFlags();
    for (auto it = map.constKeyValueBegin(); it != map.constKeyValueEnd(); ++it)
    {
      // do not return internal flags
      if ((*it).first.startsWith('_'))
        continue;
      QVariantMap flag;
      flag.insert("name", QVariant::fromValue((*it).first));
      flag.insert("value", QVariant::fromValue((*it).second));
      list.append(flag);
    }
  }
  return list;
}

void MapExtras::reloadStyle(QVariantList flags)
{
  osmscout::DBThreadRef dbThread = osmscout::OSMScoutQt::GetInstance().GetDBThread();
  if (dbThread->isInitialized())
  {
    std::unordered_map<std::string, bool> map;
    for (auto it = flags.constBegin(); it != flags.constEnd(); ++it)
    {
      QVariantMap flag = it->toMap();
      map.emplace(flag.take("name").toString().toStdString(), flag.take("value").toBool());
    }
    dbThread->LoadStyle(dbThread->GetStylesheetFilename(), map);
  }
}

void MapExtras::setStyleFlag(const QString& name, bool value)
{
  osmscout::DBThreadRef dbThread = osmscout::OSMScoutQt::GetInstance().GetDBThread();
  if (dbThread->isInitialized())
    dbThread->SetStyleFlag(name, value);
}

void MapExtras::setDaylight(bool enable)
{
  osmscout::DBThreadRef dbThread = osmscout::OSMScoutQt::GetInstance().GetDBThread();
  if (dbThread->isInitialized())
    dbThread->SetStyleFlag("daylight", enable);
}

int MapExtras::addOverlay(const QString& type, int key)
{
  QMutexLocker guard(&m_overlayLock);
  auto im = m_overlays.find(type);
  if (im == m_overlays.end())
    im = m_overlays.insert(type, Overlay());
  auto ik = im.value().find(key);
  if (ik == im.value().end())
    ik = im.value().insert(key, QList<int>());
  int id = getOverlayId();
  ik.value().push_back(id);
  return id;
}

QList<int> MapExtras::findOverlays(const QString& type, int key)
{
  QMutexLocker guard(&m_overlayLock);
  auto im = m_overlays.find(type);
  if (im == m_overlays.end())
    return QList<int>();
  auto ik = im.value().find(key);
  if (ik == im.value().end())
    return QList<int>();
  return ik.value();
}

QList<int> MapExtras::findOverlayKeys(const QString &type)
{
  QMutexLocker guard(&m_overlayLock);
  auto im = m_overlays.find(type);
  if (im == m_overlays.end())
    return QList<int>();
  return im.value().keys();
}

QList<int> MapExtras::clearOverlays(const QString& type, int key)
{
  QMutexLocker guard(&m_overlayLock);
  QList<int> ids;
  auto im = m_overlays.find(type);
  if (im == m_overlays.end())
    return ids;
  auto ik = im.value().find(key);
  if (ik == im.value().end())
    return ids;
  ids.swap(ik.value());
  im->remove(key);
  return ids;
}

void MapExtras::releaseOverlayIds(const QList<int>& ids)
{
  QMutexLocker guard(&m_overlayLock);
  // fill list of reusable ids
  for (int id : ids)
  {
    if (id >= 0 && id < m_newId)
      m_freedIds.push_back(id);
  }
}

int MapExtras::getOverlayId()
{
  if (m_freedIds.empty())
    return m_newId++;
  int id = m_freedIds.back();
  m_freedIds.pop_back();
  return id;
}
