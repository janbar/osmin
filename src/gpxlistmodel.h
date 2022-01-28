#ifndef GPXLISTMODEL_H
#define GPXLISTMODEL_H

#include "locked.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>

class GPXItem
{
public:
  GPXItem();
  GPXItem(const QString& name, const QString& path, const QDateTime& timestamp, bool dir);

  virtual ~GPXItem() { }

  bool isValid() const;

  bool dir() const { return m_dir; }

  const QString& name() const { return m_name; }
  void setName(const QString& name);

  const QString& path() const { return m_path; }

  const QDateTime& timestamp() const { return m_timestamp; }

  const QString& relativeFilePath() const { return m_relativeFilePath; }

  int bigId() const;

private:
  bool m_dir;
  QString m_name;
  QString m_path;
  QDateTime m_timestamp;
  QString m_relativeFilePath;
};

class GPXListModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)
  Q_PROPERTY(int maxTreeDepth READ maxTreeDepth CONSTANT)

public:
  enum GPXRoles
  {
    DirRole               = Qt::UserRole,
    NameRole              = Qt::UserRole + 1,
    PathRole              = Qt::UserRole + 2,
    TimestampRole         = Qt::UserRole + 3,
    BigIdRole             = Qt::UserRole + 4,
    RelativeFilePathRole  = Qt::UserRole + 5,
    AbsoluteFilePathRole  = Qt::UserRole + 6,
  };
  Q_ENUM(GPXRoles)

  typedef enum {
    DataBlank     = 0,
    DataFailure   = 1,
    DataLoaded    = 2,
  } DataStatus;

  GPXListModel(QObject* parent = nullptr);
  virtual ~GPXListModel();

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row, const QModelIndex &parent) const;

  bool init(const QString& root);

  Q_INVOKABLE bool loadData();

  Q_INVOKABLE void clearData();

  bool dataFailure() { return m_dataState == DataStatus::DataFailure; }

  int maxTreeDepth();

  Q_INVOKABLE bool renameItem(const QString& name, const QModelIndex& index);
  Q_INVOKABLE bool removeItem(const QModelIndex& index);

  Q_INVOKABLE QString findFileById(int bid);

signals:
  void loaded(bool succeeded);
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QRecursiveMutex* m_lock;
  QMultiMap<QString, GPXItem*> m_items;
  DataStatus m_dataState;
  QString m_root;

  QList<GPXItem*> findChildrenByPath(const QString& path) const;

  void loadItems(GPXItem* dirItem, int level);
};

Q_DECLARE_METATYPE(GPXItem)

#endif /* GPXLISTMODEL_H */

