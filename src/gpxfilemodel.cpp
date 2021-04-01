
#include "gpxfilemodel.h"
#include "osmscout/gpx/GpxFile.h"

#include <QThread>

GPXFile::GPXFile()
: m_valid(false)
, m_path()
, m_error()
, m_progress(0.0)
, m_gpx()
, m_callback(std::make_shared<Callback>(*this))
{
}

bool GPXFile::parse(const QString& filePath)
{
  if (m_breaker)
  {
    m_breaker->Break();
    m_breaker.reset();
  }
  m_path = filePath;
  m_breaker = std::make_shared<osmscout::ThreadedBreaker>();
  m_valid = osmscout::gpx::ImportGpx(filePath.toUtf8().constData(), m_gpx, m_breaker, m_callback);
  return m_valid;
}

QString GPXFile::name() const
{
  return QString::fromUtf8(m_gpx.name.value_or("").c_str());
}

QString GPXFile::description() const
{
  return QString::fromUtf8(m_gpx.desc.value_or("").c_str());
}

QList<GPXObject*> GPXFile::tracks() const
{
  int i = 0;
  QList<GPXObject*> list;
  for (const osmscout::gpx::Track& track : m_gpx.tracks)
  {
    list << new GPXObjectTrack(track, ++i);
  }
  return list;
}

QList<GPXObject*> GPXFile::waypoints() const
{
  int i = 0;
  QList<GPXObject*> list;
  for (const osmscout::gpx::Waypoint& waypoint : m_gpx.waypoints)
  {
    list << new GPXObjectWayPoint(waypoint, ++i);
  }
  return list;
}

const QSet<QString>& GPXFileModel::trackTypeSet()
{
  static QSet<QString> _track;
  static bool _init = false;
  if (!_init)
  {
    _track.insert(OVERLAY_WAY_TYPE);
    _track.insert(OVERLAY_WAY_TYPE "Black");
    _track.insert(OVERLAY_WAY_TYPE "DarkRed");
    _track.insert(OVERLAY_WAY_TYPE "DarkGreen");
    _track.insert(OVERLAY_WAY_TYPE "DarkYellow");
    _track.insert(OVERLAY_WAY_TYPE "DarkBlue");
    _track.insert(OVERLAY_WAY_TYPE "DarkMagenta");
    _track.insert(OVERLAY_WAY_TYPE "DarkCyan");
    _track.insert(OVERLAY_WAY_TYPE "DarkGray");
    _track.insert(OVERLAY_WAY_TYPE "LightGray");
    _track.insert(OVERLAY_WAY_TYPE "Red");
    _track.insert(OVERLAY_WAY_TYPE "Green");
    _track.insert(OVERLAY_WAY_TYPE "Yellow");
    _track.insert(OVERLAY_WAY_TYPE "Blue");
    _track.insert(OVERLAY_WAY_TYPE "Magenta");
    _track.insert(OVERLAY_WAY_TYPE "Cyan");
    _track.insert(OVERLAY_WAY_TYPE "White");
    _init = true;
  }
  return _track;
}

class GPXFileModel::Loader : public QThread
{
public:
  Loader(GPXFileModel& model) : _model(model) { }
  ~Loader() override { }
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
{
  m_lock = new QMutex(QMutex::Recursive);
  m_loader = new Loader(*this);
  connect(m_loader, &Loader::started, this, &GPXFileModel::parsingChanged);
  connect(m_loader, &Loader::finished, this, &GPXFileModel::parsingChanged);
}

GPXFileModel::~GPXFileModel()
{
  qDeleteAll(m_items);
  m_items.clear();
  if (m_file)
    delete m_file;
  delete m_loader;
  delete m_lock;
}

int GPXFileModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  osmin::LockGuard g(m_lock);
  return m_items.count();
}

