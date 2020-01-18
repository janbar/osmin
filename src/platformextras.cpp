
#include "platformextras.h"

#include <QDir>
#include <QStandardPaths>
#include <QStorageInfo>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#endif

#ifdef SAILFISHOS
#define AUTO_MOUNT        "/run/media/"
#else
#ifdef Q_OS_ANDROID
#define AUTO_MOUNT        "/storage/"
#else
#define AUTO_MOUNT        "/media/"
#endif
#endif

PlatformExtras::PlatformExtras(QObject* parent)
: QObject(parent)
, m_preventBlanking(false)
{
}

PlatformExtras::~PlatformExtras()
{
}

#ifdef Q_OS_ANDROID

QString PlatformExtras::getHomeDir()
{
  QAndroidJniObject activity = QtAndroid::androidActivity();
  QAndroidJniObject nullstr = QAndroidJniObject::fromString("");
  QAndroidJniObject file = activity.callObjectMethod("getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;", nullstr.object<jstring>());
  QAndroidJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
  return path.toString();
}

QString PlatformExtras::getDataDir(const char* appId)
{
  return PlatformExtras::getHomeDir();
}

QStringList PlatformExtras::getStorageDirs()
{
  QAndroidJniEnvironment jniEnv;
  QStringList dirs;
  // search for a mounted sdcard
  QAndroidJniObject activity = QtAndroid::androidActivity();
  QAndroidJniObject nullstr = QAndroidJniObject::fromString("");
  QAndroidJniObject files = activity.callObjectMethod("getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;", nullstr.object<jstring>());

  int len = jniEnv->GetArrayLength(files.object<jarray>());
  for (int i = 0; i < len; ++i)
  {
    QAndroidJniObject file = jniEnv->GetObjectArrayElement(files.object<jobjectArray>(), i);
    // android API returns null for unmounted storage
    if (!file.isValid())
      continue;
    QAndroidJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
    QString pathStr = path.toString();
    qInfo("Found storage: %s", pathStr.toUtf8().constData());
    dirs.push_back(pathStr);
  }
  return dirs;
}

void PlatformExtras::setPreventBlanking(bool on)
{
  m_preventBlanking = on;

  {
    QtAndroid::runOnAndroidThread([on]
    {
      static const int FLAG_KEEP_SCREEN_ON = QAndroidJniObject::getStaticField<jint>("android/view/WindowManager$LayoutParams", "FLAG_KEEP_SCREEN_ON");
      auto window = QtAndroid::androidActivity().callObjectMethod("getWindow", "()Landroid/view/Window;");
      if (on)
        window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
      else
        window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
    });
  }
}

#else

QString PlatformExtras::getHomeDir()
{
  return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString PlatformExtras::getDataDir(const char* appId)
{
  return QStandardPaths::locate(QStandardPaths::GenericDataLocation, appId, QStandardPaths::LocateDirectory);
}

QStringList PlatformExtras::getStorageDirs()
{
  QStringList dirs;
  // search for a mounted sdcard
  for (const QStorageInfo& storage : QStorageInfo::mountedVolumes())
  {
    QString path = storage.rootPath();
    if (storage.isValid() && storage.isReady() && path.startsWith(AUTO_MOUNT))
    {
      qInfo("Found storage: %s", path.toUtf8().constData());
      dirs.push_back(path);
    }
  }
  return dirs;
}

void PlatformExtras::setPreventBlanking(bool on)
{
  m_preventBlanking = on;
}

#endif
