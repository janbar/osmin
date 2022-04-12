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

#define DIR_MAPS          "Maps"
#define DIR_VOICES        "voices"
#define DIR_RES           "resources"
#define APP_TR_NAME       "osmin"                   // translations base name
#define ORG_NAME          "io.github.janbar"        // organisation id
#define APP_NAME          "osmin"                   // application name
#define APP_DISPLAY_NAME  "Osmin"                   // application display name
#ifdef Q_OS_ANDROID
#define APP_ID            ORG_NAME "." APP_NAME     // Android application id
#define AUTO_MOUNT        "/storage/"               // external mount base
#else
#define APP_ID            APP_NAME                  // Unix default application id
#define AUTO_MOUNT        "/media/"                 // external mount base
#endif

#include "signalhandler.h"

#ifndef APP_VERSION
#define APP_VERSION "Undefined"
#endif

/* The name of the service end-point must include the revision of the API */
#define SERVICE_URL                 "local:osmin42"
#define SERVICE_FLAG                "-service"

#define OSMIN_MODULE                "Osmin"         // QML module name
#define RES_GPX_DIR                 "GPX"
#define RES_FAVORITES_FILE          "favorites.csv"
#define RES_HILLSHADE_SERVER_FILE   "hillshade-tile-server.json"
#define RES_HILLSHADE_FILE_SAMPLE   "hillshade-tile-server.json.sample"

#include <osmscoutclientqt/OSMScoutQt.h>
// Custom QML objects
#include <osmscoutclientqt/MapWidget.h>
#include <osmscoutclientqt/SearchLocationModel.h>
#include <osmscoutclientqt/RoutingModel.h>
#include <osmscoutclientqt/AvailableMapsModel.h>
#include <osmscoutclientqt/MapDownloadsModel.h>
#include <osmscoutclientqt/Settings.h>
#include <osmscout/util/Logger.h>

#include "platformextras.h"
#include "converter.h"
#include "favoritesmodel.h"
#include "gpxlistmodel.h"
#include "gpxfilemodel.h"
#include "qmlsortfiltermodel.h"
#include "utils.h"
#include "compass/plugin.h"

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

QDir g_dataDir;
QDir g_homeDir;
QDir g_resDir;

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

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidService>
#include <QAndroidJniObject>
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
  QAndroidJniObject service = QtAndroid::androidService();
  QAndroidJniObject nullstr = QAndroidJniObject::fromString("");
  QAndroidJniObject file = service.callObjectMethod("getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;", nullstr.object<jstring>());
  QAndroidJniObject path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
  g_homeDir = QDir(path.toString());
  if (!g_homeDir.mkpath(DIR_RES))
    return EXIT_FAILURE;
#else
  QCoreApplication app(argc, argv);
  g_homeDir = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  /* ~/osmin/resources */
  if (!g_homeDir.mkpath(QString(APP_NAME).append("/").append(DIR_RES)))
    return EXIT_FAILURE;
  g_homeDir.cd(APP_NAME);
#endif
  g_resDir = QDir(g_homeDir.absoluteFilePath(DIR_RES));
  if (!g_homeDir.mkpath(RES_GPX_DIR))
    return EXIT_FAILURE;

  Service * svc = new Service(SERVICE_URL, g_homeDir.absoluteFilePath(RES_GPX_DIR));
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

  // check for the resource directory
  g_dataDir = QDir(PlatformExtras::getDataDir(APP_ID));
  if (!g_dataDir.cd(DIR_RES))
    return EXIT_FAILURE;
  g_homeDir = QDir(PlatformExtras::getHomeDir());
#ifdef Q_OS_ANDROID
  {
    QStringList androidPermissions;
    androidPermissions.append("android.permission.ACCESS_FINE_LOCATION");
    QtAndroid::requestPermissionsSync(androidPermissions);
  }
  if (!g_homeDir.mkpath(DIR_RES))
    return EXIT_FAILURE;
#else
  /* ~/osmin/resources */
  if (!g_homeDir.mkpath(QString(APP_NAME).append("/").append(DIR_RES)))
    return EXIT_FAILURE;
  g_homeDir.cd(APP_NAME);
