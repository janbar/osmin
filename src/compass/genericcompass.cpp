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

#include "genericcompass.h"
#include <QDebug>

#include <time.h>
#include <errno.h>

#include <qmath.h>

#define RADIANS_TO_DEGREES 57.2957795

char const * const GenericCompass::id("builtin.compass");

quint64 produceTimestamp()
{
    struct timespec tv;
    int ok;

#ifdef CLOCK_MONOTONIC_RAW
    ok = clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
    if (ok != 0)
#endif
    ok = clock_gettime(CLOCK_MONOTONIC, &tv);
    Q_ASSERT(ok == 0);

    quint64 result = (tv.tv_sec * 1000000ULL) + (tv.tv_nsec * 0.001); // scale to microseconds
    return result;
}

GenericCompass::GenericCompass(QSensor *sensor)
    : QSensorBackend(sensor),
      _initState(true)
{
    _gyroMatrix = new float[9];
    // initialize gyroMatrix with identity matrix
    _gyroMatrix[0] = 1.0f; _gyroMatrix[1] = 0.0f; _gyroMatrix[2] = 0.0f;
    _gyroMatrix[3] = 0.0f; _gyroMatrix[4] = 1.0f; _gyroMatrix[5] = 0.0f;
    _gyroMatrix[6] = 0.0f; _gyroMatrix[7] = 0.0f; _gyroMatrix[8] = 1.0f;
    _fusedOrientation[2] = _fusedOrientation[1] = _fusedOrientation[0] = 0;
    _orientation[2] = _orientation[1] = _orientation[0] = 0;
    _gyroOrientation[2] = _gyroOrientation[1] = _gyroOrientation[0] = 0;

    _gravitySensor = new QAccelerometer(this);
    _magnetmeter = new QMagnetometer(this);
    _magnetmeter->setReturnGeoValues(true);
    _gyroscope = new QGyroscope(this);

    _gravitySensor->connectToBackend();
    _magnetmeter->connectToBackend();
    //_gyroscope->connectToBackend();
    _gyroscopeEnabled = false;

    setReading<QCompassReading>(&_compassReading);
    setDataRates(_gravitySensor);

    connect(_gravitySensor, SIGNAL(readingChanged()), this, SLOT(onAccelerometerChanged()));
    connect(_gravitySensor, SIGNAL(sensorError(int)), this, SIGNAL(sensorError(int)));
    connect(_magnetmeter, SIGNAL(readingChanged()), this, SLOT(onMagnetometerChanged()));
    connect(_magnetmeter, SIGNAL(sensorError(int)), this, SIGNAL(sensorError(int)));
    connect(_gyroscope, SIGNAL(readingChanged()), this, SLOT(onGyroscopeChanged()));
    connect(_gyroscope, SIGNAL(sensorError(int)), this, SIGNAL(sensorError(int)));
}

GenericCompass::~GenericCompass()
{
    delete[] (_gyroMatrix);
}

void GenericCompass::onAccelerometerChanged()
{
    QAccelerometerReading *accelerometer = _gravitySensor->reading();
    _gravity[0] = accelerometer->x();
    _gravity[1] = accelerometer->y();
    _gravity[2] = accelerometer->z();
}

void GenericCompass::onMagnetometerChanged()
{
    QMagnetometerReading *magnetometer = _magnetmeter->reading();
    // returned by Qt in Tesla, we need to convert in microtesla:
    _geomagnetic[0] = magnetometer->x() * 1000000;
    _geomagnetic[1] = magnetometer->y() * 1000000;
    _geomagnetic[2] = magnetometer->z() * 1000000;
    _compassReading.setCalibrationLevel(magnetometer->calibrationLevel());
    checkValues();
}

void GenericCompass::onGyroscopeChanged()
{
    gyroFunction(_gyroscope->reading());
}

void GenericCompass::checkValues()
{
    float R[9] = {0}, I[9] = {0};

    if (GenericCompass::getRotationMatrix(R, 9, I, 9, _gravity, _geomagnetic)) {
        GenericCompass::getOrientation(R, 9, _orientation);
        if (_gyroscopeEnabled) {
            calculateFusedOrientation();
        } else {
            _fusedOrientation[0] = _orientation[0];
        }
        qreal newAzimuth = _fusedOrientation[0] * RADIANS_TO_DEGREES;
        if (_compassReading.azimuth() != newAzimuth) { // TODO: run thru collection of QCompassFilter
            _compassReading.setAzimuth(newAzimuth);
            _compassReading.setTimestamp(produceTimestamp());
            emit newReadingAvailable();
        }
    }
}

