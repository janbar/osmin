
#include "tracker.h"
#include "utils.h"

#include <cmath>
#include <QDebug>

Tracker::Tracker()
: QObject(nullptr)
, m_p(nullptr)
, m_vehicle(osmscout::Vehicle::vehicleCar)
, m_vehicleState(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_vehicleCoord(0, 0)
, m_vehicleBearing(new osmscout::Bearing())
, m_currentSpeed(0)
, m_distance(0)
, m_duration(0)
//, m_nextRouteStep()
//, m_remainingDistance()
{
  QThread* thread = new QThread();
  thread->setObjectName("tracker");
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  m_p = new TrackerModule(thread);
  m_p->moveToThread(thread);
  thread->start();

  connect(this, &Tracker::locationUpdate, m_p, &TrackerModule::onLocationChanged, Qt::QueuedConnection);
  connect(this, &Tracker::azimuthUpdate, m_p, &TrackerModule::onAzimuthChanged, Qt::QueuedConnection);
  connect(this, &Tracker::needReset, m_p, &TrackerModule::onReset, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::positionChanged, this, &Tracker::onPositionChanged, Qt::QueuedConnection);
  connect(m_p, &TrackerModule::dataChanged, this, &Tracker::onDataChanged, Qt::QueuedConnection);
}

Tracker::~Tracker()
{
}

osmscout::VehiclePosition* Tracker::getTrackerPosition() const
{
  std::shared_ptr<osmscout::GeoCoord> nextPosition(nullptr);
  //if (!m_nextRouteStep.getType().isEmpty())
  //  nextPosition = std::make_shared<osmscout::GeoCoord>(m_nextRouteStep.GetCoord());
  return new osmscout::VehiclePosition(m_vehicle, m_vehicleState, m_vehicleCoord, m_vehicleBearing, nextPosition);
}

void Tracker::locationChanged(bool positionValid, double lat, double lon, bool horizontalAccuracyValid, double horizontalAccuracy)
{
  Q_UNUSED(horizontalAccuracyValid)
  Q_UNUSED(horizontalAccuracy)
  emit locationUpdate(positionValid, lat, lon);
}

void Tracker::azimuthChanged(double azimuth)
{
  emit azimuthUpdate(azimuth);
}

void Tracker::reset()
{
  emit needReset();
}

void Tracker::onPositionChanged(const osmscout::PositionAgent::PositionState state,
                       const osmscout::GeoCoord coord,
                       const std::shared_ptr<osmscout::Bearing> bearing)
{
  m_vehicleState = state;
  m_vehicleCoord = coord;
  m_vehicleBearing = bearing;
  emit trackerPositionChanged();
}

void Tracker::onDataChanged(double kmph, double meters, double seconds)
{
  m_currentSpeed = kmph;
  m_distance = meters;
  m_duration = seconds;
  emit trackerDataChanged();
}

TrackerModule::TrackerModule(QThread* thread)
: m_t(thread)
, m_timer()
, m_state(osmscout::PositionAgent::PositionState::NoGpsSignal)
, m_azimuth(0)
, m_currentSpeed(0)
, m_bearing()
, m_lastPosition()
, m_distance(0)
, m_duration(0)
{
  m_timer.moveToThread(thread);
  connect(&m_timer, &QTimer::timeout, this, &TrackerModule::onTimeout);
}

TrackerModule::~TrackerModule()
{
  if (m_t)
    m_t->quit();
}

void TrackerModule::onTimeout()
{
}

void TrackerModule::onLocationChanged(bool positionValid, double lat, double lon)
{
  if (!positionValid)
    m_state = osmscout::PositionAgent::PositionState::NoGpsSignal;
  else
    m_state = osmscout::PositionAgent::PositionState::NoRoute;

  osmscout::Timestamp now = std::chrono::system_clock::now();

  if (m_lastPosition)
  {
    double d = osmin::Utils::sphericalDistance(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon);
    auto sec = std::chrono::duration_cast<std::chrono::duration<double> >(now - m_lastPosition.time);
    if (sec.count() > 0)
    {
      m_duration += sec.count();
      m_currentSpeed = 3.6 * d / sec.count();
      if (m_currentSpeed > 0.5)
        m_distance += d;
      emit dataChanged(m_currentSpeed, m_distance, m_duration);
    }
    if (m_currentSpeed < 3.0)
      m_bearing = osmscout::Bearing::Degrees(m_azimuth);
    else
      m_bearing = osmscout::Bearing::Radians(osmin::Utils::sphericalBearingFinal(m_lastPosition.coord.GetLat(), m_lastPosition.coord.GetLon(), lat, lon));
  }
  else
  {
    m_bearing = osmscout::Bearing::Degrees(m_azimuth);
  }

  m_lastPosition.coord.Set(lat, lon);
  m_lastPosition.time = now;
  emit positionChanged(m_state, m_lastPosition.coord, std::make_shared<osmscout::Bearing>(m_bearing));
}

void TrackerModule::onAzimuthChanged(double degrees)
{
  m_azimuth = degrees;
  if (m_currentSpeed < 3.0)
  {
    m_bearing = osmscout::Bearing::Degrees(m_azimuth);
    emit positionChanged(m_state, m_lastPosition.coord, std::make_shared<osmscout::Bearing>(m_bearing));
  }
}

void TrackerModule::onReset()
{
  m_distance = 0;
  m_duration = 0;
}
