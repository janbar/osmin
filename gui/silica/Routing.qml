/*
 * Copyright (C) 2021
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
import Sailfish.Silica 1.0
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
                spacing: units.gu(2)

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

            SectionHeader {
                text: qsTr("From")
            }
            ComboBox {
                id: from
                width: parent.width
                leftMargin: 0
                rightMargin: 0
                menu: ContextMenu {
                    MenuItem { text: placeFrom.address }
                    MenuItem { text: qsTr("My position") }
                    MenuItem { text: qsTr("Search Place") }
                    MenuItem { text: qsTr("Select on Map") }
                    MenuItem { text: qsTr("Favorite") }
                }
                Component.onCompleted: {
                    currentIndex = 0;
                    // by default set the current position
                    if (!placeFrom.valid && position._posValid) {
                        selectPosition(position._lat, position._lon);
                    }
                }
                onCurrentIndexChanged: {
                    var page = null;
                    if (currentIndex === 1 && position._posValid) {
                        selectPosition(position._lat, position._lon);
                    } else if (currentIndex === 2) {
                        page = pageStack.push("qrc:/silica/SearchPlace.qml", {
                                           "searchCenterLat": position._lat,
                                           "searchCenterLon": position._lon,
                                           "acceptLabel": qsTr("Accept")
                                       });
                        ToolBox.connectOnce(page.selectLocation, selectLocation);
                    } else if (currentIndex === 3) {
                        ToolBox.connectOnce(routingDialog.placePicked, picked);
                        routingDialog.state = "pickPlace";
                    } else if (currentIndex === 4) {
                        page = pageStack.push("qrc:/silica/Favorites.qml", { "state": "selection" });
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
                color: styleMap.popover.foregroundColor
                font.pixelSize: units.fx("small")
            }

            // Route to

            SectionHeader {
                text: qsTr("Destination")
            }
            ComboBox {
                id: to
                width: parent.width
                leftMargin: 0
                rightMargin: 0
                menu: ContextMenu {
                    MenuItem { text: placeTo.address }
                    MenuItem { text: qsTr("Search Place") }
                    MenuItem { text: qsTr("Select on Map") }
                    MenuItem { text: qsTr("Favorite") }
                }
                Component.onCompleted: currentIndex = 0
                onCurrentIndexChanged: {
                    var page = null;
                    if (currentIndex === 1) {
                        page = pageStack.push("qrc:/silica/SearchPlace.qml", {
                                       "searchCenterLat": position._lat,
                                       "searchCenterLon": position._lon,
                                       "acceptLabel": qsTr("Accept")
                                   });
                        ToolBox.connectOnce(page.selectLocation, selectLocation);
                    } else if (currentIndex === 2) {
                        ToolBox.connectOnce(routingDialog.placePicked, picked);
                        routingDialog.state = "pickPlace";
                    } else if (currentIndex === 3) {
                        page = pageStack.push("qrc:/silica/Favorites.qml", { "state": "selection" });
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
                color: styleMap.popover.foregroundColor
                font.pixelSize: units.fx("small")
            }

            ProgressBar {
                height: units.gu(2)
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                opacity: !routingModel.ready && routeProgress > 0 ? 1.0 : 0.0
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
                font.pixelSize: units.fx("small")
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
                        // When current position cannot be defined, i.e sensor disabled,
                        // reset the current position at the location of the start point
                        // of the route. In other way it could schedule reroute.
                        if (!position._posValid) {
                            position._lat = placeFrom.lat;
                            position._lon = placeFrom.lon;
                        }
                        mapView.navigation = true;
                        navigator.stopped.connect(onNavigatorStopped);
                        navigator.setup(vehicle, routingModel, placeTo.location);
                        routingDialog.state = "navigation";
                        routingDialog.close();
                    }
                    label.text: qsTr("Navigate")
                    enabled: routingDialog.state === "navigate"
                    opacity: enabled ? 1.0 : 0.5

                    function onNavigatorStopped() {
                        navigator.stopped.disconnect(onNavigatorStopped);
                        routingDialog.state = "navigate";
                    }
                }
                MapIcon {
                    id: clearButton
                    source: "image://theme/icon-m-delete"
                    height: units.gu(5)
                    width: parent.width / 2 - units.gu(1)
                    color: styleMap.popover.foregroundColor
                    onClicked: {
                        routingModel.cancel();
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
                model: routingModel

                header: Row {
                    spacing: units.gu(1)
                    width: parent.width
                    Label {
                        text: qsTr("Route length:")
                        color: styleMap.popover.foregroundColor
                        font.pixelSize: units.fx("small")
                    }
                    Label {
                        id: distanceLabel
                        text: Converter.readableDistance(routingModel.length)
                        color: styleMap.popover.highlightedColor
                        font.pixelSize: units.fx("small")
                    }
                    Label {
                        text: ", " + qsTr("Duration:")
                        color: styleMap.popover.foregroundColor
                        font.pixelSize: units.fx("small")
                    }
                    Label {
                        id: durationLabel
                        text: Converter.panelDurationHM(routingModel.duration)
                        color: styleMap.popover.highlightedColor
                        font.pixelSize: units.fx("small")
                    }
                }
                delegate: Row {
                    spacing: units.gu(2)
                    width: parent.width
                    height: Math.max(stepInfo.implicitHeight, icon.height)

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
                    Column {
                        id: stepInfo
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - icon.width - units.gu(2)
                        Label {
                            id: distance
                            width: parent.width
                            color: styleMap.popover.foregroundColor
                            text: Converter.panelDistance(model.distance) + " ~ " + Converter.panelDurationHM(model.time)
                            font.pixelSize: units.fx("small")
                            horizontalAlignment: Label.AlignRight
                        }
                        Label {
                            id: entryDescription
                            width: parent.width
                            color: styleMap.popover.foregroundColor
                            text: model.description
                            font.pixelSize: units.fx("small")
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }

    property int routeProgress: 0
    property string routeMessage: ""
    property bool computeRunning: false

    RoutingListModel {
        id: routingModel
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
                routingModel.clear();
            } else {
                var count = routingModel.count;
                if (count > 0) {
                    if (count > settings.maximumRouteStep) {
                        popInfo.open("The number of steps exceeds the limit. Please reduce the length of the route and restart the calculation.");
                        routingModel.clear();
                    } else {
                        routingDialog.state = "navigate";
                    }
                    computeRunning = false;
                }
            }
        }
    }

    function breakCompute() {
        routingModel.cancel();
        computeRunning = false;
    }

    function computeRoute() {
        routeProgress = 0;
        routeMessage = "";
        placeFrom.location = routingModel.locationEntryFromPosition(placeFrom.lat, placeFrom.lon);
        placeTo.location = routingModel.locationEntryFromPosition(placeTo.lat, placeTo.lon);
        if (placeFrom.location && placeTo.location) {
            computeRunning = true;
            routingModel.setStartAndTarget(placeFrom.location, placeTo.location, vehicle);
            settings.lastVehicle = vehicle;
        } else {
            routeMessage = qsTr("Invalid entry");
            routingModel.clear();
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
