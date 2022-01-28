#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H

#include "locked.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>
#include <QIODevice>

class FavoriteItem
{
public:
  FavoriteItem();

  virtual ~FavoriteItem() { }

  bool isValid() const;

  int id() const { return m_id; }
  void setId(int id) { m_id = id; }

  QString normalized() const { return m_normalized; }

  QString label() const { return m_label; }
  void setLabel(const QString& label);

  QDateTime timestamp() const { return m_timestamp; }
  void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }

  double lat() const { return m_lat; }
  void setLat(double lat) { m_lat = lat; }

  double lon() const { return m_lon; }
  void setLon(double lon) { m_lon = lon; }

  double alt() const { return m_alt; }
  void setAlt(double alt) { m_alt = alt; }

  QString type() const { return m_type; }
  void setType(const QString& type) { m_type = type; }

private:
  bool m_valid;
  int m_id;
  QString m_normalized;
  QString m_label;
  QDateTime m_timestamp;
  double m_lat;
  double m_lon;
  double m_alt;
  QString m_type;
};

class FavoritesModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)

public:
  enum FavoriteRoles
  {
    IdRole          = Qt::UserRole,
    NormalizedRole  = Qt::UserRole + 1,
    LatRole         = Qt::UserRole + 2,
    LonRole         = Qt::UserRole + 3,
    AltRole         = Qt::UserRole + 4,
    TimestampRole   = Qt::UserRole + 5,
    LabelRole       = Qt::UserRole + 6,
    TypeRole        = Qt::UserRole + 7,
  };
  Q_ENUM(FavoriteRoles)

  typedef enum {
    DataBlank     = 0,
    DataFailure   = 1,
    DataLoaded    = 2,
  } DataStatus;

  FavoritesModel(QObject* parent = nullptr);
  virtual ~FavoritesModel();

  void addItem(FavoriteItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool setData(const QModelIndex& index, const QVariant& value, int role);

  bool insertRow(int row, const QModelIndex& parent = QModelIndex());
  bool removeRow(int row, const QModelIndex& parent = QModelIndex());

  Q_INVOKABLE QModelIndex append();

  Q_INVOKABLE bool remove(int id);

  Q_INVOKABLE QVariantMap get(int row) const;

  bool init(QIODevice* io);

  Q_INVOKABLE bool storeData();

  Q_INVOKABLE bool loadData();

  Q_INVOKABLE void clearData();

  Q_INVOKABLE int isFavorite(double lat, double lon);
  Q_INVOKABLE QVariantMap getById(int id);

  bool dataFailure() { return m_dataState == DataStatus::DataFailure; }

signals:
  void loaded(bool succeeded);
  void stored(bool succeeded);
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QRecursiveMutex* m_lock;
  QList<FavoriteItem*> m_items;
  int m_seq;
  DataStatus m_dataState;
  QIODevice* m_io;
};


#endif // FAVORITESMODEL_H
