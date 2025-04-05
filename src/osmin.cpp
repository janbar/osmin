/*
 * Copyright (C) 2020-2024
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

#include <QtGlobal>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QtQuickControls2>
#include <QStandardPaths>
#include <QTranslator>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QPixmap>
#include <QIcon>
#include <QTime>

#include <locale>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidService>
#include <QAndroidJniObject>
#endif

#ifdef DEVICE_MOBILE
// Limit cache memory to avoid angering the crazy lowmemorykiller
#define ONLINE_TILE_CACHE_MB   60
#define OFFLINE_TILE_CACHE_MB  100
#define MEMORY_TARGET          500
#else
#define ONLINE_TILE_CACHE_MB   60
#define OFFLINE_TILE_CACHE_MB  200
#define MEMORY_TARGET          1000
#endif

#define APP_TR_NAME       "osmin"                   // translations base name
#define ORG_NAME          "io.github.janbar"        // organisation id
#define APP_NAME          "osmin"                   // application name
#define APP_DISPLAY_NAME  "Osmin"                   // application display name
#ifdef Q_OS_ANDROID
#define APP_ID            ORG_NAME "." APP_NAME     // Android application id
#else
#define APP_ID            APP_NAME                  // Unix default application id
#endif

#include "signalhandler.h"

#ifndef APP_VERSION
#define APP_VERSION "Undefined"
#endif

#define SERVICE_FLAG                "-service"
#define OSMIN_MODULE                "Osmin"         // QML module name
#define DIR_MAPS                    "maps"
#define DIR_VOICES                  "voices"
#define DIR_RES                     "resources"
#define DIR_GPX                     "GPX"
#define RES_FAVORITES_FILE          "favorites.csv"
#define RES_HILLSHADE_SERVER_FILE   "hillshade-tile-server.json"
#define RES_HILLSHADE_FILE_SAMPLE   "hillshade-tile-server.json.sample"
#define RES_MAP_PROVIDERS           "map-providers.json"
#define RES_VOICE_PROVIDERS         "voice-providers.json"
#define RES_TILE_PROVIDERS          "online-tile-providers.json"

#include <osmscoutclientqt/OSMScoutQt.h>
// Custom QML objects
#include <osmscoutclientqt/MapWidget.h>
#include <osmscoutclientqt/SearchLocationModel.h>
#include <osmscoutclientqt/RoutingModel.h>
#include <osmscoutclientqt/AvailableMapsModel.h>
#include <osmscoutclientqt/MapDownloadsModel.h>
#include <osmscoutclientqt/QmlSettings.h>

// Configure Logger
#include <osmscout/log/Logger.h>

#include "platformextras.h"
#include "mapextras.h"
#include "converter.h"
#include "favoritesmodel.h"
#include "gpxlistmodel.h"
#include "gpxfilemodel.h"
#include "memorymanager.h"
#include "qmlsortfiltermodel.h"
#include "utils.h"

#include "service.h"
#include "servicefrontend.h"
#include "remoteservice.h"
#include "remotecompass.h"
#include "remotepositionsource.h"
#include "remotetracker.h"

int startService(int argc, char* argv[]);
int startGUI(int argc, char* argv[]);
void setupApp(QGuiApplication& app);
void prepareTranslator(QGuiApplication& app, const QString& translationPath, const QString& translationPrefix, const QLocale& locale);
void signalCatched(int signal);
void doExit(int code);
bool testServiceUrl(const char *url, int timeout);
QString basename(const QString& filepath);
bool migration(const QString& version, const QString& pathapp, const QString& pathdata);

QDir g_assetDir;  // base path of asset
QDir g_dataDir;   // base path for user data
QDir g_appDir;    // base path for app data

QDir g_usrResDir; // directory for user resources
QDir g_appResDir; // directory for app resources

QFile*            g_favoritesFile         = nullptr;
GPXListModel*     g_GPXListModel          = nullptr;
RemoteService*    g_remoteService         = nullptr;
RemoteTracker*    g_remoteTracker         = nullptr;
QString*          g_hillshadeProvider     = nullptr;

QObject* qFavoritesModelInstance(QQmlEngine *engine, QJSEngine *scriptEngine);
QObject* qGPXListModelInstance(QQmlEngine *engine, QJSEngine *scriptEngine);
QObject* qRemoteServiceInstance(QQmlEngine *engine, QJSEngine *scriptEngine);
QObject* qRemoteTrackerInstance(QQmlEngine *engine, QJSEngine *scriptEngine);
QObject* qUtilsInstance(QQmlEngine *engine, QJSEngine *scriptEngine);

#if defined(QT_STATICPLUGIN)
void importStaticPlugins(QQmlEngine* engine)
{
  (void)engine;
  //{ myPlugin e; e.initializeEngine(engine, "pluginName"); e.registerTypes("pluginName"); }
}
#endif

int main(int argc, char *argv[])
{
  int ret = 0;

  if (argc > 1 && strcmp(argv[1], SERVICE_FLAG) == 0)
  {
    ret = startService(argc, argv);
  }
  else
  {
    ret = startGUI(argc, argv);
  }

  return ret;
}

int startService(int argc, char* argv[])
{
  /************************************************************************/
  /*                                                                      */
  /*  Start background service                                            */
  /*                                                                      */
  /************************************************************************/

  int ret = 0;
  QCoreApplication::setApplicationName(APP_NAME);
  QCoreApplication::setOrganizationName(ORG_NAME);

  qRegisterMetaType<osmscout::PositionAgent::PositionState>("osmscout::PositionAgent::PositionState");
  qRegisterMetaType<std::optional<osmscout::Bearing>>("std::optional<osmscout::Bearing>");
  qRegisterMetaType<osmscout::GeoCoord>("osmscout::GeoCoord");

