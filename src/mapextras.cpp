
#include "mapextras.h"
#include <osmscoutclientqt/OSMScoutQt.h>

MapExtras::MapExtras(QObject *parent)
: QObject(parent)
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
