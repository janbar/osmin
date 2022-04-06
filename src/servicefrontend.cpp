#include "servicefrontend.h"
#include "rep_servicemessenger_replica.h"

#include <QTimer>
#include <QElapsedTimer>

ServiceFrontend::ServiceFrontend(const QString& url)
: m_url(url)
, m_t(new QThread())
{
  m_t->setObjectName("serviceremote");
  this->moveToThread(m_t);
  connect(m_t, &QThread::finished, this, &ServiceFrontend::onFinished);
  connect(m_t, &QThread::started, this, &ServiceFrontend::run);
  m_t->start();
}

ServiceFrontend::~ServiceFrontend()
{
  if (!m_t->isFinished())
    terminate();
  m_t->deleteLater();
  qInfo("%s", __FUNCTION__);
}

void ServiceFrontend::terminate()
{
  m_t->quit();
}

void ServiceFrontend::ping(const QString &message)
{
  auto m = m_messenger;
  if (!m.isNull())
    m->ping(message);
}

void ServiceFrontend::setRecording(const QString &filename)
{
  auto m = m_messenger;
  if (!m.isNull())
    m->tracker_setRecording(filename);
}

void ServiceFrontend::resetTrackingData()
{
  auto m = m_messenger;
  if (!m.isNull())
    m->tracker_resetData();
}

void ServiceFrontend::startRecording()
{
  auto m = m_messenger;
  if (!m.isNull())
    m->tracker_startRecording();
}

void ServiceFrontend::stopRecording()
{
  auto m = m_messenger;
  if (!m.isNull())
    m->tracker_stopRecording();
}

void ServiceFrontend::pinPosition()
{
  auto m = m_messenger;
  if (!m.isNull())
    m->tracker_pinPosition();
}

void ServiceFrontend::markPosition(const QString &symbol, const QString &name, const QString &description)
{
  auto m = m_messenger;
  if (!m.isNull())
    m->tracker_markPosition(symbol, name, description);
}

void ServiceFrontend::setCompassDataRate(int datarate)
{
  auto m = m_messenger;
  if (!m.isNull())
    m->compass_setDataRate(datarate);
}

void ServiceFrontend::setPositionUpdateInterval(int interval)
{
  auto m = m_messenger;
  if (!m.isNull())
    m->position_setUpdateInterval(interval);
}

void ServiceFrontend::setPreferedPositioningMethods(int methods)
{
  auto m = m_messenger;
  if (!m.isNull())
    m->position_setPreferredPositioningMethods(methods);
}

void ServiceFrontend::run()
{
  m_messenger.reset();
  if (m_node)
    delete m_node;
  m_node = new QRemoteObjectNode();
  m_node->connectToNode(QUrl(m_url));
  m_messenger.reset(m_node->acquire<ServiceMessengerReplica>());
  bool res = false;
  qDebug("Trying to connect to service ...");
  QElapsedTimer tm;
  while (!res)
  {
    tm.restart();
    res = m_messenger->waitForSource(30000);
    if (!res)
    {
      // interrupted ?
      if (tm.elapsed() < 5000)
        return;
      qDebug("Connect to service source failed after timeout");
    }
  }
  qDebug("Connect to service succeeded");
  // setup watch dog
  connect(m_messenger.data(), &ServiceMessengerReplica::stateChanged, this, &ServiceFrontend::onStateChanged, Qt::QueuedConnection);

  connect(m_messenger.data(), &ServiceMessengerReplica::pong, [](const QString& name){
      qWarning() << "Service reply: " << name;
  });

  connect(m_messenger.data(), &ServiceMessengerReplica::compass_readingChanged, this, &ServiceFrontend::compassReadingChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::compass_activeChanged, this, &ServiceFrontend::compassActiveChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::compass_dataRateChanged, this, &ServiceFrontend::compassDataRateChanged);

  connect(m_messenger.data(), &ServiceMessengerReplica::position_positionUpdated, this, &ServiceFrontend::positionPositionUpdated);
  connect(m_messenger.data(), &ServiceMessengerReplica::position_activeChanged, this, &ServiceFrontend::positionActiveChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::position_updateIntervalChanged, this, &ServiceFrontend::positionUpdateIntervalChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::position_preferredPositioningMethodsChanged, this, &ServiceFrontend::positionPreferredPositioningMethodsChanged);

  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_recordingFailed, this, &ServiceFrontend::trackerRecordingFailed);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_resumeRecording, this, &ServiceFrontend::trackerResumeRecording);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_isRecordingChanged, this, &ServiceFrontend::trackerIsRecordingChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_recordingChanged, this, &ServiceFrontend::trackerRecordingChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_processingChanged, this, &ServiceFrontend::trackerProcessingChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_positionRecorded, this, &ServiceFrontend::trackerPositionRecorded);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_positionMarked, this, &ServiceFrontend::trackerPositionMarked);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_positionChanged, this, &ServiceFrontend::trackerPositionChanged);
  connect(m_messenger.data(), &ServiceMessengerReplica::tracker_dataChanged, this, &ServiceFrontend::trackerDataChanged);

  emit serviceConnected();
}

void ServiceFrontend::onStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState)
{
  (void)oldState;
  if (state != QRemoteObjectReplica::Valid)
  {
    emit serviceDisconnected();
    this->run();
  }
}

void ServiceFrontend::onFinished()
{
  m_messenger.reset();
  if (m_node)
    delete m_node;
  m_node = nullptr;
}