#ifdef Q_OS_ANDROID
  QAndroidService app(argc, argv);
  // retrieve the data directory for the 'service' instance
  // PlatformExtras::getDataDir() cannot be used here, because this is not 'activity' instance
  QAndroidJniObject service = QtAndroid::androidService();
  QAndroidJniObject nullstr = QAndroidJniObject::fromString("");
  QAndroidJniObject file = service.callObjectMethod("getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;", nullstr.object<jstring>());
  QAndroidJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
  g_dataDir = QDir(path.toString());
#else
  QCoreApplication app(argc, argv);
  g_dataDir = QDir(PlatformExtras::getDataDir());
  // create the base
  if (!g_dataDir.mkpath(QString(APP_NAME)))
    return EXIT_FAILURE;
  g_dataDir.cd(APP_NAME);
#endif
  // create path for GPX files, and tracker data
  if (!g_dataDir.mkpath(DIR_GPX))
    return EXIT_FAILURE;

  Service * svc = new Service(SERVICE_URL, g_dataDir.absoluteFilePath(DIR_GPX));
  QTimer::singleShot(0, svc, &Service::run);
  ret = app.exec();

  delete svc;

  return ret;
}

int startGUI(int argc, char* argv[])
{
  /************************************************************************/
  /*                                                                      */
  /*  Start GUI Application                                               */
  /*                                                                      */
  /************************************************************************/

  int ret = 0;
  QGuiApplication::setApplicationName(APP_NAME);
  QGuiApplication::setApplicationDisplayName(APP_DISPLAY_NAME);
  QGuiApplication::setOrganizationName(ORG_NAME);
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);
  setupApp(app);

  // check installed asset
  g_assetDir = QDir(PlatformExtras::getAssetDir(APP_ID));
  if (!g_assetDir.cd(DIR_RES) || !g_assetDir.exists())
  {
    qWarning("FATAL: Asset directory cannot be found: %s", g_assetDir.absolutePath().toUtf8().constData());
    return EXIT_FAILURE;
  }
  g_appDir = QDir(PlatformExtras::getAppDir());
  g_dataDir = QDir(PlatformExtras::getDataDir());

#ifdef Q_OS_ANDROID
  // request permissions for fine location
  {
    QStringList androidPermissions;
    androidPermissions.append("android.permission.ACCESS_COARSE_LOCATION");
    androidPermissions.append("android.permission.ACCESS_FINE_LOCATION");
    QtAndroid::requestPermissionsSync(androidPermissions);
  }

  // create path for app resources
  if (!g_appDir.mkpath(DIR_RES))
    return EXIT_FAILURE;
  g_appResDir = g_appDir;
  g_appResDir.cd(DIR_RES);
  // create path for user resources
  if (!g_dataDir.mkpath(DIR_RES))
    return EXIT_FAILURE;
  g_usrResDir = g_dataDir;
  g_usrResDir.cd(DIR_RES);