QVariant GPXFileModel::data(const QModelIndex& index, int role) const
{
  osmin::LockGuard g(m_lock);
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
  case DisplayColorRole:
    return item->type() == GPXObject::Track ? static_cast<const GPXObjectTrack*>(item)->displayColorHexString() : "";
  case LengthRole:
    return item->type() == GPXObject::Track ? static_cast<const GPXObjectTrack*>(item)->length() : 0.0;
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
  return roles;
}

QVariantMap GPXFileModel::get(int row) const
{
  osmin::LockGuard g(m_lock);
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
  }
  else if (item->type() == GPXObject::Track)
  {
    const GPXObjectTrack* _item = static_cast<const GPXObjectTrack*>(item);
    model[roles[DisplayColorRole]] = _item->displayColorHexString();
    model[roles[LengthRole]] = _item->length();
  }
  return model;
}

bool GPXFileModel::loadData()
{
  bool ret = false;
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
    if ((ret = (m_file && m_file->isValid())))
    {
      QList<GPXObject*> data;
      data.append(m_file->tracks());
      data.append(m_file->waypoints());
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
    osmin::LockGuard g(m_lock);
    if (m_file)
      delete m_file;
    m_file = new GPXFile();
    parsed = m_file->parse(filePath);
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

bool GPXFileModel::parsing()
{
  return m_loader->isRunning();
}

QVariantList GPXFileModel::createOverlayObjects(int id /*=-1*/)
{
  osmin::LockGuard g(m_lock);
  QVariantList list;
  for (const GPXObject* item : m_items)
  {
    if (id >= 0 && item->id() != id)
      continue;
    if (item->type() == GPXObject::Track)
    {
      const QSet<QString>& typeSet = trackTypeSet();
      const GPXObjectTrack* obj = static_cast<const GPXObjectTrack*>(item);
      for (auto const& segment : obj->m_track.segments)
      {
        std::vector<osmscout::Point> points;
        points.reserve(segment.points.size());
        for (auto const& p : segment.points)
          points.emplace_back(0, p.coord);
        osmscout::OverlayWay* way = new osmscout::OverlayWay(points);
        QString _type(OVERLAY_WAY_TYPE);
        _type.append(obj->displayColorName());
        if (typeSet.find(_type) != typeSet.end())
          way->setTypeName(_type);
        else
          way->setTypeName(OVERLAY_WAY_TYPE);
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
      node->addPoint(obj->m_waypoint.coord.GetLat(), obj->m_waypoint.coord.GetLon());
      node->setTypeName("_waypoint");
      node->setName(obj->name());
      QVariant var;
      var.setValue<osmscout::OverlayObject*>(node);
      list << var;
    }
  }
  return list;
}

QString GPXObjectTrack::displayColorName() const
{
  if (m_track.displayColor.has_value())
  {
    const osmscout::Color& color = m_track.displayColor.value();
    if (color == osmscout::Color::GREEN)
      return "Green";
    if (color == osmscout::Color::RED)
      return "Red";
    if (color == osmscout::Color::BLUE)
      return "Blue";
    if (color == osmscout::Color::YELLOW)
      return "Yellow";
    if (color == osmscout::Color::FUCHSIA)
      return "Magenta";
    if (color == osmscout::Color::AQUA)
      return "Cyan";
    if (color == osmscout::Color::LIGHT_GRAY)
      return "LightGray";
    if (color == osmscout::Color::WHITE)
      return "White";
    if (color == osmscout::Color::BLACK)
      return "Black";
    if (color == osmscout::Color::DARK_GREEN)
      return "DarkGreen";
    if (color == osmscout::Color::DARK_RED)
      return "DarkRed";
    if (color == osmscout::Color::DARK_BLUE)
      return "DarkBlue";
    if (color == osmscout::Color::DARK_YELLOW)
      return "DarkYellow";
    if (color == osmscout::Color::DARK_FUCHSIA)
      return "DarkMagenta";
    if (color == osmscout::Color::DARK_AQUA)
      return "DarkCyan";
    if (color == osmscout::Color::DARK_GRAY)
      return "DarkGray";
  }
  return "";
}
