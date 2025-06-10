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

import QtQuick 2.9
import "./components"
import "../toolbox.js" as ToolBox

MapPage {
    pageTitle: qsTr("Map View")
    showHeader: false // hide the header bar

    Loader {
        id: loader
        active: true
        anchors.fill: parent
        asynchronous: true
        sourceComponent: MapView {
            anchors.fill: parent
        }
    }

    property alias mapView: loader.item

    function addCourse(bid, overlays) {
        if (loader.status === Loader.Ready) {
            if (mapView.overlayManager.addCourse(bid, overlays))
                settings.courseId = bid;
        }
    }

    function removeCourse() {
        if (loader.status === Loader.Ready) {
            if (mapView.overlayManager.removeAllCourses())
                settings.courseId = 0;
        }
    }

    property QtObject suspendedState: QtObject {
        property bool rotateEnabled: false
        property bool lockRotation: false
        property bool navigation: false
    }

    function restoreMapViewState() {
        console.log("Restore mapView state");
        if (suspendedState.rotateEnabled)
            mapView.rotateEnabled = true;
        if (suspendedState.lockRotation)
            mapView.lockRotation = true;
        if (suspendedState.navigation)
            mapView.navigation = true;
    }

    Connections {
        target: mainView
        function onApplicationSuspendedChanged() {
            // Behavior on device mobile
            if (DeviceMobile) {
                if (mainView.applicationSuspended) {
                    // save current state
                    suspendedState.rotateEnabled = mapView.rotateEnabled;
                    suspendedState.lockRotation = mapView.lockRotation;
                    suspendedState.navigation = mapView.navigation;

                    // On Android unload MapView when the Navigator is suspended
                    if (Android && mapView.navigator.suspended) {
                        loader.active = false;
                    } else {
                        mapView.rotateEnabled = false;
                        mapView.map.lockToPosition = false;
                        if (mapView.navigation) {
                            mapView.navigation = false;
                        }
                    }
                } else if (loader.active) {
                    restoreMapViewState();
                } else {
                    ToolBox.connectOnce(loader.loaded, restoreMapViewState);
                    loader.active = true;
                }
            }
        }
    }
}