void GenericCompass::start()
{
    _gravitySensor->setDataRate(sensor()->dataRate());
    _gravitySensor->setAlwaysOn(sensor()->isAlwaysOn());

    _magnetmeter->setDataRate(sensor()->dataRate());
    _magnetmeter->setAlwaysOn(sensor()->isAlwaysOn());

    _gyroscope->setDataRate(sensor()->dataRate());
    _gyroscope->setAlwaysOn(sensor()->isAlwaysOn());

    _gravitySensor->start();
    _magnetmeter->start();
    _gyroscope->start();
}

void GenericCompass::stop()
{
    _gravitySensor->stop();
    _magnetmeter->stop();
    _gyroscope->stop();
}

void GenericCompass::gyroFunction(QGyroscopeReading *event)
{
    // don't start until first accelerometer/magnetometer orientation has been acquired
    if (_orientation[0] == 0 && _orientation[1] == 0 && _orientation[2] == 0)
        return;

    float *tempGyroMatrix = NULL;
    // initialization of the gyroscope based rotation matrix
    if (_initState) {
        float *initMatrix = getRotationMatrixFromOrientation(_orientation); // memoria allocata nella heap
        float test[3] = {0};

        tempGyroMatrix = _gyroMatrix;
        GenericCompass::getOrientation(initMatrix, 9, test);
        _gyroMatrix = matrixMultiplication(_gyroMatrix, initMatrix);
        delete[] (tempGyroMatrix);
        tempGyroMatrix = NULL;
        delete[] (initMatrix);
        _initState = false;
    }

    // copy the new gyro values into the gyro array
    // convert the raw gyro data into a rotation vector
    float deltaVector[4] = {0};
    if (_timestamp != 0) {
        float dT = (event->timestamp() - _timestamp) * NS2S;
        _gyro[0] = event->x();
        _gyro[1] = event->y();
        _gyro[2] = event->z();
        getRotationVectorFromGyro(_gyro, deltaVector, dT / 2.0f);
    }
    // measurement done, save current time for next interval
    _timestamp = event->timestamp();

    // convert rotation vector into rotation matrix
    float deltaMatrix[9] = {0};
    GenericCompass::getRotationMatrixFromVector(deltaMatrix, 9, deltaVector, 4);

    // apply the new rotation interval on the gyroscope based rotation matrix
    tempGyroMatrix = _gyroMatrix;
    _gyroMatrix = matrixMultiplication(_gyroMatrix, deltaMatrix); // alloca memoria nella heap
    delete[] (tempGyroMatrix);

    // get the gyroscope based orientation from the rotation matrix
    GenericCompass::getOrientation(_gyroMatrix, 9, _gyroOrientation);
}


void GenericCompass::calculateFusedOrientation()
{
    static float oneMinusCoeff = 1.0f - FILTER_COEFFICIENT;
    /*
     * Fix for 179° <--> -179° transition problem:
     * Check whether one of the two orientation angles (gyro or accMag) is negative while the other one is positive.
     * If so, add 360° (2 * PI) to the negative value, perform the sensor fusion, and remove the 360° from the result
     * if it is greater than 180°. This stabilizes the output in positive-to-negative-transition cases.
    */
    // azimuth
    if (_gyroOrientation[0] < -0.5 * PI && _orientation[0] > 0.0) {
        _fusedOrientation[0] = (float) (FILTER_COEFFICIENT * (_gyroOrientation[0] + 2.0 * PI)
                + oneMinusCoeff * _orientation[0]);
        _fusedOrientation[0] -= (_fusedOrientation[0] > PI) ? 2.0 * PI : 0;
    }
    else if (_orientation[0] < -0.5 * PI && _gyroOrientation[0] > 0.0) {
        _fusedOrientation[0] = (float) (FILTER_COEFFICIENT * _gyroOrientation[0] + oneMinusCoeff
                * (_orientation[0] + 2.0 * PI));
        _fusedOrientation[0] -= (_fusedOrientation[0] > PI)? 2.0 * PI : 0;
    }
    else {
        _fusedOrientation[0] = FILTER_COEFFICIENT * _gyroOrientation[0] + oneMinusCoeff * _orientation[0];
    }

    // overwrite gyro matrix and orientation with fused orientation
    // to comensate gyro drift
    float *tempGyroMatrix = _gyroMatrix;
    _gyroMatrix = getRotationMatrixFromOrientation(_fusedOrientation); // allocato nella heap
    _gyroOrientation[0] = _fusedOrientation[0];
    _gyroOrientation[1] = _fusedOrientation[1];
    _gyroOrientation[2] = _fusedOrientation[2];
    delete[] (tempGyroMatrix);
}

