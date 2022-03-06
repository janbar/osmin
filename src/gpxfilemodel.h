#ifndef GPXFILEMODEL_H
#define GPXFILEMODEL_H

#include "locked.h"

#include <osmscoutgpx/Import.h>
#include <osmscoutclientqt/OverlayObject.h>
#include <QAbstractListModel>
#include <QDateTime>
#include <QString>
#include <QSet>

#define OVERLAY_WAY_TRACK_TYPE            "_track"
#define OVERLAY_WAY_HIGHLIGHTED_TYPE      "_highlighted"
#define OVERLAY_NODE_WAYPOINT_TYPE        "_waypoint"

class GPXObject;
class GPXObjectTrack;
class GPXObjectWayPoint;

class GPXFile
{
public:
  GPXFile();
  virtual ~GPXFile() { }

  bool parse(const QString& filePath);
  void breakParse();
  bool isAborted() const;
  bool isValid() const { return m_valid; }
  const QString& path() const { return m_path; }

  QString name() const;
  QString description() const;
  QList<GPXObject*> tracks() const;
  QList<GPXObject*> waypoints() const;

  class Callback : public osmscout::gpx::ProcessCallback
  {
  public:
    Callback(GPXFile& file) : _file(file) { }
    void Progress(double p) override { _file.m_progress = p; }
    void Error(const std::string& error) override { _file.m_error = QString::fromStdString(error); }
  private:
    GPXFile& _file;
  };

private:
  bool m_valid;
  QString m_path;
  QString m_error;
  double m_progress;
  osmscout::gpx::GpxFile m_gpx;
  osmscout::gpx::ProcessCallbackRef m_callback;
  osmscout::BreakerRef m_breaker;
};

class GPXObject : public QObject
{
  Q_OBJECT
public:
  virtual ~GPXObject() { }
  enum ObjectType
  {
    Track     = 0,
    WayPoint  = 1,
  };
  Q_ENUM(ObjectType)

  virtual int id() const = 0;
  virtual ObjectType type() const = 0;
  virtual QString name() const = 0;
  virtual QString description() const = 0;
};

class GPXObjectTrack : public GPXObject
{
  friend class GPXFileModel;
public:
  GPXObjectTrack(const osmscout::gpx::Track& track, int id) : m_track(track), m_id(id) { }
  int id() const override { return m_id; }
  ObjectType type() const override { return Track; }
  QString name() const override { return QString::fromUtf8(m_track.name.value_or(std::to_string(m_id)).c_str()); }
  QString description() const override { return QString::fromUtf8(m_track.desc.value_or("").c_str()); }
  double length() const { return m_track.GetLength().AsMeter(); }
  QString displayColor() const;
private:
  const osmscout::gpx::Track& m_track;
  int m_id;
};

class GPXObjectWayPoint : public GPXObject
{
  friend class GPXFileModel;
public:
  GPXObjectWayPoint(const osmscout::gpx::Waypoint& waipoint, int id) : m_waypoint(waipoint), m_id(id) { }
  int id() const override { return m_id; }
  ObjectType type() const override { return WayPoint; }
  QString name() const override { return QString::fromUtf8(m_waypoint.name.value_or(std::to_string(m_id)).c_str()); }
  QString description() const override { return QString::fromUtf8(m_waypoint.description.value_or("").c_str()); }
  QString symbol() const { return QString::fromUtf8(m_waypoint.symbol.value_or("").c_str()); }
  double lat() const { return m_waypoint.coord.GetLat(); }
  double lon() const { return m_waypoint.coord.GetLon(); }
  double elevation() const { return m_waypoint.elevation.value_or(0.0); }
private:
  const osmscout::gpx::Waypoint& m_waypoint;
  int m_id;
};

class GPXFileModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool parsing READ parsing NOTIFY parsingChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)
  Q_PROPERTY(bool fileValid READ isValid NOTIFY loaded)
  Q_PROPERTY(QString filePath READ filePath NOTIFY loaded)
  Q_PROPERTY(QString description READ description NOTIFY loaded)

public:
  enum GPXObjectRoles
  {
    IdRole            = Qt::UserRole,
    TypeRole          = Qt::UserRole + 1,
    NameRole          = Qt::UserRole + 2,
    DescriptionRole   = Qt::UserRole + 3,
    SymbolRole        = Qt::UserRole + 4,
    DisplayColorRole  = Qt::UserRole + 5,
    LengthRole        = Qt::UserRole + 6,
    LatRole           = Qt::UserRole + 7,
    LonRole           = Qt::UserRole + 8,
    ElevationRole     = Qt::UserRole + 9
  };
  Q_ENUM(GPXObjectRoles)

  typedef enum {
    DataBlank     = 0,
    DataFailure   = 1,
    DataLoaded    = 2,
  } DataStatus;

  static const QSet<QString>& customTypeSet();

  GPXFileModel(QObject* parent = nullptr);
  virtual ~GPXFileModel();

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row) const;

  Q_INVOKABLE void parseFile(const QString& filePath);
  Q_INVOKABLE bool loadData();
  Q_INVOKABLE void clearData();

  bool dataFailure() { return m_dataState == DataStatus::DataFailure; }

  bool parsing();
  bool isValid() const { return m_file ? m_file->isValid() : false; }
  QString filePath() const { return m_file ? m_file->path() : ""; }
  QString description() const { return m_file ? m_file->description() : ""; }

  Q_INVOKABLE QVariantList createOverlayObjects(int id = -1);

signals:
  void parsingChanged();
  void parseFinished(bool succeeded);
  void loaded(bool succeeded);
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QRecursiveMutex* m_lock;
  QList<GPXObject*> m_items;
  DataStatus m_dataState;
  GPXFile* m_file;

  class Loader;
  Loader* m_loader;

  void parse(const QString& filePath);
};

Q_DECLARE_METATYPE(GPXFile)

#endif /* GPXFILEMODEL_H */