#else
  // create base path for app data
  if (!g_appDir.mkpath(QString(APP_NAME)))
    return EXIT_FAILURE;
  g_appDir.cd(APP_NAME);
  // create path for app resources
  if (!g_appDir.mkpath(DIR_RES))
    return EXIT_FAILURE;
  g_appResDir = g_appDir;
  g_appResDir.cd(DIR_RES);
  // create base path for user data
  if (!g_dataDir.mkpath(QString(APP_NAME)))
    return EXIT_FAILURE;
  g_dataDir.cd(APP_NAME);
  // create path for user resources
  if (!g_dataDir.mkpath(DIR_RES))
    return EXIT_FAILURE;
  g_usrResDir = g_dataDir;
  g_usrResDir.cd(DIR_RES);
#endif
  // create path for GPX files
  if (!g_dataDir.mkpath(DIR_GPX))
    return EXIT_FAILURE;

  // fork the service process
#if defined(Q_OS_ANDROID)
  QAndroidJniObject::callStaticMethod<void>("io/github/janbar/osmin/QtAndroidService",
                                                "startQtAndroidService",
                                                "(Landroid/content/Context;)V",
                                                QtAndroid::androidActivity().object());
#else
  QScopedPointer<QProcess> psvc(new QProcess());
  if (!testServiceUrl(SERVICE_URL, 100))
  {
    QStringList pargs;
    pargs.append(SERVICE_FLAG);
    psvc->start(argv[0], pargs);
  }
