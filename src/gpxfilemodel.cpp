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

#include "gpxfilemodel.h"
#include "osmscoutgpx/GpxFile.h"

#include <QThread>
#include <QFileInfo>

GPXFile::GPXFile()
: m_valid(false)
, m_path()
, m_gpx()
{
}

bool GPXFile::parse(const QString& filePath, osmscout::gpx::ProcessCallbackRef& callback)
{
  if (m_breaker)
    m_breaker->Break();
  m_breaker.reset(new osmscout::ThreadedBreaker());
  m_path = filePath;
  m_valid = osmscout::gpx::ImportGpx(filePath.toUtf8().constData(), m_gpx, m_breaker, callback);
  m_valid = m_valid && (!m_gpx.tracks.empty() || !m_gpx.waypoints.empty());
  return m_valid;
}

void GPXFile::breakParse()
{
  if (m_breaker)
    m_breaker->Break();
}

bool GPXFile::isAborted() const
{
  return (m_breaker && m_breaker->IsAborted());
}

QString GPXFile::filename() const
{
  return QFileInfo(m_path).fileName();
}

QString GPXFile::name() const
{
  return QString::fromUtf8(m_gpx.name.value_or("").c_str());
}

QString GPXFile::description() const
{
  return QString::fromUtf8(m_gpx.desc.value_or("").c_str());
}

QList<GPXObjectTrack> GPXFile::tracks()
{
  int i = 0;
  QList<GPXObjectTrack> list;
  for (osmscout::gpx::Track& track : m_gpx.tracks)
  {
    list << GPXObjectTrack(track, ++i);
  }
  return list;
}

QList<GPXObjectWayPoint> GPXFile::waypoints()
{
  int i = 0;
  QList<GPXObjectWayPoint> list;
  for (osmscout::gpx::Waypoint& waypoint : m_gpx.waypoints)
  {
    list << GPXObjectWayPoint(waypoint, ++i);
  }
  return list;
}

QString GPXObjectTrack::displayColor() const
{
  if (m_track.displayColor.has_value())
    return QString::fromUtf8(m_track.displayColor.value().ToHexString().c_str());
  return "";
}

const QStringList GPXFileModel::customTypeSet()
{
  static QStringList _types;
  static bool _init = false;
  if (!_init)
  {
    _types.push_back(OVERLAY_WAY_TRACK_TYPE);
    _types.push_back(OVERLAY_WAY_HIGHLIGHTED_TYPE);
    _types.push_back(OVERLAY_NODE_WAYPOINT_TYPE);
    _init = true;
  }
  return _types;
}

class GPXFileModel::Loader : public QThread
{
public:
  Loader(GPXFileModel& model) : _model(model) { }
  ~Loader() override {
    Q_ASSERT(!isRunning());
  }
  void setPath(const QString& path) { _path = path; }
  void run() override;
private:
  GPXFileModel& _model;
  QString _path;
};

void GPXFileModel::Loader::run()
{
  _model.parse(_path);
}

GPXFileModel::GPXFileModel(QObject* parent)
: QAbstractListModel(parent)
, m_lock(nullptr)
, m_dataState(DataStatus::DataBlank)
, m_file(nullptr)
, m_loader(nullptr)
, m_error()
, m_progress(0.0)
{
  m_lock = new QRecursiveMutex();
  m_loader = new Loader(*this);
  connect(m_loader, &Loader::started, this, &GPXFileModel::parsingChanged);
  connect(m_loader, &Loader::finished, this, &GPXFileModel::parsingChanged);
}

GPXFileModel::~GPXFileModel()
{
  qDeleteAll(m_items);
  m_items.clear();
  if (m_loader->isRunning())
  {
    qDebug("(%p) waiting for thread finished", this);
    do
    {
      if (m_file)
        m_file->breakParse();
      if (m_loader->wait(100)) // 100 millisec
        qDebug("(%p) thread finished", this);
    } while (m_loader->isRunning());
  }
  if (m_file)
    delete m_file;
  delete m_loader;
  delete m_lock;
}

int GPXFileModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  return m_items.count();
}

