#include <QtGlobal>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#ifdef SAILFISHOS
#include <sailfishapp/sailfishapp.h>
#include <QQuickView>
#else
#include <QtQuickControls2>
#endif
#include <QStandardPaths>
#include <QTranslator>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QPixmap>
#include <QIcon>
#include <QTime>

//#define Q_OS_ANDROID

#define DIR_MAPS          "Maps"
#define DIR_VOICES        "voices"
#define DIR_RES           "resources"
#define APP_TR_NAME       "osmin"
#ifdef SAILFISHOS
#define ORG_NAME          "harbour-osmin"
#define APP_NAME          "harbour-osmin"
#define APP_DISPLAY_NAME  "Osmin"
#define APP_ID            APP_NAME
#define AUTO_MOUNT        "/run/media/"
#else
#define ORG_NAME          "io.github.janbar"
#define APP_NAME          "osmin"
#define APP_DISPLAY_NAME  "Osmin"
#ifdef Q_OS_ANDROID
#define APP_ID            ORG_NAME "." APP_NAME
#define AUTO_MOUNT        "/storage/"
#else
#define APP_ID            APP_NAME
#define AUTO_MOUNT        "/media/"
#endif
#endif

#include "signalhandler.h"

#ifndef APP_VERSION
#define APP_VERSION "Undefined"
#endif

#define OSMIN_MODULE              "Osmin"
#define RES_GPX_DIR               "GPX"
#define RES_FAVORITES_FILE        "favorites.csv"
#define RES_HILLSHADE_SERVER_FILE "hillshade-tile-server.json"
#define RES_HILLSHADE_FILE_SAMPLE "hillshade-tile-server.json.sample"

#include <osmscout/OSMScoutQt.h>
// Custom QML objects
#include <osmscout/MapWidget.h>
#include <osmscout/SearchLocationModel.h>
#include <osmscout/RoutingModel.h>
#include <osmscout/AvailableMapsModel.h>
#include <osmscout/MapDownloadsModel.h>
#include <osmscout/Settings.h>
#include <osmscout/util/Logger.h>

#include "platformextras.h"
#include "converter.h"
#include "favoritesmodel.h"
#include "gpxlistmodel.h"
#include "gpxfilemodel.h"
#include "tracker.h"
#include "qmlsortfiltermodel.h"
#include "utils.h"
#include "compass/plugin.h"

void setupApp(QGuiApplication& app);
void prepareTranslator(QGuiApplication& app, const QString& translationPath, const QString& translationPrefix, const QLocale& locale);
void doExit(int code);

QFile*            g_favoritesFile         = nullptr;
GPXListModel*     g_GPXListModel          = nullptr;
Tracker*          g_Tracker               = nullptr;
QString*          g_hillshadeProvider     = nullptr;

QObject* getFavoritesModel(QQmlEngine *engine, QJSEngine *scriptEngine);
QObject* getGPXListModel(QQmlEngine *engine, QJSEngine *scriptEngine);
QObject* getTracker(QQmlEngine *engine, QJSEngine *scriptEngine);
QObject* getUtils(QQmlEngine *engine, QJSEngine *scriptEngine);

#if defined(QT_STATICPLUGIN)
void importStaticPlugins(QQmlEngine* engine)
{
  //{ myPlugin e; e.initializeEngine(engine, "pluginName"); e.registerTypes("pluginName"); }
}
#endif

