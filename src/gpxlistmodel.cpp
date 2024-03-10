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

#include "gpxlistmodel.h"

#include <cstdint>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

#define TREE_DEPTH    2

inline uint_fast32_t __hashvalue(uint_fast32_t maxsize, const QString& str)
{
  /*
   * DJB Hash Function
   */
  uint_fast32_t h = 5381;
  auto it = str.constBegin();
  while (it != str.constEnd())
  {
    h = ((h << 5) + h) + it->unicode();
    ++it;
  }
  return h % maxsize;
}

GPXItem::GPXItem()
: m_dir(false)
, m_name()
, m_path()
, m_timestamp()
, m_relativeFilePath()
{
}

GPXItem::GPXItem(const QString& name, const QString& path, const QDateTime& timestamp, bool dir)
: m_dir(dir)
, m_name(name)
, m_path(path)
, m_timestamp(timestamp)
{
  m_relativeFilePath = path;
  m_relativeFilePath.append(QDir::separator()).append(name);
}

bool GPXItem::isValid() const
{
  return !m_relativeFilePath.isEmpty();
}

void GPXItem::setName(const QString& name)
{
  m_name = name;
  m_relativeFilePath = m_path;
  m_relativeFilePath.append(QDir::separator()).append(name);
}

int GPXItem::bigId() const
{
  return static_cast<int>(__hashvalue(0x7fffffff, m_relativeFilePath));
}

GPXListModel::GPXListModel(QObject* parent)
  : QAbstractListModel(parent)
, m_lock(nullptr)
, m_dataState(DataStatus::DataBlank)
, m_root()
{
  m_lock = new QRecursiveMutex();
}

GPXListModel::~GPXListModel()
{
  qDeleteAll(m_items);
  m_items.clear();
  delete m_lock;
}

int GPXListModel::rowCount(const QModelIndex& parent) const
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  QString path("."); // base path
  if (parent.isValid())
  {
    GPXItem* parentItem = static_cast<GPXItem*>(parent.internalPointer());
    path = QDir(parentItem->path()).filePath(parentItem->name());
  }
  QList<GPXItem*> candidates = findChildrenByPath(path);
  return candidates.size();
}

QModelIndex GPXListModel::index(int row, int column, const QModelIndex& parent) const
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  QString path("."); // base path
  if (parent.isValid())
  {
    GPXItem* parentItem = static_cast<GPXItem*>(parent.internalPointer());
    path = QDir(parentItem->path()).filePath(parentItem->name());
  }
  QList<GPXItem*> items = findChildrenByPath(path);
  if (row >= 0 && row < items.size())
    return createIndex(row, column, items[row]);
  qWarning() << "Can't find item on row" << row << "parent" << parent;
  return QModelIndex(); // should not happen
}

QModelIndex GPXListModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  GPXItem* childItem = static_cast<GPXItem*>(index.internalPointer());
  QDir dir(childItem->path());
  if (dir.path() == ".") // base path
    return QModelIndex();
  QString parentName = dir.dirName();
  dir.cdUp();
  QString parentPath = dir.path();

  QList<GPXItem*> parentSiblings = findChildrenByPath(parentPath);
  for (int row = 0; row < parentSiblings.size(); ++row)
  {
    GPXItem* parentCandidate = parentSiblings[row];
    if (parentCandidate->name() == parentName)
      return createIndex(row, 0, parentCandidate);
  }
  return QModelIndex(); // should not happen
}

QVariant GPXListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  const GPXItem* item = static_cast<GPXItem*>(index.internalPointer());
  switch (role)
  {
  case DirRole:
    return item->dir();
  case NameRole:
    return item->name();
  case PathRole:
    return item->path();
  case TimestampRole:
    return item->timestamp();
  case BigIdRole:
    return item->bigId();
  case RelativeFilePathRole:
    return item->relativeFilePath();
  case AbsoluteFilePathRole:
    return QDir(m_root).absolutePath().append(QDir::separator()).append(item->relativeFilePath());
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> GPXListModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[DirRole] = "dir";
  roles[NameRole] = "name";
  roles[PathRole] = "path";
  roles[TimestampRole] = "timestamp";
  roles[BigIdRole] = "bigId";
  roles[RelativeFilePathRole] = "relativeFilePath";
  roles[AbsoluteFilePathRole] = "absoluteFilePath";
  return roles;
}