QVariant GPXFileModel::data(const QModelIndex& index, int role) const
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const GPXObject* item = m_items[index.row()];
  switch (role)
  {
  case IdRole:
    return item->id();
  case TypeRole:
    return item->type();
  case NameRole:
    return item->name();
  case DescriptionRole:
    return item->description();
  case SymbolRole:
    return item->type() == GPXObject::WayPoint ? static_cast<const GPXObjectWayPoint*>(item)->symbol() : "";
  case LengthRole:
    return item->type() == GPXObject::Track ? static_cast<const GPXObjectTrack*>(item)->length() : QVariant();
  case DisplayColorRole:
    return item->type() == GPXObject::Track ? static_cast<const GPXObjectTrack*>(item)->displayColor() : "";
  case LatRole:
    return item->type() == GPXObject::WayPoint ? static_cast<const GPXObjectWayPoint*>(item)->lat() : QVariant();
  case LonRole:
    return item->type() == GPXObject::WayPoint ? static_cast<const GPXObjectWayPoint*>(item)->lon() : QVariant();
  case ElevationRole:
    return item->type() == GPXObject::WayPoint ? static_cast<const GPXObjectWayPoint*>(item)->elevation() : QVariant();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> GPXFileModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[IdRole] = "id";
  roles[TypeRole] = "type";
  roles[NameRole] = "name";
  roles[DescriptionRole] = "description";
  roles[DisplayColorRole] = "displayColor";
  roles[SymbolRole] = "symbol";
  roles[LengthRole] = "length";
  roles[LatRole] = "lat";
  roles[LonRole] = "lon";
  roles[ElevationRole] = "elevation";
  return roles;
}

QVariantMap GPXFileModel::get(int row) const
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const GPXObject* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[IdRole]] = item->id();
  model[roles[TypeRole]] = item->type();
  model[roles[NameRole]] = item->name();
  model[roles[DescriptionRole]] = item->description();
  if (item->type() == GPXObject::WayPoint)
  {
    const GPXObjectWayPoint* _item = static_cast<const GPXObjectWayPoint*>(item);
    model[roles[SymbolRole]] = _item->symbol();
    model[roles[LatRole]] = _item->lat();
    model[roles[LonRole]] = _item->lon();
    model[roles[ElevationRole]] = _item->elevation();
  }
  else if (item->type() == GPXObject::Track)
  {
    const GPXObjectTrack* _item = static_cast<const GPXObjectTrack*>(item);
    model[roles[LengthRole]] = _item->length();
    model[roles[DisplayColorRole]] = _item->displayColor();
  }
  return model;
}

bool GPXFileModel::loadData()
{
  bool ret = false;
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
    if ((ret = (m_file && m_file->isValid())))
    {
      QList<GPXObject*> data;
      for (GPXObjectTrack& t : m_file->tracks())
        data.append(new GPXObjectTrack(t));
      for (GPXObjectWayPoint& w : m_file->waypoints())
        data.append(new GPXObjectWayPoint(w));
      beginInsertRows(QModelIndex(), 0, data.count()-1);
      for (GPXObject* item : data)
      {
        m_items << item;
      }
      data.clear();
      endInsertRows();
    }
    m_dataState = DataStatus::DataLoaded;
    endResetModel();
  }
  emit loaded(ret);
  emit countChanged();
  return ret;
}

void GPXFileModel::parse(const QString& filePath)
{
  bool parsed = false;
  {
    osmin::LockGuard<QRecursiveMutex> g(m_lock);
    if (m_file)
      delete m_file;
    m_file = new GPXFile();
    osmscout::gpx::ProcessCallbackRef cb(new Callback(*this));
    parsed = m_file->parse(filePath, cb);
    if (!parsed)
    {
      if (m_file->isAborted())
        qDebug("parse aborted for file %s", filePath.toUtf8().constData());
      else
        qWarning("parse failed for file %s", filePath.toUtf8().constData());
    }
  }
  emit parseFinished(parsed);
}

void GPXFileModel::parseFile(const QString &filePath)
{
  m_loader->setPath(filePath);
  m_loader->start();
}

void GPXFileModel::clearData()
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

bool GPXFileModel::parsing()
{
  return m_loader->isRunning();
}

