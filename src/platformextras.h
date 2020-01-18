#ifndef PLATFORM_EXTRAS_H
#define PLATFORM_EXTRAS_H

//#define Q_OS_ANDROID

#include <QObject>
#include <QQmlEngine>

class PlatformExtras : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool preventBlanking READ getPreventBlanking WRITE setPreventBlanking NOTIFY preventBlanking)

public:
  explicit PlatformExtras(QObject *parent = nullptr);
  ~PlatformExtras();

  // Define singleton provider functions
  static QObject* createPlatformExtras(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new PlatformExtras;
  }

  static QString getHomeDir();
  static QString getDataDir(const char* appId);
  static QStringList getStorageDirs();

signals:
  void preventBlanking();

private:
  bool getPreventBlanking() const { return m_preventBlanking; };

  void setPreventBlanking(bool on);

  bool m_preventBlanking;
};

#endif // PLATFORM_EXTRAS_H
