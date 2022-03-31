#ifndef REMOTECOMPASS_H
#define REMOTECOMPASS_H

#include "remote.h"

#include <QObject>

class RemoteCompassReading : public QObject
{
  Q_OBJECT
  Q_PROPERTY(float azimuth READ getAzimuth CONSTANT)
  Q_PROPERTY(float calibrationLevel READ getCalibrationLevel CONSTANT)

public:
  explicit RemoteCompassReading(QObject * parent = nullptr) : QObject(parent) { }

  float getAzimuth() { return m_azimuth; }
  float getCalibrationLevel() { return m_calibrationLevel; }
  void set(float azimuth, float calibration)
  {
    m_azimuth = azimuth;
    m_calibrationLevel = calibration;
  }
private:
  float m_azimuth = 0.0f;
  float m_calibrationLevel = 0.0f;
};

class RemoteCompass : public QObject, public Remote
{
  Q_OBJECT
  Q_PROPERTY(RemoteCompassReading* reading READ getReading NOTIFY readingChanged)
  Q_PROPERTY(bool active READ getActive WRITE setActive NOTIFY activeChanged)
  Q_PROPERTY(int dataRate READ getDataRate WRITE setDataRate NOTIFY dataRateChanged)

public:
  explicit RemoteCompass(QObject * parent = nullptr) : QObject(parent), m_reading(this) { }

  RemoteCompassReading* getReading() { return &m_reading; }
  bool getActive() { return m_active; }
  int getDataRate() { return m_dataRate; }
  void setActive(bool active);
  void setDataRate(int datarate) { (void)datarate; /*disabled*/ }

  // for testing
  void set(float azimuth, float calibration)
  {
    _readingChanged(azimuth, calibration);
  }

  Q_INVOKABLE void connectToService(QVariant service) override;
  void connectToService(ServiceFrontendPtr& service) override;

signals:
  // operations
  // callbacks
  void readingChanged();
  void activeChanged();
  void dataRateChanged();

private slots:
  void _readingChanged(float azimuth, float calibration);
  void _dataRateChanged(int datarate);

private:
  ServiceFrontendPtr m_service;
  RemoteCompassReading m_reading;
  bool m_active = false;
  int m_dataRate = 0;
};

#endif // REMOTECOMPASS_H