void GenericCompass::getRotationMatrixFromVector(float *R, size_t lenR, float *rotationVector, size_t lenRotationVector)
{
    float q0;
    float q1 = rotationVector[0];
    float q2 = rotationVector[1];
    float q3 = rotationVector[2];

    if (lenRotationVector >= 4) {
        q0 = rotationVector[3];
    }
    else {
        q0 = 1 - q1*q1 - q2*q2 - q3*q3;
        q0 = (q0 > 0) ? (float)qSqrt(q0) : 0;
    }
    float sq_q1 = 2 * q1 * q1;
    float sq_q2 = 2 * q2 * q2;
    float sq_q3 = 2 * q3 * q3;
    float q1_q2 = 2 * q1 * q2;
    float q3_q0 = 2 * q3 * q0;
    float q1_q3 = 2 * q1 * q3;
    float q2_q0 = 2 * q2 * q0;
    float q2_q3 = 2 * q2 * q3;
    float q1_q0 = 2 * q1 * q0;

    if (lenR == 9) {
        R[0] = 1 - sq_q2 - sq_q3;
        R[1] = q1_q2 - q3_q0;
        R[2] = q1_q3 + q2_q0;
        R[3] = q1_q2 + q3_q0;
        R[4] = 1 - sq_q1 - sq_q3;
        R[5] = q2_q3 - q1_q0;
        R[6] = q1_q3 - q2_q0;
        R[7] = q2_q3 + q1_q0;
        R[8] = 1 - sq_q1 - sq_q2;
    }
    else if (lenR == 16) {
        R[0] = 1 - sq_q2 - sq_q3;
        R[1] = q1_q2 - q3_q0;
        R[2] = q1_q3 + q2_q0;
        R[3] = 0.0f;
        R[4] = q1_q2 + q3_q0;
        R[5] = 1 - sq_q1 - sq_q3;
        R[6] = q2_q3 - q1_q0;
        R[7] = 0.0f;
        R[8] = q1_q3 - q2_q0;
        R[9] = q2_q3 + q1_q0;
        R[10] = 1 - sq_q1 - sq_q2;
        R[11] = 0.0f;
        R[12] = R[13] = R[14] = 0.0f;
        R[15] = 1.0f;
    }
}
// ritorna oggetto nella heap da eliminare
float * GenericCompass::getRotationMatrixFromOrientation(float *o)
{
    float xM[9] = {0};
    float yM[9] = {0};
    float zM[9] = {0};
    float sinX = (float)qSin(o[1]);
    float cosX = (float)qCos(o[1]);
    float sinY = (float)qSin(o[2]);
    float cosY = (float)qCos(o[2]);
    float sinZ = (float)qSin(o[0]);
    float cosZ = (float)qCos(o[0]);

    // rotation about x-axis (pitch)
    xM[0] = 1.0f; xM[1] = 0.0f; xM[2] = 0.0f;
    xM[3] = 0.0f; xM[4] = cosX; xM[5] = sinX;
    xM[6] = 0.0f; xM[7] = -sinX; xM[8] = cosX;

    // rotation about y-axis (roll)
    yM[0] = cosY; yM[1] = 0.0f; yM[2] = sinY;
    yM[3] = 0.0f; yM[4] = 1.0f; yM[5] = 0.0f;
    yM[6] = -sinY; yM[7] = 0.0f; yM[8] = cosY;

    // rotation about z-axis (azimuth)
    zM[0] = cosZ; zM[1] = sinZ; zM[2] = 0.0f;
    zM[3] = -sinZ; zM[4] = cosZ; zM[5] = 0.0f;
    zM[6] = 0.0f; zM[7] = 0.0f; zM[8] = 1.0f;

    // rotation order is y, x, z (roll, pitch, azimuth)
    float *resultMatrix = NULL;
    float *tempMatrix = matrixMultiplication(xM, yM);

    resultMatrix = matrixMultiplication(zM, tempMatrix);
    delete[] (tempMatrix);
    return resultMatrix;
}

