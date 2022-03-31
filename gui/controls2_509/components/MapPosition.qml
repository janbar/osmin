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

import QtQuick 2.2
//import QtPositioning 5.2
import Osmin 1.0 as Osmin

Osmin.PositionSource {
    id: positionSource
    property double _lat: 0.0
    property double _lon: 0.0
    property double _alt: 0.0
    property real _acc: 0.0
    property bool _posValid: false
    property bool _accValid: false

    signal dataUpdated(bool valid, double lat, double lon, bool accValid, real acc, double alt)

    onPositionChanged: {
        _lat = position.latitude;
        _lon = position.longitude;
        _alt = position.altitude;
        _acc = position.horizontalAccuracy;
        _posValid = position.valid;
        _accValid = position.horizontalAccuracyValid;
        dataUpdated(_posValid, _lat, _lon, _accValid, _acc, _alt);
    }

    active: true
    updateInterval: 1000
    //preferredPositioningMethods: PositionSource.SatellitePositioningMethods

    Component.onCompleted: { connectToService(Osmin.Service.getServiceHandle()) }
}
