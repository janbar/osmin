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

#include "remotetracker.h"
#include "servicefrontend.h"

RemoteTracker::RemoteTracker(QObject *parent) : QObject(parent)
, m_vehicle(osmscout::Vehicle::vehicleCar)
, m_vehicleState(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_vehicleCoord(0, 0)
, m_vehicleBearing(osmscout::Bearing())
, m_elevation(0)
, m_currentSpeed(0)
, m_distance(0)
, m_duration(0)
, m_ascent(0)
, m_descent(0)
, m_maxSpeed(0)
, m_busy(false)
, m_recording("")
, m_isRecording(false)
{ }

osmscout::VehiclePosition *RemoteTracker::getTrackerPosition() const
{
  std::optional<osmscout::GeoCoord> nextPosition;
  //if (!m_nextRouteStep.getType().isEmpty())
  //  nextPosition = std::make_shared<osmscout::GeoCoord>(m_nextRouteStep.GetCoord());
  return new osmscout::VehiclePosition(m_vehicle, m_vehicleState, m_vehicleCoord, m_vehicleBearing, nextPosition);
}

double RemoteTracker::getBearing() const
{
  if (m_vehicleBearing.has_value())
    return m_vehicleBearing->AsRadians();
  return 0.0;
}

void RemoteTracker::connectToService(QVariant service)
{
  ServiceFrontendPtr _service = service.value<ServiceFrontendPtr>();
  connectToService(_service);
}

void RemoteTracker::connectToService(ServiceFrontendPtr& service)
{
  if (m_service)
  {
    // callbacks
    disconnect(m_service.data(), &ServiceFrontend::trackerRecordingFailed, this, &RemoteTracker::recordingFailed);
    disconnect(m_service.data(), &ServiceFrontend::trackerResumeRecording, this, &RemoteTracker::resumeRecording);
    disconnect(m_service.data(), &ServiceFrontend::trackerIsRecordingChanged, this, &RemoteTracker::_trackerIsRecordingChanged);
    disconnect(m_service.data(), &ServiceFrontend::trackerRecordingChanged, this, &RemoteTracker::_trackerRecordingChanged);
    disconnect(m_service.data(), &ServiceFrontend::trackerProcessingChanged, this, &RemoteTracker::_trackerProcessingChanged);
    disconnect(m_service.data(), &ServiceFrontend::trackerPositionChanged, this, &RemoteTracker::_trackerPositionChanged);
    disconnect(m_service.data(), &ServiceFrontend::trackerPositionRecorded, this, &RemoteTracker::trackerPositionRecorded);
    disconnect(m_service.data(), &ServiceFrontend::trackerPositionMarked, this, &RemoteTracker::trackerPositionMarked);
    disconnect(m_service.data(), &ServiceFrontend::trackerDataChanged, this, &RemoteTracker::_trackerDataChanged);
    // operations
    disconnect(this, &RemoteTracker::reset, m_service.data(), &ServiceFrontend::resetTrackingData);
    disconnect(this, &RemoteTracker::startRecording, m_service.data(), &ServiceFrontend::startRecording);
    disconnect(this, &RemoteTracker::stopRecording, m_service.data(), &ServiceFrontend::stopRecording);
    disconnect(this, &RemoteTracker::pinPosition, m_service.data(), &ServiceFrontend::pinPosition);
    disconnect(this, &RemoteTracker::markPosition, m_service.data(), &ServiceFrontend::markPosition);
  }
  m_service = service;
  if (m_service)
  {
    // callbacks
    connect(m_service.data(), &ServiceFrontend::trackerRecordingFailed, this, &RemoteTracker::recordingFailed);
    connect(m_service.data(), &ServiceFrontend::trackerResumeRecording, this, &RemoteTracker::resumeRecording);
    connect(m_service.data(), &ServiceFrontend::trackerIsRecordingChanged, this, &RemoteTracker::_trackerIsRecordingChanged);
    connect(m_service.data(), &ServiceFrontend::trackerRecordingChanged, this, &RemoteTracker::_trackerRecordingChanged);
    connect(m_service.data(), &ServiceFrontend::trackerProcessingChanged, this, &RemoteTracker::_trackerProcessingChanged);
    connect(m_service.data(), &ServiceFrontend::trackerPositionChanged, this, &RemoteTracker::_trackerPositionChanged);
    connect(m_service.data(), &ServiceFrontend::trackerPositionRecorded, this, &RemoteTracker::trackerPositionRecorded);
    connect(m_service.data(), &ServiceFrontend::trackerPositionMarked, this, &RemoteTracker::trackerPositionMarked);
    connect(m_service.data(), &ServiceFrontend::trackerDataChanged, this, &RemoteTracker::_trackerDataChanged);
    // operations
    connect(this, &RemoteTracker::reset, m_service.data(), &ServiceFrontend::resetTrackingData);
    connect(this, &RemoteTracker::startRecording, m_service.data(), &ServiceFrontend::startRecording);
    connect(this, &RemoteTracker::stopRecording, m_service.data(), &ServiceFrontend::stopRecording);
    connect(this, &RemoteTracker::pinPosition, m_service.data(), &ServiceFrontend::pinPosition);
    connect(this, &RemoteTracker::markPosition, m_service.data(), &ServiceFrontend::markPosition);
  }
}

void RemoteTracker::_trackerIsRecordingChanged(bool recording)
{
  m_isRecording = recording;
  emit trackerIsRecordingChanged();
}

void RemoteTracker::_trackerRecordingChanged(const QString &filename)
{
  m_recording = filename;
  emit trackerRecordingChanged();
}

void RemoteTracker::_trackerProcessingChanged(bool processing)
{
  m_busy = processing;
  emit trackerProcessingChanged();
}

void RemoteTracker::_trackerPositionChanged(bool valid, double lat, double lon, double bearing)
{
  if (!valid)
    m_vehicleState = osmscout::PositionAgent::PositionState::NoGpsSignal;
  else
    m_vehicleState = osmscout::PositionAgent::PositionState::OffRoute;
  m_vehicleCoord.Set(lat, lon);
  m_vehicleBearing = osmscout::Bearing::Radians(bearing);
  emit trackerPositionChanged();
}

void RemoteTracker::_trackerDataChanged(double elevation, double currentSpeed, double distance, double duration, double ascent, double descent, double maxSpeed)
{
  m_elevation = elevation;
  m_currentSpeed = currentSpeed;
  m_distance = distance;
  m_duration = duration;
  m_ascent = ascent;
  m_descent = descent;
  m_maxSpeed = maxSpeed;
  emit trackerDataChanged();
}
