#ifndef MAP_EXTRAS_H
#define MAP_EXTRAS_H

#include <osmscoutclientqt/DBThread.h>

#include <QObject>
#include <QQmlEngine>

class MapExtras : public QObject
{
  Q_OBJECT

public:
  explicit MapExtras(QObject *parent = nullptr);
  ~MapExtras();

  // Define singleton provider functions
  static QObject* createMapExtras(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new MapExtras;
  }

  Q_INVOKABLE QVariantList getStyleFlags();
  Q_INVOKABLE void reloadStyle(QVariantList flags);

  Q_INVOKABLE void setStyleFlag(const QString& name, bool value);
  Q_INVOKABLE void setDaylight(bool enable);

};

#endif // MAP_EXTRAS_H