#endif

  // init service frontend
  ServiceFrontendPtr serviceFrontend(new ServiceFrontend(SERVICE_URL));
  // create singleton for the remote service
  g_remoteService = new RemoteService();
  g_remoteService->connectToService(serviceFrontend);
  // create singleton for the remote tracker
  g_remoteTracker = new RemoteTracker();
  g_remoteTracker->connectToService(serviceFrontend);

  // initialize app resources
  QString oldVersionPath = g_appResDir.absoluteFilePath("version");
  // migration ? override version file if one exists in data
  if (g_usrResDir.exists("version"))
    oldVersionPath = g_usrResDir.absoluteFilePath("version");
  QFile oldVersion(oldVersionPath);
  if (oldVersion.exists())
  {
    qInfo("Checking installed assets...");
    QByteArray _version;
    if (oldVersion.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      _version = oldVersion.readLine(64);
      oldVersion.close();
    }
    qInfo("Found assets version %s", _version.constData());
    if (strlen(APP_VERSION) != _version.length() || ::strncmp(APP_VERSION, _version.constData(), _version.length()) != 0)
    {
      // launch the migration
      migration(QString::fromUtf8(_version), g_appDir.absolutePath(), g_dataDir.absolutePath());
      oldVersion.remove();
      qWarning("Assets will be upgraded to version %s", APP_VERSION);
    }
  }
  QFile resVersion(g_appResDir.absoluteFilePath("version"));
  if (!resVersion.exists())
  {
    QStringList folders;
    folders.push_back("");
    while (!folders.empty())
    {
      QString folder = folders.front();
      folders.pop_front();
      if (folder.isEmpty() || g_appResDir.exists(folder) || g_appResDir.mkpath(folder))
      {
        QDir assets;
        if (folder.isEmpty())
          assets.setPath(g_assetDir.absolutePath());
        else
          assets.setPath(g_assetDir.absoluteFilePath(folder));
        for (QFileInfo& asset : assets.entryInfoList())
        {
          if (asset.isSymLink() || asset.isHidden())
            continue;
          else if (asset.isFile())
          {
            QString filepath;
            if (folder.isEmpty())
              filepath = g_appResDir.absolutePath().append('/').append(basename(asset.filePath()));
            else
              filepath = g_appResDir.absolutePath().append('/').append(folder).append('/').append(basename(asset.filePath()));
            if ((!QFile::exists(filepath) || QFile::remove(filepath)) &&
                QFile::copy(asset.absoluteFilePath(), filepath))
              continue;
            qWarning("Failed to create file %s", filepath.toUtf8().constData());
            return EXIT_FAILURE;
          }
          else if (asset.isDir())
          {
            QString path;
            if (folder.isEmpty())
              folders.push_back(path.append(basename(asset.filePath())));
            else
              folders.push_back(path.append(folder).append('/').append(basename(asset.filePath())));
            continue;
          }
        }
      }
      else
      {
        qWarning("Failed to create directory %s/%s", g_appResDir.absolutePath().toUtf8().constData(), folder.toUtf8().constData());
        return EXIT_FAILURE;
      }
    }
    if (!resVersion.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate) || resVersion.write(APP_VERSION) <= 0)
    {
      resVersion.close();
      qWarning("Failed to initialize assets %s", APP_VERSION);
      return EXIT_FAILURE;
    }
    resVersion.close();
  }
  qInfo("Resource directory is %s", g_appResDir.path().toUtf8().constData());

  // initialize user resources
  if (!g_usrResDir.exists(RES_MAP_PROVIDERS))
    QFile::copy(g_appResDir.absoluteFilePath(RES_MAP_PROVIDERS), g_usrResDir.absoluteFilePath(RES_MAP_PROVIDERS));
  if (!g_usrResDir.exists(RES_VOICE_PROVIDERS))
    QFile::copy(g_appResDir.absoluteFilePath(RES_VOICE_PROVIDERS), g_usrResDir.absoluteFilePath(RES_VOICE_PROVIDERS));
  if (!g_usrResDir.exists(RES_TILE_PROVIDERS))
    QFile::copy(g_appResDir.absoluteFilePath(RES_TILE_PROVIDERS), g_usrResDir.absoluteFilePath(RES_TILE_PROVIDERS));
  // create the sample hillshade server file from resources as needed
  if (!g_usrResDir.exists(RES_HILLSHADE_SERVER_FILE) && !g_usrResDir.exists(RES_HILLSHADE_FILE_SAMPLE)
      && g_appResDir.exists(RES_HILLSHADE_FILE_SAMPLE))
  {
    qInfo("Create sample file for hillshade tile server '%s'", g_usrResDir.absoluteFilePath(RES_HILLSHADE_FILE_SAMPLE).toUtf8().constData());
    QFile::copy(g_appResDir.absoluteFilePath(RES_HILLSHADE_FILE_SAMPLE), g_usrResDir.absoluteFilePath(RES_HILLSHADE_FILE_SAMPLE));
  }
  // load hillshade server file
  g_hillshadeProvider = new QString("{}");
  if (g_usrResDir.exists(RES_HILLSHADE_SERVER_FILE))
  {
    QFile file(g_usrResDir.absoluteFilePath(RES_HILLSHADE_SERVER_FILE));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qInfo("Found hillshade tile server file '%s'", file.fileName().toUtf8().constData());
      QByteArray json;
      json.append(file.read(0x7ff));
      file.close();
      g_hillshadeProvider->clear();
      g_hillshadeProvider->append(QString::fromUtf8(json));
    }
  }

  g_favoritesFile = new QFile(g_usrResDir.absoluteFilePath(RES_FAVORITES_FILE), &app);
  g_GPXListModel = new GPXListModel(&app);
  g_GPXListModel->init(g_dataDir.absoluteFilePath(DIR_GPX));

  // initialize directories for database
  if (!g_appDir.mkpath(DIR_VOICES))
  {
    qWarning("Failed to create directory %s/%s", g_appDir.absolutePath().toUtf8().constData(), DIR_VOICES);
    return EXIT_FAILURE;
  }
  if (!g_appDir.mkpath(DIR_MAPS))
  {
    qWarning("Failed to create directory %s/%s", g_appDir.absolutePath().toUtf8().constData(), DIR_MAPS);
    return EXIT_FAILURE;
  }
  QStringList mapDirs;
  mapDirs.push_back(g_appDir.absoluteFilePath(DIR_MAPS));

  // register OSMScout library QML types
  osmscout::OSMScoutQt::RegisterQmlTypes(OSMIN_MODULE, 1, 0);

  {
    osmscout::OSMScoutQtBuilder builder = osmscout::OSMScoutQt::NewInstance()
        .WithUserAgent(OSMIN_MODULE, APP_VERSION)
        .WithBasemapLookupDirectory(g_appResDir.absoluteFilePath("world"))
        .WithStyleSheetDirectory(g_appResDir.absoluteFilePath("stylesheets"))
        .WithIconDirectory(g_appResDir.absoluteFilePath("icons"))
        .WithMapLookupDirectories(mapDirs)
        .WithVoiceLookupDirectory(g_appDir.absoluteFilePath(DIR_VOICES))
        .AddOnlineTileProviders(g_usrResDir.absoluteFilePath("online-tile-providers.json"))
        .AddMapProviders(g_usrResDir.absoluteFilePath("map-providers.json"))
        .AddVoiceProviders(g_usrResDir.absoluteFilePath("voice-providers.json"))
        .WithCacheLocation(QStandardPaths::writableLocation(QStandardPaths::CacheLocation).append("/tiles"))
        .WithTileCacheSizes(ONLINE_TILE_CACHE_MB, OFFLINE_TILE_CACHE_MB);

    // declare required types for tracks
    for (const QString& customType : GPXFileModel::customTypeSet())
      builder.AddCustomPoiType(customType);
    // declare required type for favorite poi
    builder.AddCustomPoiType("_waypoint_favorite");

    if (!builder.Init())
    {
        qWarning("Failed to initialize osmscout");
        return EXIT_FAILURE;
    }

    osmscout::log.Info(false);
    osmscout::log.Warn(false);
  }

  qmlRegisterType<osmscout::QmlSettings>(OSMIN_MODULE, 1, 0, "Settings");
  qmlRegisterSingletonType<PlatformExtras>(OSMIN_MODULE, 1, 0, "PlatformExtras", PlatformExtras::createPlatformExtras);
  qmlRegisterSingletonType<Converter>(OSMIN_MODULE, 1, 0, "Converter", Converter::createConverter);
  qmlRegisterType<osmin::QSortFilterProxyModelQML>(OSMIN_MODULE, 1, 0, "SortFilterModel");
  qmlRegisterUncreatableType<osmin::FilterBehavior>(OSMIN_MODULE, 1, 0, "FilterBehavior", "Not instantiable");
  qmlRegisterUncreatableType<osmin::SortBehavior>(OSMIN_MODULE, 1, 0, "SortBehavior", "Not instantiable");
  qmlRegisterSingletonType<osmin::Utils>(OSMIN_MODULE, 1, 0, "Utils", qUtilsInstance);
  qmlRegisterSingletonType<FavoritesModel>(OSMIN_MODULE, 1, 0, "FavoritesModel", qFavoritesModelInstance);
  qRegisterMetaType<FavoritesModel::FavoriteRoles>("FavoritesModel::Roles");
  qmlRegisterSingletonType<GPXListModel>(OSMIN_MODULE, 1, 0, "GPXListModel", qGPXListModelInstance);
  qRegisterMetaType<GPXListModel::GPXRoles>("GPXListModel::Roles");
  qmlRegisterType<GPXFileModel>(OSMIN_MODULE, 1, 0, "GPXFileModel");
  qRegisterMetaType<GPXFileModel::GPXObjectRoles>("GPXFileModel::Roles");
  qRegisterMetaType<QList<osmscout::OverlayObject*> >("QList<osmscout::OverlayObject*>");
  qmlRegisterSingletonType<RemoteService>(OSMIN_MODULE, 1, 0, "Service", qRemoteServiceInstance);
  qRegisterMetaType<RemoteService::ServiceStatus>("Service::ServiceStatus");
  qRegisterMetaType<RemoteService::PositioningMethods>("Service::PositioningMethods");
  qmlRegisterSingletonType<RemoteTracker>(OSMIN_MODULE, 1, 0, "Tracker", qRemoteTrackerInstance);
  qmlRegisterType<RemoteCompass>(OSMIN_MODULE, 1, 0, "Compass");
  qmlRegisterType<RemotePositionSource>(OSMIN_MODULE, 1, 0, "PositionSource");
  qmlRegisterType<RemotePosition>(OSMIN_MODULE, 1, 0, "Position");
  qmlRegisterSingletonType<MapExtras>(OSMIN_MODULE, 1, 0, "MapExtras", MapExtras::createMapExtras);

  QSettings settings;
  if (settings.value("style").isNull() || settings.value("firstRun", QVariant::fromValue(true)).toBool())
  {
#if defined(Q_OS_ANDROID)
    QQuickStyle::setStyle("Material");
#else
    QQuickStyle::setStyle("Material");
#endif
    settings.setValue("style", QQuickStyle::name());
  }
  QQuickStyle::setStyle(settings.value("style").toString());

  QQmlApplicationEngine engine;
  // bind version string
  engine.rootContext()->setContextProperty("VersionString", QString(APP_VERSION));
  // bind arguments
  engine.rootContext()->setContextProperty("ApplicationArguments", app.arguments());
  // bind SCALE_FACTOR
  engine.rootContext()->setContextProperty("ScreenScaleFactor", QVariant(app.primaryScreen()->devicePixelRatio()));
  // bind directories
  engine.rootContext()->setContextProperty("DataDirectory", g_dataDir.absolutePath());
  engine.rootContext()->setContextProperty("MapsDirectories", mapDirs);
  // bind hillshade provider
  engine.rootContext()->setContextProperty("HillshadeProvider", *g_hillshadeProvider);
  // bind flag Android
