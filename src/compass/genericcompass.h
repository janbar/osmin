/****************************************************************************
**
** Copyright (C) 2014 Giovanni Romano <giovanni.romano.76@gmail.com>
** Copyright (C) 2014 Lorn Potter <lorn.potter@jollamobile.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSensors module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GENERICCOMPASS_H
#define GENERICCOMPASS_H

#include <QObject>
#include <QCompassReading>
#include <QGyroscopeReading>
#include <QAccelerometer>
#include <QMagnetometer>
#include <QGyroscope>
#include <qsensorbackend.h>

#define FILTER_COEFFICIENT 0.98f
#define EPSILON 0.000000001f
#define NS2S (1.0f / 1000000000.0f)
#define PI 3.1415926



class GenericCompass : public QSensorBackend
{
    Q_OBJECT
public:
    static char const * const id;
    GenericCompass(QSensor *sensor);
    ~GenericCompass();
    void start() Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;

signals:
    void sensorError(int);

private:
    QAccelerometer *_gravitySensor;
    QMagnetometer *_magnetmeter;
    QGyroscope *_gyroscope;
    mutable QCompassReading _compassReading;

    float _gravity[3];
    float _gyro[3];
    float _orientation[3]; //alias: accMagOrientation
    float _geomagnetic[3];
    float *_gyroMatrix;
    float _gyroOrientation[3];
    float _fusedOrientation[3];
    float _timestamp;
    bool _initState;

    void checkValues();
    void calculateFusedOrientation();
    float *matrixMultiplication(float *A, float *B);
    float *getRotationMatrixFromOrientation(float *o);
    void gyroFunction(QGyroscopeReading *event);
    void getRotationVectorFromGyro(float *gyroValues, float *deltaRotationVector, float timeFactor);
    static void getRotationMatrixFromVector(float *R, size_t lenR, float *rotationVector, size_t lenRotationVector);
    static bool getRotationMatrix(float *R, size_t lenR, float *I, size_t lenI, float *gravity, float *geomagnetic);
    static float *getOrientation(float *R, size_t lenR, float *values);

private slots:
    void onAccelerometerChanged();
    void onMagnetometerChanged();
    void onGyroscopeChanged();
};

#endif // GENERICCOMPASS_H
