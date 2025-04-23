/*
 * Copyright (C) 2020-2023
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

#include "favoritesmodel.h"
#include "csvparser.h"
#include "utils.h"
#include <cmath>

#define MAX_ROWCOUNT  100
#define AREA_RADIUS   30.0

FavoriteItem::FavoriteItem()
: m_valid(false)
, m_id(0)
, m_normalized()
, m_label()
, m_timestamp()
, m_lat(0.0)
, m_lon(0.0)
, m_alt(0.0)
, m_type()
{
}

bool FavoriteItem::isValid() const
{
  return !m_label.isEmpty();
}

void FavoriteItem::setLabel(const QString& label)
{
  m_label = label;
  m_normalized = osmin::Utils::normalizedInputString(label);
}

FavoritesModel::FavoritesModel(QObject* parent)
  : QAbstractListModel(parent)
, m_lock(nullptr)
, m_seq(0)
, m_dataState(DataStatus::DataBlank)
, m_io(nullptr)
{
  m_lock = new QRecursiveMutex();
}

FavoritesModel::~FavoritesModel()
{
  qDeleteAll(m_items);
  m_items.clear();
  delete m_lock;
}

void FavoritesModel::addItem(FavoriteItem* item)
{
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int FavoritesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  return m_items.count();
}

QVariant FavoritesModel::data(const QModelIndex& index, int role) const
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const FavoriteItem* item = m_items[index.row()];
  switch (role)
  {
  case IdRole:
    return item->id();
  case NormalizedRole:
    return item->normalized();
  case TimestampRole:
    return item->timestamp();
  case LabelRole:
    return item->label();
  case LatRole:
    return item->lat();
  case LonRole:
    return item->lon();
  case AltRole:
    return item->alt();
  case TypeRole:
    return item->type();
  default:
    return QVariant();
  }
}

bool FavoritesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  FavoriteItem* item = m_items[index.row()];
  switch (role)
  {
  case LabelRole:
    item->setLabel(value.toString());
    break;
  case LatRole:
    item->setLat(value.toDouble());
    break;
  case LonRole:
    item->setLon(value.toDouble());
    break;
  case AltRole:
    item->setAlt(value.toDouble());
    break;
  case TypeRole:
    item->setType(value.toString());
    break;
  case IdRole:
  case NormalizedRole:
  case TimestampRole:
  default:
    return false;
  }
  item->setTimestamp(QDateTime::currentDateTime());
  emit dataChanged(index, index);
  return true;
}

bool FavoritesModel::insertRow(int row, const QModelIndex& parent)
{
  Q_UNUSED(parent)
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    if (row < 0 || row > m_items.count() || row == MAX_ROWCOUNT)
      return false;
    FavoriteItem* item = new FavoriteItem();
    item->setId(++m_seq);
    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, item);
    endInsertRows();
  }
  emit countChanged();
  return true;
}

bool FavoritesModel::removeRow(int row, const QModelIndex& parent)
{
  Q_UNUSED(parent)
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    if (row < 0 || row >= m_items.count())
      return false;
    beginRemoveRows(QModelIndex(), row, row);
    delete m_items.at(row);
    m_items.removeAt(row);
    endRemoveRows();
  }
  emit countChanged();
  return true;
}

int FavoritesModel::append(double lat, double lon, const QString& label, const QString& type)
{
  int id = 0;
  // start critical section
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    int row = m_items.count();
    if (row >= MAX_ROWCOUNT)
      return 0;
    id = ++m_seq;
    FavoriteItem* item = new FavoriteItem();
    item->setId(id);
    item->setLat(lat);
    item->setLon(lon);
    item->setAlt(0.0);
    item->setTimestamp(QDateTime::currentDateTime());
    item->setLabel(label);
    item->setType(type);
    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, item);
    endInsertRows();
  }
  // end critical section
  emit countChanged();
  emit appended(id);
  return id;
}

bool FavoritesModel::remove(int id)
{
  int row = 0;
  for (FavoriteItem* item : qAsConst(m_items))
  {
    if (item->id() == id)
    {
      if (!removeRow(row))
        return false;
      emit removed(id);
      return true;
    }
    ++row;
  }
  return false;
}

QHash<int, QByteArray> FavoritesModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[IdRole] = "id";
  roles[NormalizedRole] = "normalized";
  roles[TimestampRole] = "timestamp";
  roles[LabelRole] = "label";
  roles[LatRole] = "lat";
  roles[LonRole] = "lon";
  roles[AltRole] = "alt";
  roles[TypeRole] = "type";
  return roles;
}

QVariantMap FavoritesModel::get(int row) const
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const FavoriteItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[NormalizedRole]] = item->normalized();
  model[roles[TimestampRole]] = item->timestamp();
  model[roles[LabelRole]] = item->label();
  model[roles[LatRole]] = item->lat();
  model[roles[LonRole]] = item->lon();
  model[roles[AltRole]] = item->alt();
  model[roles[TypeRole]] = item->type();
  model[roles[IdRole]] = item->id();
  return model;
}

bool FavoritesModel::init(QIODevice* io)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (io->open(QIODevice::ReadWrite | QIODevice::Text)) {
    io->close();
    m_io = io;
    return true;
  }
  return false;
}

bool FavoritesModel::storeData()
{
  bool succeeded = false;
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (!m_io)
    return succeeded;
  if (m_io->open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text))
  {
    succeeded = true;
    osmin::CSVParser csv(',', '"');
    for (FavoriteItem* item : qAsConst(m_items))
    {
      osmin::CSVParser::container row;
      QString num;
      row.push_back(num.setNum(item->lat(), 'f', 6).toStdString());
      row.push_back(num.setNum(item->lon(), 'f', 6).toStdString());
      row.push_back(num.setNum(item->alt(), 'f', 1).toStdString());
      row.push_back(item->timestamp().toString(Qt::ISODate).toStdString());
      row.push_back(item->label().toStdString());
      row.push_back(item->type().toStdString());
      row.push_back(std::string(""));
      std::string line;
      csv.serialize(line, row);
      //qDebug("Saving favorite item: %s", line.c_str());
      line.append("\r\n");
      if (m_io->write(line.c_str()) != (qint64) line.length())
      {
        succeeded = false;
        break;
      }
    }
    m_io->close();
  }
  if (!succeeded)
    qWarning("Saving favorites failed");
  return succeeded;
}

bool FavoritesModel::loadData()
{
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    if (!m_io)
      return false;

    beginResetModel();
    if (m_items.count() > 0)
    {
      beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
      qDeleteAll(m_items);
      m_items.clear();
      endRemoveRows();
    }

    QList<FavoriteItem*> data;
    if (m_io->open(QIODevice::ReadOnly | QIODevice::Text))
    {
      int c = 0;
      osmin::CSVParser csv(',', '"');
      for (;;)
      {
        osmin::CSVParser::container row;
        bool next = csv.deserialize(row, m_io->readLine(0x3ff).toStdString());
        // paranoia: on corruption the last field could overflow
        while (next && !m_io->atEnd() && row.back().size() < 0x3ff)
        {
          // the row continue with next line
          next = csv.deserialize_next(row, m_io->readLine(0x3ff).toStdString());
        }
        if (row.size() == 0 || c == MAX_ROWCOUNT)
          break;
        else if (row.size() >= 5)
        {
          ++c;
          FavoriteItem* item = new FavoriteItem();
          item->setLat(QString::fromUtf8(row[0].c_str()).toDouble());
          item->setLon(QString::fromUtf8(row[1].c_str()).toDouble());
          item->setAlt(QString::fromUtf8(row[2].c_str()).toDouble());
          item->setTimestamp(QDateTime::fromString(QString::fromUtf8(row[3].c_str()), Qt::ISODate));
          item->setLabel(QString::fromUtf8(row[4].c_str()));
          if (row.size() > 5)
            item->setType(QString::fromUtf8(row[5].c_str()));
          data << item;
          //qDebug("Loading favorite item: %3.5f , %3.5f : %s [%s]", item->lat(), item->lon(),
          //       item->label().toUtf8().constData(), item->type().toUtf8().constData());
        }
      }
      m_io->close();
    }

    m_seq = 0; // reset the sequence for id
    if (data.count() > 0)
    {
      beginInsertRows(QModelIndex(), 0, data.count()-1);
      for (FavoriteItem* item : qAsConst(data))
      {
        item->setId(++m_seq);
        m_items << item;
      }
      data.clear();
      endInsertRows();
    }
    m_dataState = DataStatus::DataLoaded;
    endResetModel();
  }
  emit loaded(true);
  emit countChanged();
  return true;
}

void FavoritesModel::clearData()
{
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    beginResetModel();
    if (m_items.count() > 0)
    {
      beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
      qDeleteAll(m_items);
      m_items.clear();
      endRemoveRows();
    }
    endResetModel();
  }
  emit countChanged();
}

int FavoritesModel::isFavorite(double lat, double lon)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  for (FavoriteItem* item : qAsConst(m_items))
  {
    double r = osmin::Utils::sphericalDistance(item->lat(), item->lon(), lat, lon);
    if (r < AREA_RADIUS)
      return item->id();
  }
  return 0;
}

QVariantMap FavoritesModel::getById(int id)
{
  for (int row = 0; row < m_items.count(); ++row)
  {
    if (m_items[row]->id() == id)
      return get(row);
  }
  return QVariantMap();
}