#if defined(Q_OS_ANDROID)
  engine.rootContext()->setContextProperty("Android", QVariant(true));
  engine.rootContext()->setContextProperty("DeviceMobile", QVariant(true));
#else
  engine.rootContext()->setContextProperty("Android", QVariant(false));
  // bind flag DeviceMobile
#if defined(DEVICE_MOBILE)
  engine.rootContext()->setContextProperty("DeviceMobile", QVariant(true));
#else
  engine.rootContext()->setContextProperty("DeviceMobile", QVariant(false));
#endif
#endif
  // select and bind styles available and known to work
  QStringList availableStyles;
#if defined(Q_OS_ANDROID)
  availableStyles.append("Material");
#else
  availableStyles.append("Material");
#endif
  engine.rootContext()->setContextProperty("AvailableStyles", availableStyles);

  // handle signal exit(int) issued by the qml instance
  QObject::connect(&engine, &QQmlApplicationEngine::exit, doExit);

#if defined(QT_STATICPLUGIN)
  importStaticPlugins(&engine);
#endif

  engine.load(QUrl("qrc:/controls2/osmin.qml"));
  if (engine.rootObjects().isEmpty()) {
      qWarning() << "Failed to load QML";
      return -1;
  }

  MemoryManagerPtr memoryManager(new MemoryManager(MEMORY_TARGET));

  ret = app.exec();

  // next run won't be the first
  if (settings.value("firstRun", QVariant::fromValue(true)).toBool())
  {
    settings.setValue("firstRun", QVariant::fromValue(false));
  }

  memoryManager->terminate();
  osmscout::OSMScoutQt::FreeInstance();
  serviceFrontend->terminate();