QVariantList GPXFileModel::createOverlayObjects(int id /*=-1*/)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  QVariantList list;
  for (const GPXObject* item : std::as_const(m_items))
  {
    if (id >= 0 && item->id() != id)
      continue;
    if (item->type() == GPXObject::Track)
    {
      const GPXObjectTrack* obj = static_cast<const GPXObjectTrack*>(item);
      for (auto const& segment : obj->data().segments)
      {
        std::vector<osmscout::Point> points;
        points.reserve(segment.points.size());
        for (auto const& p : segment.points)
          points.emplace_back(0, p.coord);
        osmscout::OverlayWay* way = new osmscout::OverlayWay(points);
        way->setColor(obj->displayColor());
        way->setTypeName(OVERLAY_WAY_TRACK_TYPE);
        way->setName(obj->name());
        QVariant var;
        var.setValue<osmscout::OverlayObject*>(way);
        list << var;
      }
    }
    else if (item->type() == GPXObject::WayPoint)
    {
      const GPXObjectWayPoint* obj = static_cast<const GPXObjectWayPoint*>(item);
      osmscout::OverlayNode* node = new osmscout::OverlayNode();
      node->addPoint(obj->data().coord.GetLat(), obj->data().coord.GetLon());
      node->setTypeName(OVERLAY_NODE_WAYPOINT_TYPE);
      node->setName(obj->name());
      QVariant var;
      var.setValue<osmscout::OverlayObject*>(node);
      list << var;
    }
  }
  return list;
}

QVariantMap GPXFileModel::createTrackProfile(int id, int width)
{
  osmin::LockGuard<QRecursiveMutex> g(m_lock);
  QVariantMap data;
  for (const GPXObject* item : std::as_const(m_items))
  {
    if (item->id() != id)
      continue;
    if (item->type() == GPXObject::Track)
    {
      const GPXObjectTrack* obj = static_cast<const GPXObjectTrack*>(item);
      double length = obj->length();

      // setup x
      int cx = 0;
      for (auto const& segment : obj->data().segments)
        cx += segment.points.size();
      if (width > 0 && width < cx)
        cx = width;

      // fill y data
      QList<double> wlist;
      wlist.assign(cx, std::nan(""));
      const osmscout::gpx::TrackPoint * from = nullptr;
      double minele = 0.0;
      double maxele = 0.0;
      double d = std::nan(""); // progress from start
      std::optional<osmscout::Timestamp> startTime;
      std::optional<osmscout::Timestamp> endTime;
      for (auto const& segment : obj->data().segments)
      {
        // for all segments
        for (auto const& p : segment.points)
        {
          if (std::isnan(d))
          {
            // begining
            d = 0.0;
            startTime = p.timestamp;
          }
          if (from)
          {
            endTime = p.timestamp;
            d += from->coord.GetDistance(p.coord).AsMeter();
            int idx = std::round(double(cx - 1) * d / length);
            if (idx >= cx)
            {
              d = length;
              qWarning("Truncate data out of range !!!");
              break;
            }
            if (p.elevation.has_value())
            {
              double e = p.elevation.value();
              double o = wlist.at(idx);
              if (!std::isnan(e) && (std::isnan(o) || std::fabs(e) > std::fabs(o)))
                wlist[idx] = e;
              if (e < minele)
                minele = e;
              if (e > maxele)
                maxele = e;
            }
            from = &p;
          }
          else
          {
            wlist[0] = p.elevation.value_or(std::nan(""));
            from = &p;
            minele = maxele = wlist[0];
          }
        }
      }

      // cleanup data
      QVariantList dlist;
      QVariantList elist;
      dlist.reserve(cx);
      elist.reserve(cx);
      for (int idx = 0; idx < wlist.size(); ++idx)
      {
        if (!std::isnan(wlist[idx]))
        {
          dlist.push_back(length * double(idx) / double(cx -1));
          elist.push_back(wlist[idx]);
          //qDebug("x = %f , y = %f", dlist.back().value<double>(), elist.back().value<double>());
        }
      }
      wlist.clear();

      // create the object to return
      data.insert("filePath",  m_file->path());
      data.insert("fileName",  m_file->filename());
      data.insert("trackName", obj->name());
      double duration = 0;
      if (startTime.has_value() && endTime.has_value())
        duration = std::chrono::duration<double>(endTime.value() - startTime.value()).count();
      if (duration > 0)
        data.insert("duration", duration);
      else
        data.insert("duration", std::nan(""));
      data.insert("distance", length);
      data.insert("minElevation", minele);
      data.insert("maxElevation", maxele);
      data.insert("dataX", dlist);
      data.insert("dataY", elist);
    }
  }
  return data;
}

GPXFileModel::Callback::Callback(GPXFileModel& model)
: _model(model)
{
  model.m_error.clear();
  model.m_progress = 0.0;
}

void GPXFileModel::Callback::Progress(double p)
{
  double _p = round(p * 1000.0);
  if (_p > round(_model.m_progress * 1000.0))
  {
    _model.m_progress = _p / 1000.0;
    emit _model.progressChanged();
  }
}

void GPXFileModel::Callback::Error(const std::string &error)
{
  _model.m_error = QString::fromStdString(error);
}
