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
import QtQuick.Controls 2.2
import QtQml.Models 2.3
import Osmin 1.0
import "./components"
import "../toolbox.js" as ToolBox

PopOver {
    id: routingDialog
    title: ""
    property MapPosition position
    property Navigator navigator
    property string vehicle: "car"
    property real maximumHeight: parent.height

    states: [
        State {
            name: "dialog"
            PropertyChanges { target: routingDialog; title: qsTr("Navigation"); height: maximumHeight; }
            PropertyChanges { target: dialog; visible: true; }
            PropertyChanges { target: way; visible: false; }
        },
        State {
            name: "pickPlace"
            PropertyChanges { target: routingDialog; title: qsTr("Pick a place on the Map"); height: minimumHeight; }
        },
        State {
            name: "navigate"
            PropertyChanges { target: routingDialog; title: qsTr("Navigate"); height: maximumHeight; }
            PropertyChanges { target: dialog; visible: false; }
            PropertyChanges { target: way; visible: true; }
        },
        State {
            name: "navigation"
            PropertyChanges { target: routingDialog; title: qsTr("Navigation"); height: maximumHeight; }
            PropertyChanges { target: dialog; visible: false; }
            PropertyChanges { target: way; visible: true; }
        }
    ]
    state: "dialog"

    signal placePicked(bool valid, double lat, double lon)

    property QtObject placeFrom: QtObject {
        property bool valid: false
        property string address: qsTr("Select a position")
        property double lat: 0.0
        property double lon: 0.0
        property LocationEntry location: null
    }

    property QtObject placeTo: QtObject {
        property bool valid: false
        property string address: qsTr("Select a destination")
        property double lat: 0.0
        property double lon: 0.0
        property LocationEntry location: null
    }

    LocationInfoModel{
        id: locationInfoModel
    }

    function locationDescription(index) {
        var str = "";
        if (locationInfoModel.rowCount() > index) {
            var mi = locationInfoModel.index(index, 0);
            var d = locationInfoModel.data(mi, LocationInfoModel.DistanceRole);
            if (d < 100.0) {
                str = locationInfoModel.data(mi, LocationInfoModel.AddressRole);
                if (str === "")
                    str = locationInfoModel.data(mi, LocationInfoModel.PoiRole);
                if (str !== "")
                    str += " - ";
                str += locationInfoModel.data(mi, LocationInfoModel.RegionRole);
            }
        }
        return str;
    }

    contents: Column {
        id: routingBody

        Column {
            id: dialog
            width: parent.width
            spacing: units.gu(1)

            // vehicle

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: units.gu(3)

                MapIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/images/trip/walk.svg"
                    height: units.gu(7)
                    width: height
                    color: vehicle === "foot" ? styleMap.popover.highlightedColor : styleMap.popover.foregroundColor
                    onClicked: vehicle = "foot"
                }
                MapIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/images/trip/bike.svg"
                    height: units.gu(7)
                    width: height
                    color: vehicle === "bicycle" ? styleMap.popover.highlightedColor : styleMap.popover.foregroundColor
                    onClicked: vehicle = "bicycle"
                }
                MapIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/images/trip/car.svg"
                    height: units.gu(7)
                    width: height
                    color: vehicle === "car" ? styleMap.popover.highlightedColor : styleMap.popover.foregroundColor
                    onClicked: vehicle = "car"
                }
            }

            // Route from

            Label {
                text: qsTr("From")
                color: styleMap.popover.highlightedColor
                font.pointSize: units.fs("medium")
            }
            ComboBox {
                id: from
                width: parent.width
                height: units.gu(4)
                flat: true
                background: Rectangle {
                    color: styleMap.view.backgroundColor
                    anchors.fill: parent
                }
                textRole: "text"
                Component.onCompleted: {
                    currentIndex = 0;
                    // by default set the current position
                    if (!placeFrom.valid && position._posValid) {
                        selectPosition(position._lat, position._lon);
                    }
                }
                model: [
                    { text: placeFrom.address },
                    { text: qsTr("My position") },
                    { text: qsTr("Search Place") },
                    { text: qsTr("Select on Map") },
                    { text: qsTr("Favorite") }
                ]

                onActivated: {
                    var page = null;
                    if (currentIndex === 1 && position._posValid) {
                        selectPosition(position._lat, position._lon);
                    } else if (currentIndex === 2) {
                        page = stackView.push("qrc:/controls2/SearchPlace.qml", {
                                           "searchCenterLat": position._lat,
                                           "searchCenterLon": position._lon,
                                           "acceptLabel": qsTr("Accept")
                                       });
                        ToolBox.connectOnce(page.selectLocation, selectLocation);
                    } else if (currentIndex === 3) {
                        ToolBox.connectOnce(routingDialog.placePicked, picked);
                        routingDialog.state = "pickPlace";
                    } else if (currentIndex === 4) {
                        page = stackView.push("qrc:/controls2/Favorites.qml", { "state": "selection" });
                        ToolBox.connectOnce(page.selectPOI, selectPOI);
                    }
                    if (currentIndex !== 0)
                        currentIndex = 0;
                }
                function selectPosition(lat, lon) {
                        placeFrom.valid = true;
                        placeFrom.lat = lat;
                        placeFrom.lon = lon;
                        ToolBox.connectWhileFalse(locationInfoModel.readyChange, infoReadyChange);
                        locationInfoModel.setLocation(placeFrom.lat, placeFrom.lon);
                }
                function picked(valid, lat, lon) {
                    routingDialog.state = "dialog";
                    if (valid) {
                        placeFrom.valid = true;
                        placeFrom.lat = lat;
                        placeFrom.lon = lon;
                        ToolBox.connectWhileFalse(locationInfoModel.readyChange, infoReadyChange);
                        locationInfoModel.setLocation(lat, lon);
                    }
                }
                function infoReadyChange(ready) {
                    if (ready) {
                        var str = locationDescription(0);
                        placeFrom.address = str.length > 0 ? str : Converter.readableCoordinatesGeocaching(placeFrom.lat, placeFrom.lon);
                    }
                }
                function selectLocation(location, lat, lon, label) {
                    if (lat !== NaN && lon !== NaN && label !== "") {
                        placeFrom.valid = true;
                        placeFrom.lat = lat;
                        placeFrom.lon = lon;
                        placeFrom.address = label;
                    }
                }
                function selectPOI(poi) {
                    if (poi)
                        selectLocation(null, poi.lat, poi.lon, poi.label);
                }
            }
            Label {
                id: fromCoordinates
                width: parent.width
                text: placeFrom.valid ? Converter.readableCoordinatesGeocaching(placeFrom.lat, placeFrom.lon) : ""
                color: styleMap.popover.highlightedColor
                font.pointSize: units.fs("small")
            }

            // Separator
            Item {
                width: parent.width
                height: units.gu(1)
            }

            // Route to

            Label {
                text: qsTr("Destination")
                color: styleMap.popover.highlightedColor
                font.pointSize: units.fs("medium")
            }
            ComboBox {
                id: to
                width: parent.width
                height: units.gu(4)
                flat: true
                background: Rectangle {
                    color: styleMap.view.backgroundColor
                    anchors.fill: parent
                }
                textRole: "text"
                Component.onCompleted: currentIndex = 0
                model: [
                    { text: placeTo.address },
                    { text: qsTr("Search Place") },
                    { text: qsTr("Select on Map") },
                    { text: qsTr("Favorite") }
                ]

                onActivated: {
                    var page = null;
                    if (currentIndex === 1) {
                        page = stackView.push("qrc:/controls2/SearchPlace.qml", {
                                       "searchCenterLat": position._lat,
                                       "searchCenterLon": position._lon,
                                       "acceptLabel": qsTr("Accept")
                                   });
                        ToolBox.connectOnce(page.selectLocation, selectLocation);
                    } else if (currentIndex === 2) {
                        ToolBox.connectOnce(routingDialog.placePicked, picked);
                        routingDialog.state = "pickPlace";
                    } else if (currentIndex === 3) {
                        page = stackView.push("qrc:/controls2/Favorites.qml", { "state": "selection" });
                        ToolBox.connectOnce(page.selectPOI, selectPOI);
                    }
                    if (currentIndex !== 0)
                        currentIndex = 0;
                }
                function picked(valid, lat, lon) {
                    routingDialog.state = "dialog";
                    if (valid) {
                        placeTo.valid = true;
                        placeTo.lat = lat;
                        placeTo.lon = lon;
                        ToolBox.connectWhileFalse(locationInfoModel.readyChange, infoReadyChange);
                        locationInfoModel.setLocation(lat, lon);
                    }
                }
                function infoReadyChange(ready) {
                    if (ready) {
                        var str = locationDescription(0);
                        placeTo.address = str.length > 0 ? str : Converter.readableCoordinatesGeocaching(placeTo.lat, placeTo.lon);
                    }
                }
                function selectLocation(location, lat, lon, label) {
                    if (lat !== NaN && lon !== NaN && label !== "") {
                        placeTo.valid = true;
                        placeTo.lat = lat;
                        placeTo.lon = lon;
                        placeTo.address = label;
                    }
                }
                function selectPOI(poi) {
                    if (poi)
                        selectLocation(null, poi.lat, poi.lon, poi.label);
                }
            }
            Label {
                id: toCoordinates
                width: parent.width
                text: placeTo.valid ? Converter.readableCoordinatesGeocaching(placeTo.lat, placeTo.lon) : ""
                color: styleMap.popover.highlightedColor
                font.pointSize: units.fs("small")
            }

            ProgressBar {
                height: units.gu(2)
                width: parent.width
                from: 0
                to: 100
                opacity: computeRunning && routeProgress > 0 ? 1.0 : 0.0
                value: routeProgress
            }

            MapIcon {
                id: routeButton
                source: "qrc:/images/trip/route.svg"
                height: units.gu(5)
                width: parent.width
                color: styleMap.popover.foregroundColor
                onClicked: {
                    if (computeRunning) {
                        breakCompute();
                    } else {
                        computeRoute();
                    }
                }
                label.text: computeRunning ? qsTr("Cancel") : qsTr("Compute route")
                enabled: (!computeRunning && placeFrom.valid && placeTo.valid) || computeRunning
                opacity: enabled ? 1.0 : 0.5
            }
            Label {
                width: parent.width
                color: styleMap.popover.foregroundColor
                font.pointSize: units.fs("small")
                text: routeMessage
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                visible: text !== ""
            }
        }

        Column {
            id: way
            width: parent.width
            spacing: units.gu(1)

            Row {
                width: parent.width
                spacing: units.gu(2)
                MapIcon {
                    id: navigateButton
                    source: "qrc:/images/trip/navigation.svg"
                    height: units.gu(5)
                    width: parent.width / 2 - units.gu(1)
                    color: styleMap.popover.foregroundColor
                    onClicked: {
                        mapView.navigation = true;
                        ToolBox.connectOnce(navigator.stopped, onNavigatorStopped);
                        navigator.setup(vehicle, route.route, route.routeWay, placeTo.location);
                        routingDialog.state = "navigation";
                        routingDialog.close();
                    }
                    label.text: qsTr("Navigate")
                    enabled: routingDialog.state === "navigate"
                    opacity: enabled ? 1.0 : 0.5

                    function onNavigatorStopped() {
                        routingDialog.state = "navigate";
                    }
                }
                MapIcon {
                    id: clearButton
                    source: "qrc:/images/delete.svg"
                    height: units.gu(5)
                    width: parent.width / 2 - units.gu(1)
                    color: styleMap.popover.foregroundColor
                    onClicked: {
                        route.cancel();
                        routingDialog.state = "dialog"
                    }
                    label.text: qsTr("Clear")
                }
            }

            ListView {
                id: stepsView
                width: parent.width
                height: contentHeight
                interactive: false
                spacing: units.gu(1)
                model: route

                header: Row {
                    spacing: units.gu(1)
                    width: parent.width
                    Label {
                        text: qsTr("Route length:")
                        color: styleMap.popover.foregroundColor
                        font.pointSize: units.fs("small")
                    }
                    Label {
                        id: distanceLabel
                        text: Converter.readableDistance(route.length)
                        color: styleMap.popover.highlightedColor
                        font.pointSize: units.fs("small")
                    }
                    Label {
                        text: ", " + qsTr("Duration:")
                        color: styleMap.popover.foregroundColor
                        font.pointSize: units.fs("small")
                    }
                    Label {
                        id: durationLabel
                        text: Converter.panelDurationHM(route.duration)
                        color: styleMap.popover.highlightedColor
                        font.pointSize: units.fs("small")
                    }
                }
                delegate: Row {
                    spacing: units.gu(2)
                    width: parent.width
                    height: Math.max(entryDescription.implicitHeight, icon.height)

                    WAYIcon {
                        id: icon
                        anchors.verticalCenter: parent.verticalCenter
                        color: styleMap.popover.foregroundColor
                        stepType: model.type
                        roundaboutExit: model.roundaboutExit
                        roundaboutClockwise: model.roundaboutClockwise
                        width: units.gu(7)
                        height: width
                    }
                    Label {
                        id: entryDescription
                        anchors.verticalCenter: parent.verticalCenter
                        color: styleMap.popover.foregroundColor
                        width: parent.width - icon.width - units.gu(2)
                        text: model.description
                        font.pointSize: units.fs("small")
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }

    property int routeProgress: 0
    property string routeMessage: ""
    property bool computeRunning: false

    RoutingListModel {
        id: route
        onRouteFailed: {
            routeMessage = qsTranslate("message", reason);
            computeRunning = false;
        }
        onRoutingProgress: {
            routeProgress = percent;
        }
        onComputingChanged: {
            routeProgress = 0;
            if (!computeRunning) {
                console.log("Computing aborted");
                route.clear();
            } else {
                var count = route.count;
                if (count > 0) {
                    if (count > settings.maximumRouteStep) {
                        popInfo.open("The number of steps exceeds the limit. Please reduce the length of the route and restart the calculation.");
                        route.clear();
                    } else {
                        routingDialog.state = "navigate";
                    }
                    computeRunning = false;
                }
            }
        }
    }

    function breakCompute() {
        route.cancel();
        computeRunning = false;
    }

    function computeRoute() {
        routeProgress = 0;
        routeMessage = "";
        placeFrom.location = route.locationEntryFromPosition(placeFrom.lat, placeFrom.lon);
        placeTo.location = route.locationEntryFromPosition(placeTo.lat, placeTo.lon);
        if (placeFrom.location && placeTo.location) {
            computeRunning = true;
            route.setStartAndTarget(placeFrom.location, placeTo.location, vehicle);
            settings.lastVehicle = vehicle;
        } else {
            routeMessage = qsTr("Invalid entry");
            route.clear();
        }
    }

    Component.onCompleted: {
        if (settings.lastVehicle !== "")
            vehicle = settings.lastVehicle;
    }

    onClose: {
        // abort pick
        if (state === "pickPlace")
            placePicked(false, 0.0, 0.0);
    }
}