QVariantMap GPXListModel::get(int row, const QModelIndex& parent) const
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  QString path("."); // base path
  if (parent.isValid())
  {
    GPXItem* parentItem = static_cast<GPXItem*>(parent.internalPointer());
    path = QDir(parentItem->path()).filePath(parentItem->name());
  }
  QList<GPXItem*> items = findChildrenByPath(path);
  if (row < 0 || row >= items.size())
    return QVariantMap();
  const GPXItem* item = items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[DirRole]] = item->dir();
  model[roles[NameRole]] = item->name();
  model[roles[PathRole]] = item->path();
  model[roles[TimestampRole]] = item->timestamp();
  model[roles[BigIdRole]] = item->bigId();
  model[roles[RelativeFilePathRole]] = item->relativeFilePath();
  model[roles[AbsoluteFilePathRole]] = QDir(m_root).absolutePath().append(QDir::separator())
    .append(item->relativeFilePath());
  return model;
}

bool GPXListModel::init(const QString& root)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  m_root = root;
  return true;
}

bool GPXListModel::loadData()
{
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    beginResetModel();
    if (m_items.count() > 0)
    {
      qDeleteAll(m_items);
      m_items.clear();
    }
    loadItems(nullptr, 0);
    m_dataState = DataStatus::DataLoaded;
    endResetModel();
  }
  emit loaded(true);
  emit countChanged();
  return true;
}

void GPXListModel::clearData()
{
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    beginResetModel();
    if (m_items.count() > 0)
    {
      qDeleteAll(m_items);
      m_items.clear();
    }
    endResetModel();
  }
  emit countChanged();
}

int GPXListModel::maxTreeDepth()
{
  return TREE_DEPTH;
}

bool GPXListModel::renameItem(const QString& newName, const QModelIndex& index)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (!index.isValid())
    return false;
  GPXItem* item = static_cast<GPXItem*>(index.internalPointer());
  QString absolutePath(m_root);
  absolutePath.append(QDir::separator()).append(item->path());
  QDir dir(absolutePath);
  QFile file(absolutePath);
  if (!dir.rename(item->name(), newName))
    return false;
  item->setName(newName);
  emit dataChanged(index, index);
  return true;
}

bool GPXListModel::removeItem(const QModelIndex& index)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (!index.isValid())
    return false;
  GPXItem* item = static_cast<GPXItem*>(index.internalPointer());
  QString absolutePath(m_root);
  absolutePath.append(QDir::separator()).append(item->path());
  QDir dir(absolutePath);
  QFileInfo finfo(dir, item->name());
  if (finfo.isDir())
  {
    if (!dir.rmdir(item->name()))
      return false;
  }
  else if (!dir.remove(item->name()))
    return false;
  QMultiMap<QString, GPXItem*>::iterator it = m_items.find(item->path());
  while (it != m_items.end() && it.key() == item->path())
  {
    if (it.value()->name() == item->name())
    {
      beginRemoveRows(index.parent(), index.row(), index.row());
      removeRow(index.row(), index.parent());
      m_items.erase(it);
      endRemoveRows();
      break;
    }
    ++it;
  }
  return true;
}

QString GPXListModel::findFileById(int bid)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  for (GPXItem* item : std::as_const(m_items))
  {
    if (item->bigId() == bid)
      return QDir(m_root).absolutePath().append(QDir::separator())
        .append(item->relativeFilePath());
  }
  return QString();
}

QList<GPXItem*> GPXListModel::findChildrenByPath(const QString& path) const
{
  QList<GPXItem*> list;
  QMultiMap<QString, GPXItem*>::const_iterator it = m_items.find(path);
  while (it != m_items.end() && it.key() == path)
  {
    list.push_front(*it);
    ++it;
  }
  return list;
}

void GPXListModel::loadItems(GPXItem *dirItem, int level)
{
  static QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::Writable | QDir::NoDotAndDotDot;
  QString path("."); // base path
  if (dirItem)
    path = QDir(dirItem->path()).filePath(dirItem->name());
  QString absolutePath(m_root);
  absolutePath.append(QDir::separator()).append(path);
  QFileInfoList list = QDir(absolutePath).entryInfoList(filters);
  for (QFileInfo& info : list)
  {
    if ((!info.isFile() && !info.isDir()) || info.isHidden())
      continue;
    if (info.isFile() && info.suffix().toLower() != "gpx")
      continue;
#if QT_VERSION >= 0x050A00
    GPXItem* item = new GPXItem(info.fileName(), path, info.birthTime(), info.isDir());
#else
    GPXItem* item = new GPXItem(info.fileName(), path, info.created(), info.isDir());
#endif
    m_items.insert(path, item);
    if (item->dir() && level < TREE_DEPTH)
      loadItems(item, level + 1);
  }
}
