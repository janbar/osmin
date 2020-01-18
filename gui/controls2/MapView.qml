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
import QtQuick.Layouts 1.3
import Osmin 1.0
import "./components"
import "../toolbox.js" as ToolBox

MapPage {
    id: mapView
    pageTitle: qsTr("Map View")
    showHeader: false // hide the header bar

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global functions
    ////

    function navigateTo(lat, lon ,label) {
        popRouting.goTo(lat, lon, label);
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Map
    ////

    property real widgetBottomY: (map.height / 2) * 1.05 // the map view should be visible below

    property bool preventBlanking: navigation && !applicationSuspended
    onPreventBlankingChanged: {
        if (PlatformExtras.preventBlanking !== preventBlanking) {
            PlatformExtras.preventBlanking = preventBlanking;
            console.log("PreventBlanking: " + preventBlanking);
        }
    }

    Component.onCompleted: {
        positionSource.update();
        if (positionSource._posValid) {
            map.showCoordinates(positionSource._lat, positionSource._lon);
            popInfo.open(qsTr("Current position is %1").arg(Converter.readableCoordinatesGeocaching(positionSource._lat, positionSource._lon)));
        } else {
            popInfo.open(qsTr("Current position cannot be gathered"));
        }
    }

    ListModel {
        id: mapOverlays
        ListElement {
            title: "Hill Shading"
            provider: "{ \"id\":\"wmflabs\", \"name\":\"wmflabs\", \"servers\":[\"http://tiles.wmflabs.org/hillshading/%1/%2/%3.png\"], \"maximumZoomLevel\":18, \"copyright\":\"© wmflabs Hillshading\" }"
        }
    }

    property QtObject mark: QtObject {
        property real screenX: 0.0
        property real screenY: 0.0
        property double lat: 0.0
        property double lon: 0.0
    }

    Map {
        id: map
        anchors.fill: parent
        renderingType: (mapUserSettings.renderingTypeTiled || (mapView.navigation && navigator.ready) ? "tiled" : "plane")

        followVehicle: mapView.navigation && navigator.ready
        vehiclePosition: navigator.ready ? navigator.vehiclePosition : null
        vehicleIconSize: 10
        showCurrentPosition: true

        TiledMapOverlay {
            id: mapOverlay
            anchors.fill: parent
            view: map.view
            enabled: mapUserSettings.hillShadesEnabled
            opacity: 0.6
            provider: JSON.parse(mapOverlays.get(0).provider)
        }

        onTap: {
            mark.screenX = screenX;
            mark.screenY = screenY;
            mark.lat = lat;
            mark.lon = lon;
            switch (mapView.state) {
            case "view":
                mapView.showToolbar = true;
                break;
            case "locationInfo":
                popLocationInfo.show();
                break;
            case "routing":
                popRouting.placePicked(true, lat, lon);
                break;
            default:
                break;
            }
        }

        onLongTap: {
            mark.screenX = screenX;
            mark.screenY = screenY;
            mark.lat = lat;
            mark.lon = lon;
            switch (mapView.state) {
            case "view":
                popLocationInfo.show();
                break;
            default:
                break;
            }
        }

        onLockToPositionChanged: {
            // delay to lock position again while navigation
            if (!lockToPosition && navigation && !followVehicle) {
                delayLockPosition.start();
            }
        }

        Timer {
            id: delayLockPosition
            interval: 5000
            onTriggered: {
                if (navigation && !map.followVehicle) {
                    if (mapView.state === "view")
                        map.lockToPosition = true;
                    else
                        restart(); // wait for state view
                }
            }
        }

        Component.onCompleted: {
            setPixelRatio(ScreenScaleFactor);
            positionSource.dataUpdated.connect(locationChanged);
        }
    }

    states: [
        State {
            name: "view"
        },
        State {
            name: "locationInfo"
            PropertyChanges { target: addFavorite; visible: true; }
            PropertyChanges { target: addMarker; visible: true; }
        },
        State {
            name: "pickLocation"
        },
        State {
            name: "measureDistance"
        },
        State {
            name: "routing"
        }
    ]
    state: "view"
    property var stateStack: [ "view" ]

    function pushState(state) {
        var tmp = [];
        for (var i = 0; i < stateStack.length; ++i) {
            if (i === 0 || stateStack[i] !== state)
                tmp.push(stateStack[i]);
        }
        tmp.push(state);
        stateStack = tmp;
        mapView.state = state;
        console.log("mapView.state: " + mapView.state);
    }
    function popState(state) {
        var tmp = [];
        for (var i = 0; i < stateStack.length; ++i) {
            if (i === 0 || stateStack[i] !== state)
                tmp.push(stateStack[i]);
        }
        stateStack = tmp;
        mapView.state = tmp[tmp.length - 1];
        console.log("mapView.state: " + mapView.state);
    }

    // property to enable/disable the navigation mode
    property bool navigation: false

    property int positionState: !positionSource._posValid || !positionSource.active ? 0
                                 : !navigation ? 1
                                 : map.lockToPosition ? 2
                                 : 3

    function positionColor(val) {
        switch(val) {
        case 0: return "transparent";
        case 1: return "limegreen"
        case 2: return "deepskyblue"
        case 3: return "deepskyblue"
        default: return "lightgrey"
        }
    }

    // button for location
    MapIcon {
        id: buttonLocation
        anchors{
            bottom: parent.bottom
            right: parent.right
            bottomMargin: units.gu(1)
            rightMargin: units.gu(1)
        }
        source: "qrc:/images/trip/location.svg"
        backgroundColor: positionColor(positionState)
        color: positionState === 0 ? "red" : "black"
        borderPadding: units.gu(0)
        opacity: 0.9
        height: units.gu(6)
        animationRunning: (positionState === 0 || positionState === 3)
        onClicked: {
            switch(positionState) {
            case 0:
                // try to enable the source
                positionSource.active = true;
                break;
            case 1:
                // enable navigation until next click
                navigation = true;
                // lock to the current position when no route
                if (!navigator.ready) {
                    map.lockToPosition = true;
                }
                break;
            case 2:
            case 3:
                // disable navigation until next click
                navigation = false;
                break;
            default:
                break;
            }
        }
    }

    ScaleIndicator{
        id: scaleIndicator
        pixelSize: map.pixelSize
        color: "black"
        visible: !showToolbar
        anchors{
          bottom: parent.bottom
          right: buttonLocation.left
          bottomMargin: units.gu(1)
          rightMargin: units.gu(1)
        }
    }

    // button for zoom out
    MapIcon {
        id: buttonZoomOut
        anchors{
            bottom: buttonLocation.top
            right: parent.right
            bottomMargin: units.gu(2)
            rightMargin: units.gu(1)
        }
        source: "qrc:/images/zoomout.svg"
        color: "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.5)
        opacity: 0.5
        height: units.gu(6)
        onClicked: map.zoomOut(1.732)
    }

    MapIcon {
        id: buttonZommIn
        anchors{
            bottom: buttonZoomOut.top
            right: parent.right
            bottomMargin: units.gu(2)
            rightMargin: units.gu(1)
        }
        source: "qrc:/images/zoomin.svg"
        color: "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.5)
        opacity: 0.5
        height: units.gu(6)
        onClicked: map.zoomIn(1.732)
    }

    function addRoute(id, routeWay) {
        map.addOverlayObject(id, routeWay);
    }
    function removeRoute(id) {
        map.removeOverlayObject(id);
    }

    function addMarkStart(lat, lon) {
        map.addPositionMark(501, lat, lon);
    }
    function removeMarkStart() {
        map.removePositionMark(501);
    }
    function addMarkEnd(lat, lon) {
        map.addPositionMark(502, lat, lon);
    }
    function removeMarkEnd() {
        map.removePositionMark(502);
    }

    function addMark(id, lat, lon) {
        map.addPositionMark(1000 + id, lat, lon);
    }
    function removeMark(id) {
        map.removePositionMark(1000 + id);
    }

    function addWayPoint(id, lat, lon) {
        var wpt = map.createOverlayNode("_waypoint");
        wpt.addPoint(lat, lon);
        wpt.name = "Pos: " + lat.toFixed(4) + " " + lon.toFixed(4);
        map.addOverlayObject(2000 + id, wpt);
    }
    function removeWayPoint(id) {
        map.removeOverlayObject(2000 + id);
    }


    // Add/Remove a favorite place. The button is visible in state 'locationInfo'
    MapIcon {
        id: addFavorite
        visible: false
        z: 1
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: units.gu(1)
        source: "qrc:/images/trip/favourite.svg"
        color: popLocationInfo.isFavorite > 0 ? "deepskyblue" : "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.0)
        opacity: 0.7
        height: units.gu(6)
        onClicked: {
            if (popLocationInfo.isFavorite === 0) {
                popLocationInfo.isFavorite = createFavorite(popLocationInfo.placeLat,
                                                popLocationInfo.placeLon,
                                                popLocationInfo.placeLabel,
                                                popLocationInfo.placeType);
            } else if (removeFavorite(popLocationInfo.isFavorite)) {
                popLocationInfo.isFavorite = 0;
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Toolbar
    ////

    property bool showToolbar: false

    onShowToolbarChanged: {
        if (showToolbar && !hideFooter.running) {
            hideFooter.start();
        }
    }

    Timer {
        id: hideFooter
        interval: 5000
        onTriggered: {
            showToolbar = false;
        }
    }

    Item {
        id: footerToolbar
        height: units.gu(8)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: showToolbar ? 0 : - height
        anchors.left: parent.left
        anchors.right: parent.right
        z: 99

        Behavior on anchors.bottomMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutBack; }
        }

        Rectangle {
            id: defaultToolBar
            anchors.fill: parent
            color: "transparent"
            opacity: mapView.state === "view" ? 1.0 : 0.0
            enabled: opacity > 0

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(1)
                anchors.rightMargin: units.gu(1)
                height: units.gu(8)
                color: "transparent"

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: units.gu(1)

                    MapIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/navigation-menu.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.5)
                        opacity: 0.9
                        height: units.gu(6)
                        onClicked: {
                            popMainMenu.show();
                        }
                    }

                    MapIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/trip/info.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.9
                        height: units.gu(6)
                        onClicked: {
                        }
                    }

                    MapIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/trip/pin.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.9
                        height: units.gu(6)
                        onClicked: {
                            var page = stackView.push("qrc:/controls2/Favorites.qml");
                        }
                    }

                    MapIcon {
                        id: find
                        visible: true
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/trip/search.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.9
                        height: units.gu(6)
                        onClicked: {
                            var page = stackView.push("qrc:/controls2/SearchPlace.qml", {
                                               "searchCenterLat": positionSource._lat,
                                               "searchCenterLon": positionSource._lon,
                                               "acceptLabel": qsTr("Go"),
                                               "acceptIcon" : "qrc:/images/trip/navigator.svg"
                                           });
                             ToolBox.connectOnce(page.selectLocation, function(location, lat, lon, label){
                                 if (lat !== NaN && lon !== NaN && label !== "") {
                                    popRouting.goTo(lat, lon, label);
                                 }
                            });
                        }
                    }

                    MapIcon {
                        id: cancel
                        visible: true
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/day-night.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.9
                        height: units.gu(6)
                        onClicked: {
                            map.toggleDaylight();
                        }
                    }
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Widgets
    ////

    PopInfo {
        id: popInfo
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }

    NavigatorInfo {
        id: popNavigatorInfo
        navigator: navigator
        anchors {
            top: popInfo.bottom
            left: parent.left
            right: parent.right
        }
        maximumHeight: units.gu(16)
        visible: false
        onClose: {
            navigator.stop();
            visible = false;
        }
    }

    LocationInfo {
        id: popLocationInfo
        position: positionSource
        anchors {
            top: popInfo.bottom
            left: parent.left
            right: parent.right
        }
        maximumHeight: widgetBottomY
        visible: false
        onClose: {
            visible = false;
            mapView.removeMark(0);
        }
        onShow: {
            mapView.addMark(0, mark.lat, mark.lon);
            if (mark.screenY < widgetBottomY / 2)
                    map.up();
            popLocationInfo.searchLocation(mark.lat, mark.lon);
            visible = true;
        }
        onVisibleChanged: {
            if (visible)
                mapView.pushState("locationInfo");
            else
                mapView.popState("locationInfo");
        }
    }

    Routing {
        id: popRouting
        position: positionSource
        navigator: navigator
        anchors {
            top: popInfo.bottom
            left: parent.left
            right: parent.right
        }
        maximumHeight: map.height - units.gu(8) - y
        height: maximumHeight
        visible: false
        onClose: visible = false
        onShow: visible = true
        onVisibleChanged: {
            if (visible)
                mapView.pushState("routing");
            else
                mapView.popState("routing");
        }

        function goTo(lat, lon, label) {
            placeTo.lat = lat;
            placeTo.lon = lon;
            placeTo.address = label;
            placeTo.valid = true;
            placeFrom.lat = position._lat;
            placeFrom.lon = position._lon;
            placeFrom.address = qsTr("My Position");
            placeFrom.valid = true;
            state = "dialog";
            show();
        }
    }

    MainMenu {
        id: popMainMenu
        anchors {
            top: popInfo.bottom
            left: parent.left
            right: parent.right
        }
        height: map.height - units.gu(8) - y
        visible: false
        onClose: visible = false
        onShow: visible = true
        onVisibleChanged: {
            if (visible)
                mapView.pushState("mainMenu");
            else
                mapView.popState("mainMenu");
        }
    }

    ConfigureMap {
        id: popConfigureMap
        anchors {
            top: popInfo.bottom
            left: parent.left
            right: parent.right
        }
        height: map.height - units.gu(8) - y
        visible: false
        onClose: visible = false
        onShow: visible = true
        onVisibleChanged: {
            if (visible)
                mapView.pushState("configureMap");
            else
                mapView.popState("configureMap");
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Navigation
    ////

    Navigator {
        id: navigator
        position: positionSource

        onStarted: {
            popNavigatorInfo.visible = Qt.binding(function() { return mapView.state === "view"; });
        }

        onReadyChanged: {
            if (mapView.navigation) {
                if (!navigator.ready)
                    rotator.rotateTo(0);
                else
                    rotator.stop();
            }
        }

        onTargetReached: {
            popInfo.open(qsTr("Target reached, in %1 %2.").arg(Converter.readableDistance(targetDistance)).arg(Converter.readableBearing(targetBearing)));
        }
    }

    MapRotator {
        id: rotator
        map: map
    }

    onNavigationChanged: {
        if (mapView.navigation) {
            if (navigator.ready)
                rotator.stop();
        } else {
            if (navigator.ready)
                rotator.rotateTo(0);
        }
    }
}
