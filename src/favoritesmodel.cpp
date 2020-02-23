
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
  m_lock = new QMutex(QMutex::Recursive);
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
    osmin::LockGuard g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int FavoritesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  osmin::LockGuard g(m_lock);
  return m_items.count();
}

QVariant FavoritesModel::data(const QModelIndex& index, int role) const
{
  osmin::LockGuard g(m_lock);
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
  osmin::LockGuard g(m_lock);
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
    osmin::LockGuard g(m_lock);
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
    osmin::LockGuard g(m_lock);
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

QModelIndex FavoritesModel::append()
{
  int row = m_items.count();
  if (insertRow(row))
    return index(row);
  return QModelIndex();
}

bool FavoritesModel::remove(int id)
{
  int row = 0;
  for (FavoriteItem* item : m_items)
  {
    if (item->id() == id)
      return removeRow(row);
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
  osmin::LockGuard g(m_lock);
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
  return model;
}

bool FavoritesModel::init(QIODevice* io)
{
  osmin::LockGuard g(m_lock);
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
  osmin::LockGuard g(m_lock);
  if (!m_io)
    return succeeded;
  QList<FavoriteItem*> data;
  if (m_io->open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text))
  {
    succeeded = true;
    osmin::CSVParser csv(',', '"');
    for (FavoriteItem* item : m_items)
    {
      QList<QByteArray*> row;
      QString num;
      row << new QByteArray(num.setNum(item->lat(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(item->lon(), 'f', 6).toUtf8());
      row << new QByteArray(num.setNum(item->alt(), 'f', 1).toUtf8());
      row << new QByteArray(item->timestamp().toString(Qt::ISODate).toUtf8());
      row << new QByteArray(item->label().toUtf8());
      row << new QByteArray(item->type().toUtf8());
      row << new QByteArray("");
      QByteArray line = csv.serialize(row);
      qDeleteAll(row);
      //qDebug("Saving favorite item: %s", line.constData());
      line.append("\r\n");
      if (m_io->write(line) != line.length())
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
    osmin::LockGuard g(m_lock);
    if (!m_io) {
      return false; }
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
        QList<QByteArray*> row = csv.deserialize(m_io->readLine(0x3ff));
        if (row.length() == 0 || c == MAX_ROWCOUNT)
          break;
        else if (row.length() >= 5)
        {
          ++c;
          FavoriteItem* item = new FavoriteItem();
          item->setLat(QString::fromUtf8(row[0]->constData()).toDouble());
          item->setLon(QString::fromUtf8(row[1]->constData()).toDouble());
          item->setAlt(QString::fromUtf8(row[2]->constData()).toDouble());
          item->setTimestamp(QDateTime::fromString(QString::fromUtf8(row[3]->constData()), Qt::ISODate));
          item->setLabel(QString::fromUtf8(row[4]->constData()));
          if (row.length() > 5)
            item->setType(QString::fromUtf8(row[5]->constData()));
          data << item;
          //qDebug("Loading favorite item: %3.5f , %3.5f : %s", item->lat(), item->lon(), item->label().toUtf8().constData());
        }
        qDeleteAll(row);
      }
      m_io->close();
    }

    m_seq = 0; // reset the sequence for id
    if (data.count() > 0)
    {
      beginInsertRows(QModelIndex(), 0, data.count()-1);
      for (FavoriteItem* item : data)
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
    osmin::LockGuard g(m_lock);
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
  osmin::LockGuard g(m_lock);
  for (FavoriteItem* item : m_items)
  {
    double r = osmin::Utils::sphericalDistance(item->lat(), item->lon(), lat, lon);
    if (r < AREA_RADIUS)
      return item->id();
  }
  return 0;
}