#if defined(Q_OS_ANDROID)
  QAndroidJniObject::callStaticMethod<void>("io/github/janbar/osmin/QtAndroidService",
                                                "stopQtAndroidService",
                                                "(Landroid/content/Context;)V",
                                                QtAndroid::androidActivity().object());
#else
  if (psvc->state() != QProcess::NotRunning)
  {
    psvc->terminate();
    if (!psvc->waitForFinished())
      psvc->kill();
  }
#endif

  return ret;
}

void setupApp(QGuiApplication& app)
{

  SignalHandler *sh = new SignalHandler(&app);
  sh->catchSignal(SIGHUP);
  sh->catchSignal(SIGALRM);
  sh->catchSignal(SIGQUIT);
  sh->catchSignal(SIGTERM);
  QObject::connect(sh, &SignalHandler::catched, &signalCatched);

  QLocale locale = QLocale::system();
  qInfo("User locale setting is %s", std::locale().name().c_str());
  // set translators
  prepareTranslator(app, QString(":/i18n"), QString(APP_TR_NAME), locale);
#ifdef Q_OS_MAC
  QDir appDir(app.applicationDirPath());
  if (appDir.cdUp() && appDir.cd("Resources/translations"))
    prepareTranslator(app, appDir.absolutePath(), "qt", locale);
#elif defined(Q_OS_ANDROID)
  prepareTranslator(app, "assets:/translations", "qt", locale);
#endif
  app.setWindowIcon(QIcon(QPixmap(":/images/osmin.png")));
}