#endif
  g_resDir = QDir(g_homeDir.absoluteFilePath(DIR_RES));
  if (!g_homeDir.mkpath(RES_GPX_DIR))
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

  // check assets
  QFile resVersion(g_resDir.absoluteFilePath("version"));
  if (resVersion.exists())
  {
    qInfo("Checking installed assets...");
    QByteArray _version;
    if (resVersion.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      _version = resVersion.readLine(64);
      resVersion.close();
    }
    qInfo("Found assets version %s", _version.constData());
    if (strlen(APP_VERSION) != _version.length() || ::strncmp(APP_VERSION, _version.constData(), _version.length()) != 0)
    {
      QFile::remove(resVersion.fileName());
      qWarning("Assets will be upgraded to version %s", APP_VERSION);
    }
  }
  if (!resVersion.exists())
  {
    QStringList folders;
    folders.push_back("");
    folders.push_back("icons");
    folders.push_back("stylesheets");
    folders.push_back("world");
    for (QString& folder : folders)
    {
      if (folder.length() == 0 || g_resDir.exists(folder) || g_resDir.mkpath(folder))
      {
        QDir assets(g_dataDir.absoluteFilePath(folder));
        for (QFileInfo& asset : assets.entryInfoList())
        {
          if (asset.isFile())
          {
            QString filename = g_resDir.absoluteFilePath(folder).append('/').append(asset.fileName());
            if ((!QFile::exists(filename) || QFile::remove(filename)) &&
                QFile::copy(asset.absoluteFilePath(), filename))
              continue;
            qWarning("Failed to install file %s", asset.fileName().toUtf8().constData());
            return EXIT_FAILURE;
          }
        }
      }
      else
      {
        qWarning("Failed to create path %s", g_resDir.absoluteFilePath(folder).toUtf8().constData());
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
  qInfo("Resource directory is %s", g_resDir.path().toUtf8().constData());
  g_favoritesFile = new QFile(g_resDir.absoluteFilePath(RES_FAVORITES_FILE), &app);
  g_hillshadeProvider = new QString("{}");
  // create the hillshade server file from sample if needed
  //if (!g_resDir.exists(RES_HILLSHADE_SERVER_FILE) && g_resDir.exists(RES_HILLSHADE_FILE_SAMPLE))
  //{
  //  qInfo("Create hillshade tile server file '%s' from sample", g_resDir.absoluteFilePath(RES_HILLSHADE_SERVER_FILE).toUtf8().constData());
  //  QFile::copy(g_resDir.absoluteFilePath(RES_HILLSHADE_FILE_SAMPLE), g_resDir.absoluteFilePath(RES_HILLSHADE_SERVER_FILE));
  //}
  // load hillshade server file
  if (g_resDir.exists(RES_HILLSHADE_SERVER_FILE))
  {
    QFile file(g_resDir.absoluteFilePath(RES_HILLSHADE_SERVER_FILE));
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

  g_GPXListModel = new GPXListModel(&app);
  g_GPXListModel->init(g_homeDir.absoluteFilePath(RES_GPX_DIR));

  if (!g_homeDir.exists(DIR_VOICES))
    g_homeDir.mkdir(DIR_VOICES);

  // initialize the map directories
  QStringList mapDirs;
  QStringList storages;

#ifdef Q_OS_ANDROID
  storages.append(PlatformExtras::getStorageDirs());
  for (QString& storage : storages)
  {
    QDir dir(storage);
    if (!dir.exists(DIR_MAPS) && !dir.mkdir(DIR_MAPS))
      qWarning("Failed to create maps storage at %s", dir.absoluteFilePath(DIR_MAPS).toUtf8().constData());
    else
    {
      mapDirs.push_back(dir.absoluteFilePath(DIR_MAPS));
      if (mapDirs.length() > 1) // no more 2 (internal + external sdcard)
        break;
    }
  }
#else
  storages.push_back(PlatformExtras::getHomeDir());
  for (QString& storage : storages)
  {
    QDir dir(storage);
    if (!dir.exists(DIR_MAPS) && !dir.mkdir(DIR_MAPS))
      qWarning("Failed to create maps storage at %s", dir.absoluteFilePath(DIR_MAPS).toUtf8().constData());
    else
    {
      mapDirs.push_back(dir.absoluteFilePath(DIR_MAPS));
      break;
    }
  }
#endif

  if (mapDirs.length() == 0)
  {
    qWarning("No available storage for maps");
    return EXIT_FAILURE;
  }


  // register OSMScout library QML types
  osmscout::OSMScoutQt::RegisterQmlTypes(OSMIN_MODULE, 1, 0);

  {
    osmscout::OSMScoutQtBuilder builder = osmscout::OSMScoutQt::NewInstance()
         .WithUserAgent(OSMIN_MODULE, APP_VERSION)
         .WithBasemapLookupDirectory(g_resDir.absoluteFilePath("world"))
         .WithStyleSheetDirectory(g_resDir.absoluteFilePath("stylesheets"))
         .WithIconDirectory(g_resDir.absoluteFilePath("icons"))
         .WithMapLookupDirectories(mapDirs)
         .WithOnlineTileProviders(g_resDir.absoluteFilePath("online-tile-providers.json"))
         .WithMapProviders(g_resDir.absoluteFilePath("map-providers.json"))
         .WithVoiceLookupDirectory(g_homeDir.absoluteFilePath(DIR_VOICES))
         .WithVoiceProviders(g_resDir.absoluteFilePath("voice-providers.json"))
         .WithCacheLocation(QStandardPaths::writableLocation(QStandardPaths::CacheLocation).append("/tiles"))
         .WithTileCacheSizes(60, 200);

    for (const QString& customType : GPXFileModel::customTypeSet())
      builder.AddCustomPoiType(customType);

    if (!builder.Init())
    {
        qWarning("Failed to initialize osmscout");
        return EXIT_FAILURE;
    }
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
  engine.rootContext()->setContextProperty("DataDirectory", g_homeDir.absolutePath());
  engine.rootContext()->setContextProperty("MapsDirectories", mapDirs);
  // bind hillshade provider
  engine.rootContext()->setContextProperty("HillshadeProvider", *g_hillshadeProvider);
  // bind flag Android
#if defined(Q_OS_ANDROID)
  engine.rootContext()->setContextProperty("Android", QVariant(true));
#else
  engine.rootContext()->setContextProperty("Android", QVariant(false));
#endif
  // bind flag DeviceMobile
#if defined(DEVICE_MOBILE)
  engine.rootContext()->setContextProperty("DeviceMobile", QVariant(true));
#else
  engine.rootContext()->setContextProperty("DeviceMobile", QVariant(false));
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

  ret = app.exec();

  // next run won't be the first
  if (settings.value("firstRun", QVariant::fromValue(true)).toBool())
  {
    settings.setValue("firstRun", QVariant::fromValue(false));
  }

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