int main(int argc, char *argv[])
{
    int ret = 0;

    QGuiApplication::setApplicationName(APP_NAME);
    QGuiApplication::setApplicationDisplayName(APP_DISPLAY_NAME);
    QGuiApplication::setOrganizationName(ORG_NAME);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    setupApp(app);

    // check for the resource directory
    QDir dataDir = QDir(PlatformExtras::getDataDir(APP_ID));
    if (!dataDir.cd(DIR_RES))
      return EXIT_FAILURE;
    QDir homeDir = QDir(PlatformExtras::getHomeDir());
#ifdef Q_OS_ANDROID
    if (!homeDir.mkpath(DIR_RES))
      return EXIT_FAILURE;
#else
    /* ~/osmin/resources */
    if (!homeDir.mkpath(QString(APP_NAME).append("/").append(DIR_RES)))
      return EXIT_FAILURE;
    homeDir.cd(APP_NAME);
#endif
    QDir resDir = QDir(homeDir.absoluteFilePath(DIR_RES));
    QFile resVersion(resDir.absoluteFilePath("version"));
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
        if (folder.length() == 0 || resDir.exists(folder) || resDir.mkpath(folder))
        {
          QDir assets(dataDir.absoluteFilePath(folder));
          for (QFileInfo& asset : assets.entryInfoList())
          {
            if (asset.isFile())
            {
              QString filename = resDir.absoluteFilePath(folder).append('/').append(asset.fileName());
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
          qWarning("Failed to create path %s", resDir.absoluteFilePath(folder).toUtf8().constData());
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
    qInfo("Resource directory is %s", resDir.path().toUtf8().constData());
    g_favoritesFile = new QFile(resDir.absoluteFilePath(RES_FAVORITES_FILE), &app);
    g_hillshadeProvider = new QString("{}");
    // create the hillshade server file from sample if needed
    if (!resDir.exists(RES_HILLSHADE_SERVER_FILE) && resDir.exists(RES_HILLSHADE_FILE_SAMPLE))
    {
      qInfo("Create hillshade tile server file '%s' from sample", resDir.absoluteFilePath(RES_HILLSHADE_SERVER_FILE).toUtf8().constData());
      QFile::copy(resDir.absoluteFilePath(RES_HILLSHADE_FILE_SAMPLE), resDir.absoluteFilePath(RES_HILLSHADE_SERVER_FILE));
    }
    // load hillshade server file
    if (resDir.exists(RES_HILLSHADE_SERVER_FILE))
    {
      QFile file(resDir.absoluteFilePath(RES_HILLSHADE_SERVER_FILE));
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

    if (!homeDir.exists("GPX"))
      homeDir.mkdir("GPX");
    g_GPXListModel = new GPXListModel(&app);
    g_GPXListModel->init(homeDir.absoluteFilePath(RES_GPX_DIR));
    g_Tracker = new Tracker(&app);
    g_Tracker->init(homeDir.absoluteFilePath(RES_GPX_DIR));

    if (!homeDir.exists(DIR_VOICES))
      homeDir.mkdir(DIR_VOICES);

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
#elif defined(SAILFISHOS)
    storages.push_back(PlatformExtras::getHomeDir());
    storages.append(PlatformExtras::getStorageDirs());
    for (QString& storage : storages)
    {
      QDir dir(storage);
      if (!dir.exists(DIR_MAPS) && !dir.mkdir(DIR_MAPS))
        qWarning("Failed to create maps storage at %s", dir.absoluteFilePath(DIR_MAPS).toUtf8().constData());
      else
      {
        mapDirs.push_back(dir.absoluteFilePath(DIR_MAPS));
        break; // no more 2
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
           .WithBasemapLookupDirectory(resDir.absoluteFilePath("world"))
           .WithStyleSheetDirectory(resDir.absoluteFilePath("stylesheets"))
           .WithIconDirectory(resDir.absoluteFilePath("icons"))
           .WithMapLookupDirectories(mapDirs)
           .WithOnlineTileProviders(resDir.absoluteFilePath("online-tile-providers.json"))
           .WithMapProviders(resDir.absoluteFilePath("map-providers.json"))
           .WithVoiceLookupDirectory(homeDir.absoluteFilePath(DIR_VOICES))
           .WithVoiceProviders(resDir.absoluteFilePath("voice-providers.json"))
           .WithCacheLocation(QStandardPaths::writableLocation(QStandardPaths::CacheLocation).append("/tiles"))
           .WithTileCacheSizes(60, 200)
           .AddCustomPoiType("_waypoint");

      for (const QString& trk : GPXFileModel::trackTypeSet())
        builder.AddCustomPoiType(trk);

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
    qmlRegisterSingletonType<osmin::Utils>(OSMIN_MODULE, 1, 0, "Utils", getUtils);
    qmlRegisterSingletonType<FavoritesModel>(OSMIN_MODULE, 1, 0, "FavoritesModel", getFavoritesModel);
    qRegisterMetaType<FavoritesModel::FavoriteRoles>("FavoritesModel::Roles");
    qmlRegisterSingletonType<GPXListModel>(OSMIN_MODULE, 1, 0, "GPXListModel", getGPXListModel);
    qRegisterMetaType<GPXListModel::GPXRoles>("GPXListModel::Roles");
    qmlRegisterType<GPXFileModel>(OSMIN_MODULE, 1, 0, "GPXFileModel");
    qRegisterMetaType<GPXFileModel::GPXObjectRoles>("GPXFileModel::Roles");
    qRegisterMetaType<QList<osmscout::OverlayObject*> >("QList<osmscout::OverlayObject*>");
    qmlRegisterSingletonType<Tracker>(OSMIN_MODULE, 1, 0, "Tracker", getTracker);

    // register the generic compass
    qmlRegisterType<BuiltInCompass>(OSMIN_MODULE, 1, 0, "Compass");
    QScopedPointer<BuiltInSensorPlugin> sensor(new BuiltInSensorPlugin());
    sensor->registerSensors();

    QSettings settings;
#ifndef SAILFISHOS
    QString style = QQuickStyle::name();
    if (!style.isEmpty())
        settings.setValue("style", style);
    else
    {
        if (settings.value("style").isNull())
        {
#if defined(Q_OS_ANDROID)
            QQuickStyle::setStyle("Material");
#else
            QQuickStyle::setStyle("Material");
#endif
            settings.setValue("style", QQuickStyle::name());
        }
        QQuickStyle::setStyle(settings.value("style").toString());
    }

    QQmlApplicationEngine engine;
    // bind version string
    engine.rootContext()->setContextProperty("VersionString", QString(APP_VERSION));
    // bind arguments
    engine.rootContext()->setContextProperty("ApplicationArguments", app.arguments());
    // bind SCALE_FACTOR
    engine.rootContext()->setContextProperty("ScreenScaleFactor", QVariant(app.primaryScreen()->devicePixelRatio()));
    // bind directories
    engine.rootContext()->setContextProperty("DataDirectory", homeDir.absolutePath());
    engine.rootContext()->setContextProperty("MapsDirectories", mapDirs);
    // bind hillshade provider
    engine.rootContext()->setContextProperty("HillshadeProvider", *g_hillshadeProvider);
    // bind Android flag
#if defined(Q_OS_ANDROID)
    engine.rootContext()->setContextProperty("Android", QVariant(true));
#else
    engine.rootContext()->setContextProperty("Android", QVariant(false));
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
#else
    QScopedPointer<QQuickView> view(SailfishApp::createView());
#ifdef QT_STATICPLUGIN
    importStaticPlugins(view->engine());
#endif
    QObject::connect(view->engine(), &QQmlApplicationEngine::quit, &app, QCoreApplication::quit);
    // bind version string
    view->engine()->rootContext()->setContextProperty("VersionString", QString(APP_VERSION));
    // bind arguments
    view->engine()->rootContext()->setContextProperty("ApplicationArguments", app.arguments());
    // bind SCALE_FACTOR
    view->engine()->rootContext()->setContextProperty("ScreenScaleFactor", QVariant(app.primaryScreen()->devicePixelRatio()));
    // bind directories
    view->engine()->rootContext()->setContextProperty("DataDirectory", homeDir.absolutePath());
    view->engine()->rootContext()->setContextProperty("MapsDirectories", mapDirs);
    // bind hillshade provider
    view->engine()->rootContext()->setContextProperty("HillshadeProvider", *g_hillshadeProvider);
    view->setSource(QUrl("qrc:/silica/osmin.qml"));
    view->showFullScreen();
#endif

    ret = app.exec();
    osmscout::OSMScoutQt::FreeInstance();
    return ret;
}

void setupApp(QGuiApplication& app) {

    SignalHandler *sh = new SignalHandler(&app);
    sh->catchSignal(SIGHUP);
    sh->catchSignal(SIGALRM);

    // set translators
    QLocale locale = QLocale::system();
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
    QTranslator * translator = new QTranslator();
    if (!translator->load(locale, translationPrefix, QString("_"), translationPath))
    {
        qWarning() << "no file found for translations '"+ translationPath + "/" + translationPrefix + "_" + locale.name().left(2) + ".qm' (using default).";
    }
    else
    {
        qInfo() << "using file '"+ translationPath + "/" + translationPrefix + "_" + locale.name().left(2) + ".qm ' for translations.";
        app.installTranslator(translator);
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

QObject* getFavoritesModel(QQmlEngine *engine, QJSEngine *scriptEngine)
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

QObject* getGPXListModel(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  return g_GPXListModel;
}

QObject* getTracker(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  return g_Tracker;
}

QObject* getUtils(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  static osmin::Utils* utils = nullptr;
  if (utils == nullptr)
    utils = new osmin::Utils();
  return utils;
}
