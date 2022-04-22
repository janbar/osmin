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

  /**
   * @brief Return the array of flags with properties { name: string, value: bool }
   * @return array of object
   */
  Q_INVOKABLE QVariantList getStyleFlags();

  /**
   * @brief Reload the style with the given flags
   * @param flags the array of flags { name: string, value: bool }
   */
  Q_INVOKABLE void reloadStyle(QVariantList flags);

  /**
   * @brief Set the given flag to the desired value
   * @param name the flag name
   * @param value the flag value
   */
  Q_INVOKABLE void setStyleFlag(const QString& name, bool value);

  /**
   * @brief Enable/Disable the flag 'daylight'
   * @param enable true to enable, else false
   */
  Q_INVOKABLE void setDaylight(bool enable);

};

#endif // MAP_EXTRAS_H
