/*
 * Copyright (C) 2022
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
#ifndef SERVICEFRONTEND_H
#define SERVICEFRONTEND_H

#include <osmscoutclientqt/VehiclePosition.h>
#include <osmscoutclientqt/RouteStep.h>
#include <osmscoutgpx/TrackPoint.h>
#include <osmscoutgpx/Waypoint.h>

#include <QObject>
#include <QThread>
#include <QSharedPointer>
#include "rep_servicemessenger_replica.h"

class ServiceFrontend;

typedef QSharedPointer<ServiceFrontend> ServiceFrontendPtr;

class ServiceFrontend : public QObject
{
  Q_OBJECT

public:
  ServiceFrontend(const QString& url);
  virtual ~ServiceFrontend();
  ServiceFrontend(const ServiceFrontend&) = delete;

  void terminate();

signals:
  // callbacks
  void trackerRecordingFailed();
  void trackerResumeRecording();
  void trackerIsRecordingChanged(bool recording);
  void trackerRecordingChanged(const QString& filename);
  void trackerProcessingChanged(bool processing);
  void trackerPositionRecorded(double lat, double lon);
  void trackerPositionMarked(double lat, double lon, const QString& symbol, const QString& name);
  void trackerPositionChanged(bool valid, double lat, double lon, double bearing);
  void trackerDataChanged(
                double elevation,
                double currentSpeed,
                double distance,
                double duration,
                double ascent,
                double descent,
                double maxSpeed);

  void compassReadingChanged(float azimuth, float calibration);
  void compassDataRateChanged(int dataRate);
  void compassActiveChanged(bool active);

  void positionPositionUpdated(bool valid, double lat, double lon, bool haccvalid, float hacc, double alt);
  void positionActiveChanged(bool active);
  void positionUpdateIntervalChanged(int interval);
  void positionPreferredPositioningMethodsChanged(int methods);

  void serviceDisconnected();
  void serviceConnected();

public slots:
  void ping(const QString& message);
  void setRecording(const QString& filename);
  void resetTrackingData();
  void startRecording();
  void stopRecording();
  void pinPosition();
  void markPosition(const QString& symbol, const QString& name, const QString& description);
  void setCompassDataRate(int datarate);
  void setPositionUpdateInterval(int interval);
  void setPreferedPositioningMethods(int methods);
  void positionStartUpdates();
  void positionStopUpdates();

private slots:
  void run();
  void onStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState);
  void onFinished();

private:
  QString m_url;
  QThread * m_t;
  QRemoteObjectNode * m_node = nullptr;
  QSharedPointer<ServiceMessengerReplica> m_messenger;
};

#endif // SERVICEFRONTEND_H