void prepareTranslator(QGuiApplication& app, const QString& translationPath, const QString& translationPrefix, const QLocale& locale)
{
  QString i18Path(translationPath);
  i18Path.append("/").append(translationPrefix).append("_").append(locale.name().left(2)).append(".qm");
  QTranslator * translator = new QTranslator();
  if (!translator->load(locale, translationPrefix, QString("_"), translationPath))
  {
      qWarning("no file found for translations '%s' (using default).", i18Path.toUtf8().constData());
  }
  else
  {
      app.installTranslator(translator);
      qInfo("using file '%s' for translations.", i18Path.toUtf8().constData());
  }
}

void signalCatched(int signal)
{
  switch(signal)
  {
  case SIGQUIT:
  case SIGTERM:
    doExit(0);
  default:
    qDebug("Received signal %d", signal);
  }
}

void doExit(int code)
{
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
  if (code == 16)
  {
    // loop a short time to flush setting changes
    QTime syncTime = QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < syncTime)
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);

    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
  }
#else
  (void)code;
#endif
  QCoreApplication::quit();
}

bool testServiceUrl(const char *url, int timeout)
{
  QRemoteObjectNode node;
  node.connectToNode(QUrl(QString::fromUtf8(url)));
  QScopedPointer<ServiceMessengerReplica> replica(node.acquire<ServiceMessengerReplica>());
  return (replica->waitForSource(timeout));
}

QObject* qFavoritesModelInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  static FavoritesModel* _model = nullptr;
  if (_model == nullptr)
  {
    qInfo("Initialize favorites");
    _model = new FavoritesModel(g_favoritesFile);
    _model->init(g_favoritesFile);
    qInfo("Loading favorites");
    _model->loadData();
  }
  return _model;
}

QObject* qGPXListModelInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  return g_GPXListModel;
}

QObject* qRemoteTrackerInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  return g_remoteTracker;
}

QObject* qRemoteServiceInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  return g_remoteService;
}

QObject* qUtilsInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  static osmin::Utils* utils = nullptr;
  if (utils == nullptr)
    utils = new osmin::Utils();
  return utils;
}

QString basename(const QString& filepath)
{
  int p = filepath.lastIndexOf('/');
  if (p < 0)
    return filepath;
  p += 1;
  if (p == filepath.length())
    return basename(filepath.mid(0, p - 1));
  return filepath.mid(p);
}

bool migration(const QString& version, const QString& pathapp, const QString& pathdata)
{
  qWarning("Running migration");
  int p1 = version.indexOf('.', 0);
  if (p1 < 0)
  {
    qWarning("... Version string is invalid");
    return false;
  }
  unsigned v = version.left(p1).toInt() << 16;
  int p2 = version.indexOf('.', ++p1);
  if (p2 >= 0)
  {
    v += version.mid(p1, p2 - p1).toInt() << 8;
    v += version.mid(++p2).toInt();
  }

  QDir appDir(pathapp);
  QDir dataDir(pathdata);

  // from 1.11.0
  if (v < 0x010b00)
  {
#if defined(Q_OS_ANDROID)
    if (dataDir.exists("Maps"))
    {
      qWarning("... Purge directory 'Maps'");
      QDir _dir = dataDir;
      _dir.cd("Maps");
      _dir.removeRecursively();
      qWarning("... done");
    }
    if (dataDir.exists("voices"))
    {
      qWarning("... Purge directory 'voices'");
      QDir _dir = dataDir;
      _dir.cd("voices");
      _dir.removeRecursively();
      qWarning("... done");
    }
    if (dataDir.exists("resources"))
    {
      QDir resDir = dataDir;
      resDir.cd("resources");
      QStringList subs;
      subs.push_back("icons");
      subs.push_back("stylesheets");
      subs.push_back("world");
      for (const QString& sub : subs)
      {
        if (resDir.exists(sub))
        {
          qWarning("... Purge directory '%s'", sub.toUtf8().constData());
          QDir _dir = resDir;
          _dir.cd(sub);
          _dir.removeRecursively();
          qWarning("... done");
        }
      }
    }
#else
    QDir _dir = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    if (!appDir.exists(DIR_MAPS) && _dir.exists("Maps"))
    {
      qWarning("... Relocate map databases to %s/%s", APP_NAME, DIR_MAPS);
      _dir.rename("Maps", QString(APP_NAME).append('/').append(DIR_MAPS));
      qWarning("... done");
    }
#endif
  }

  return true;
}
