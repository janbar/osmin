/*
 * Copyright (C) 2024
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
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QTimer>
#include <QTime>

#include <locale>

#define APP_TR_NAME       "osmin"                   // translations base name
#define ORG_NAME          "io.github.janbar"        // organisation id
#define APP_NAME          "osmin"                   // application name
#define APP_DISPLAY_NAME  "Osmin"                   // application display name
#define APP_ID            APP_NAME                  // Unix default application id

#include <signalhandler.h>

#ifndef APP_VERSION
#define APP_VERSION "Undefined"
#endif

#include <converter.h>
#include <service.h>
#include <servicefrontend.h>

#include "commandline.h"
#include "simulator.h"
#include "simulatorbreaker.h"

int startService(int argc, char* argv[]);
void setupApp(QCoreApplication& app);
void signalCatched(int signal);
void doExit(int code);
void messageOutput(QtMsgType type, const QMessageLogContext &, const QString & msg);
bool testServiceUrl(const char *url, int timeout);
QString basename(const QString& filepath);

SimulatorBreaker * g_breaker = nullptr;

int main(int argc, char *argv[])
{
  int ret = 0;

  fprintf(stdout, "Simulation tool for OSMIN navigator\n"
                  "Version " APP_VERSION " compiled on " __DATE__ " at " __TIME__ "\n");

  ret = startService(argc, argv);

  return ret;
}

int startService(int argc, char* argv[])
{
  int ret = 0;
  QCoreApplication::setApplicationName(APP_NAME);
  QCoreApplication::setOrganizationName(ORG_NAME);

  qRegisterMetaType<osmscout::PositionAgent::PositionState>("osmscout::PositionAgent::PositionState");
  qRegisterMetaType<std::optional<osmscout::Bearing>>("std::optional<osmscout::Bearing>");
  qRegisterMetaType<osmscout::GeoCoord>("osmscout::GeoCoord");
  qInstallMessageHandler(messageOutput);

  QCoreApplication app(argc, argv);

  if (testServiceUrl(SERVICE_URL, 100))
  {
    fprintf(stdout, "\nPlease stop the running tracker or OSMIN application.\n"
                    "The simulation tool must be started first.\n");
    return EXIT_FAILURE;
  }
  fprintf(stdout, "Type HELP, for a list of commands.\n");

  setupApp(app);

  QDir dataDir(".");

  Simulator * simulator = new Simulator();
  g_breaker = simulator;

  CommandLine * cmd = new CommandLine();
  simulator->enableCLI(cmd);

  Service * svc = new Service(SERVICE_URL, dataDir.absolutePath()
                             , simulator->compassSensor(), simulator->positionSource());
  QTimer::singleShot(0, svc, &Service::run);
  ret = app.exec();

  g_breaker = nullptr;
  delete simulator;
  delete cmd;
  delete svc;

  return ret;
}

void setupApp(QCoreApplication& app)
{

  SignalHandler *sh = new SignalHandler(&app);
  sh->catchSignal(SIGHUP);
  sh->catchSignal(SIGINT);
  sh->catchSignal(SIGALRM);
  sh->catchSignal(SIGQUIT);
  sh->catchSignal(SIGTERM);
  QObject::connect(sh, &SignalHandler::catched, &signalCatched);

  qInfo("User locale setting is %s", std::locale().name().c_str());
  (void)app;
}

void signalCatched(int signal)
{
  switch(signal)
  {
    case SIGINT:
    if (g_breaker && !g_breaker->onKeyBreak())
      break;
    case SIGQUIT:
    case SIGTERM:
      doExit(0);
    default:
      qDebug("Received signal %d", signal);
  }
}

void doExit(int code)
{
  (void)code;
  QCoreApplication::quit();
}

void messageOutput(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
  switch (type) {
  case QtDebugMsg:
    //fprintf(stderr, "D: %s\n", msg.toStdString().c_str());
    break;
  case QtInfoMsg:
    //fprintf(stderr, "I: %s\n", msg.toStdString().c_str());
    break;
  case QtWarningMsg:
    fprintf(stderr, "W: %s\n", msg.toStdString().c_str());
    break;
  case QtCriticalMsg:
    fprintf(stderr, "E: %s\n", msg.toStdString().c_str());
    break;
  case QtFatalMsg:
    fprintf(stderr, "F: %s\n", msg.toStdString().c_str());
    abort();
  }
}

bool testServiceUrl(const char *url, int timeout)
{
  QRemoteObjectNode node;
  node.connectToNode(QUrl(QString::fromUtf8(url)));
  QScopedPointer<ServiceMessengerReplica> replica(node.acquire<ServiceMessengerReplica>());
  return (replica->waitForSource(timeout));
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