// ritorna oggetto nella heap da eliminare
float * GenericCompass::matrixMultiplication(float *A, float *B)
{
    float *result = new float[9];

    result[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
    result[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
    result[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];

    result[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
    result[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
    result[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];

    result[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
    result[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
    result[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];

    return result;
}

bool GenericCompass::getRotationMatrix(float *R, size_t lenR, float *I, size_t lenI, float *gravity, float *geomagnetic)
{
    float Ax = gravity[0];
    float Ay = gravity[1];
    float Az = gravity[2];
    float Ex = geomagnetic[0];
    float Ey = geomagnetic[1];
    float Ez = geomagnetic[2];
    float Hx = Ey * Az - Ez * Ay;
    float Hy = Ez * Ax - Ex * Az;
    float Hz = Ex * Ay - Ey * Ax;
    float normH = (float)qSqrt(Hx*Hx + Hy*Hy + Hz*Hz);

    if (normH < 0.1f) {
        // device is close to free fall (or in space?), or close to
        // magnetic north pole. Typical values are  > 100.
        return false;
    }
    float invH = 1.0f / normH;
    Hx *= invH;
    Hy *= invH;
    Hz *= invH;
    float invA = 1.0f / (float)qSqrt(Ax*Ax + Ay*Ay + Az*Az);
    Ax *= invA;
    Ay *= invA;
    Az *= invA;
    float Mx = Ay * Hz - Az * Hy;
    float My = Az * Hx - Ax * Hz;
    float Mz = Ax * Hy - Ay * Hx;
    if (R != NULL) {
        if (lenR == 9) {
            R[0] = Hx; R[1] = Hy; R[2] = Hz;
            R[3] = Mx; R[4] = My; R[5] = Mz;
            R[6] = Ax; R[7] = Ay; R[8] = Az;
        }
        else if (lenR == 16) {
            R[0]  = Hx; R[1]  = Hy; R[2]  = Hz; R[3]  = 0;
            R[4]  = Mx; R[5]  = My; R[6]  = Mz; R[7]  = 0;
            R[8]  = Ax; R[9]  = Ay; R[10] = Az; R[11] = 0;
            R[12] = 0; R[13] = 0; R[14] = 0; R[15] = 1;
        }
    }
    if (I != NULL) {
        // compute the inclination matrix by projecting the geomagnetic
        // vector onto the Z (gravity) and X (horizontal component
        // of geomagnetic vector) axes.
        float invE = 1.0f / (float)qSqrt(Ex * Ex + Ey * Ey + Ez * Ez);
        float c = (Ex * Mx + Ey * My + Ez * Mz) * invE;
        float s = (Ex * Ax + Ey * Ay + Ez * Az) * invE;
        if (lenI == 9) {
            I[0] = 1; I[1] = 0; I[2] = 0;
            I[3] = 0; I[4] = c; I[5] = s;
            I[6] = 0; I[7] =-s; I[8] = c;
        }
        else if (lenI == 16) {
            I[0] = 1; I[1] = 0; I[2] = 0;
            I[4] = 0; I[5] = c; I[6] = s;
            I[8] = 0; I[9] =-s; I[10]= c;
            I[3] = I[7] = I[11] = I[12] = I[13] = I[14] = 0;
            I[15] = 1;
        }
    }
    return true;
}

float * GenericCompass::getOrientation(float *R, size_t lenR, float *values)
{
    /*
     * 4x4 (length=16) case:
     *   /  R[ 0]   R[ 1]   R[ 2]   0  \
     *   |  R[ 4]   R[ 5]   R[ 6]   0  |
     *   |  R[ 8]   R[ 9]   R[10]   0  |
     *   \      0       0       0   1  /
     *
     * 3x3 (length=9) case:
     *   /  R[ 0]   R[ 1]   R[ 2]  \
     *   |  R[ 3]   R[ 4]   R[ 5]  |
     *   \  R[ 6]   R[ 7]   R[ 8]  /
     *
     */
    if (lenR == 9) {
        values[0] = (float)qAtan2(R[1], R[4]);
        values[1] = (float)qAsin(-R[7]);
        values[2] = (float)qAtan2(-R[6], R[8]);
    }
    else {
        values[0] = (float)qAtan2(R[1], R[5]);
        values[1] = (float)qAsin(-R[9]);
        values[2] = (float)qAtan2(-R[8], R[10]);
    }
    return values;
}

void GenericCompass::getRotationVectorFromGyro(float *gyroValues, float *deltaRotationVector, float timeFactor)
{
    float normValues[3] = {0};

    // Calculate the angular speed of the sample
    float omegaMagnitude = (float)qSqrt(gyroValues[0] * gyroValues[0]
            + gyroValues[1] * gyroValues[1]
            + gyroValues[2] * gyroValues[2]);

    // Normalize the rotation vector if it's big enough to get the axis
    if (omegaMagnitude > EPSILON) {
        normValues[0] = gyroValues[0] / omegaMagnitude;
        normValues[1] = gyroValues[1] / omegaMagnitude;
        normValues[2] = gyroValues[2] / omegaMagnitude;
    }
    // Integrate around this axis with the angular speed by the timestep
    // in order to get a delta rotation from this sample over the timestep
    // We will convert this axis-angle representation of the delta rotation
    // into a quaternion before turning it into the rotation matrix.
    float thetaOverTwo = omegaMagnitude * timeFactor;
    float sinThetaOverTwo = (float)qSin(thetaOverTwo);
    float cosThetaOverTwo = (float)qCos(thetaOverTwo);

    deltaRotationVector[0] = sinThetaOverTwo * normValues[0];
    deltaRotationVector[1] = sinThetaOverTwo * normValues[1];
    deltaRotationVector[2] = sinThetaOverTwo * normValues[2];
    deltaRotationVector[3] = cosThetaOverTwo;
}
