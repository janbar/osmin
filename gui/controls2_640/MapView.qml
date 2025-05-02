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
import QtQml 2.2
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
        PlatformExtras.setPreventBlanking(preventBlanking, 1); // lock bit 1
    }

    function showPositionInfo() {
        if (positionSource._posValid) {
            popInfo.open(qsTr("Current position is %1").arg(Converter.readableCoordinatesGeocaching(positionSource._lat, positionSource._lon)));
        } else {
            popInfo.open(qsTr("Current position cannot be gathered"));
        }
    }

    function setStyleFlags(flags) {
        // flags: [{name,value}, ...]
        if (flags.length > 0) {
            var styleFlags = MapExtras.getStyleFlags();
            for (var i = 0; i < flags.length; ++i) {
                for (var j = 0; j < styleFlags.length; ++j) {
                    if (styleFlags[j].name === flags[i].name) {
                        console.log("set style flag '" + flags[i].name + "' to " + flags[i].value);
                        styleFlags[j].value = flags[i].value;
                    }
                }
            }
            console.log("reload customized style");
            MapExtras.reloadStyle(styleFlags);
        }
    }

    Component.onCompleted: {
        // Syncing all data from the tracker
        Service.ping("ALL");
        // show current coordinates
        ToolBox.connectOnce(positionSource.dataUpdated, function(){
            if (positionSource._posValid) {
                map.showCoordinatesInstantly(positionSource._lat, positionSource._lon);
            }
            showPositionInfo();
        });
        // load current course or clean
        if (settings.courseId > 0) {
            if (!loadGPX(settings.courseId)) {
                settings.courseId = 0;
            }
        }

        // configure style from setting
        var flags = JSON.parse(settings.styleFlags);
        if (Array.isArray(flags)) {
            flags.push({ "name": "daylight", "value": !nightView });
            setStyleFlags(flags);
        }

        // show/hide favorite POIs
        if (mainView.showFavorites)
            overlayManager.showFavorites();
    }

    property QtObject mark: QtObject {
        property bool showOverlay: true
        property real screenX: 0.0
        property real screenY: 0.0
        property double lat: 0.0
        property double lon: 0.0
    }

    property bool nightView: false
    property bool rotateEnabled: false
    property real rotation: 0.0 // rotation of the map (radians)
    property bool lockRotation: true // lock or unlock rotation of the map

    onRotationChanged: {
        if (lockRotation)
            rotator.rotateTo(rotation, map.lockToPosition);
    }
    onLockRotationChanged: {
        if (lockRotation)
            rotator.rotateTo(rotation, map.lockToPosition);
        else
            rotator.stop();
    }

    Map {
        id: map
        anchors.fill: parent

        // by default use plane unless tiled is forced
        renderingType: ((mapView.rotateEnabled || mapView.navigation) && settings.renderingTypeTiled  ? "tiled" : "plane")

        followVehicle: mapView.navigation
        vehiclePosition: mapView.navigation && navigator.ready ? navigator.vehiclePosition : Tracker.trackerPosition
        vehicleIconSize: 10
        showCurrentPosition: true
        interactiveIcons: true

        TiledMapOverlay {
            id: mapOverlay
            anchors.fill: parent
            view: map.view
            enabled: settings.hillShadesEnabled && hillshadeProvider
            opacity: 0.6
            provider: hillshadeProvider
        }

        onTap: {
            mark.screenX = screenX;
            mark.screenY = screenY;
            mark.lat = lat;
            mark.lon = lon;
            switch (mapView.state) {
            case "view":
                if (!edgeToolbar.visible)
                    mapView.showToolbar = true;
                break;
            case "locationInfo":
                // check icon tapped previously
                if (popLocationInfo.visible && !mark.showOverlay)
                    popLocationInfo.close();
                else
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
            mark.showOverlay = true;
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

        onIconTapped: {
            mark.showOverlay = false;
            mark.screenX = screenCoord.x;
            mark.screenY = screenCoord.y;
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

        Component.onCompleted: {
            map.setVehicleScaleFactor(0.1); // hide vehicle
            positionSource.dataUpdated.connect(function(valid, lat, lon, accValid, acc, alt){
                locationChanged(valid, lat, lon, accValid, acc);
            });
            ToolBox.connectWhileFalse(map.finishedChanged, function(finished){
               if (finished) {
                   console.log("Configure after finished map");
                   //rotateEnabled = true;
                   positionSource.active = true;
               }
            });
        }
    }

    states: [
        State {
            name: "view"
        },
        State {
            name: "locationInfo"
            PropertyChanges { target: searchAroundPlace; visible: true; }
            PropertyChanges { target: addFavorite; visible: true; }
            PropertyChanges { target: goThere; visible: true; }
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

    onNavigationChanged: {
        if (navigation) {
            // unlock rotation and disable rotate
            lockRotation = false;
            rotateEnabled = false;
            // show vehicle icon
            map.setVehicleScaleFactor(1.0);
        } else {
            // hide vehicle icon
            map.setVehicleScaleFactor(0.1);
            // lock rotation
            mapView.rotation = 0.0;
            lockRotation = true;
        }
    }

    property int positionState: !positionSource._posValid ? 0 : (!navigation ? 1 : 2)

    function positionColor(val) {
        switch(val) {
        case 0: return "transparent";
        case 1: return "limegreen"
        case 2: return "deepskyblue"
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
        animationRunning: (positionState === 0)
        onClicked: {
            switch(positionState) {
            case 0:
                // try to activate the source provided by service
                Service.positionActive = true;
                break;
            case 1:
                // enable navigation
                navigation = true;
                break;
            case 2:
                // disable navigation
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

    Label {
        id: copyright
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        text: "© OpenStreetMap contributors"
        font.pointSize: units.fs("x-small")
        font.weight: Font.Thin
        color: nightView ? "white" : "black"
        visible: !showToolbar && !navigation
    }

    MapIcon {
        id: trackRecording
        anchors{
            bottom: parent.bottom
            left: parent.left
            bottomMargin: units.gu(1)
            leftMargin: units.gu(1)
        }
        source: "qrc:/images/record.svg"
        backgroundColor: "white"
        color: Tracker.isRecording ? "red" : "black"
        animationRunning: Tracker.isRecording && !navigation
        visible: mapView.state === "view" && !showToolbar && (navigation || Tracker.isRecording)
        borderPadding: units.gu(0.0)
        opacity: 0.7
        height: units.gu(6)
        onClicked: {
            if (Tracker.isRecording)
                Tracker.stopRecording();
            else
                Tracker.startRecording();
        }
    }

    MapIcon {
        id: trackMarkPosition
        anchors{
            bottom: parent.bottom
            left: trackRecording.right
            bottomMargin: units.gu(1)
            leftMargin: units.gu(1)
        }
        source: "qrc:/images/trip/marker.svg"
        color: "black"
        backgroundColor: "white"
        visible: mapView.state === "view" && !showToolbar && Tracker.isRecording
        borderPadding: units.gu(1.0)
        label.text: qsTr("Mark")
        label.font.pointSize: units.fs("medium")
        label.color: "black"
        opacity: 0.7
        height: units.gu(6)
        onClicked: {
            Tracker.pinPosition(); // pin the current position
            var name = "[" + Converter.panelDistance(Tracker.distance) + "]";
            dialogMarkPosition.model = { "name": name };
            dialogMarkPosition.open();
            ToolBox.connectOnce(dialogMarkPosition.reply, function(model){
                if (model !== null) {
                    Tracker.markPosition(model.symbol, model.name, "");
                }
            });
        }
    }

    DialogMarkPosition {
        id: dialogMarkPosition
    }

    // button for zoom out
    MapIcon {
        id: buttonZoomOut
        anchors{
            bottom: buttonLocation.top
            right: parent.right
            bottomMargin: units.gu(1)
            rightMargin: units.gu(1)
        }
        source: "qrc:/images/zoomout.svg"
        color: "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.5)
        opacity: 0.7
        height: units.gu(6)
        onClicked: map.zoomOut(1.732)
    }

    MapIcon {
        id: buttonZoomIn
        anchors{
            bottom: buttonZoomOut.top
            right: parent.right
            bottomMargin: units.gu(1)
            rightMargin: units.gu(1)
        }
        source: "qrc:/images/zoomin.svg"
        color: "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.5)
        opacity: 0.7
        height: units.gu(6)
        onClicked: map.zoomIn(1.732)
    }

    MapIcon {
        id: buttonRotate
        anchors{
            top: parent.top
            right: parent.right
            topMargin: units.gu(1)
            rightMargin: units.gu(1)
        }
        source: "qrc:/images/compass.svg"
        color: rotateEnabled ? "white" : "black"
        backgroundColor: rotateEnabled ? "black" : "white"
        borderPadding: 0
        opacity: rotateEnabled || navigation ? 0.9 : 0.7
        height: units.gu(6)
        onClicked: rotateEnabled = !rotateEnabled
        rotation: navigation ? (-180.0 * Tracker.bearing / Math.PI) : (180.0 * mapView.rotation / Math.PI)
        visible: !popNavigatorInfo.visible
        enabled: !navigation
    }

    Item {
        visible: navigation && !popNavigatorInfo.visible
        opacity: 0.9
        anchors{
            top: parent.top
            left: parent.left
            topMargin: units.gu(1)
            leftMargin: units.gu(1)
        }
        Column {
            Label {
                id: currentSpeed
                text: Converter.readableSpeed(Tracker.currentSpeed)
                font.pointSize: 1.5 * units.fs("x-large")
                color: nightView ? "white" : "black"
            }
            Row {
                spacing: units.gu(2)
                Label {
                    id: duration
                    text: Converter.panelDurationHMS(Tracker.duration)
                    font.pointSize: units.fs("medium")
                    color: nightView ? "white" : "black"
                }
                Label {
                    id: distance
                    text: Converter.panelDistance(Tracker.distance)
                    font.pointSize: units.fs("medium")
                    color: nightView ? "white" : "black"
                }
            }
            MapIcon {
                id: elevation
                source: "qrc:/images/trip/elevation.svg"
                color: nightView ? "white" : "black"
                enabled: false
                height: units.gu(2)
                borderPadding: 0
                label.font.pointSize: units.fs("medium")
                label.text: Converter.panelElevation(Tracker.elevation)
            }
        }
    }

    OverlayManager {
        id: overlayManager
        map: map
    }

    // Go there. The button is visible in state 'locationInfo'
    MapIcon {
        id: goThere
        visible: false
        z: 1
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: units.gu(1)
        source: "qrc:/images/trip/navigator.svg"
        color: "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.0)
        opacity: 0.7
        height: units.gu(6)
        label.text: qsTr("Go")
        label.font.pointSize: units.fs("medium")
        label.color: "black"
        onClicked: {
            navigateTo(popLocationInfo.placeLat,
                       popLocationInfo.placeLon,
                       popLocationInfo.placeLabel);
            popLocationInfo.close();
        }
    }
    // Add/Remove a favorite place. The button is visible in state 'locationInfo'
    MapIcon {
        id: addFavorite
        visible: false
        z: 1
        anchors.bottom: parent.bottom
        anchors.left: goThere.right
        anchors.margins: units.gu(1)
        source: "qrc:/images/trip/favourite.svg"
        color: popLocationInfo.isFavorite > 0 ? "deepskyblue" : "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.0)
        opacity: 0.7
        height: units.gu(6)
        onClicked: {
            if (popLocationInfo.isFavorite === 0) {
                dialogEnter.title = qsTr("Add favorite");
                dialogEnter.userEntry = popLocationInfo.placeLabel;
                dialogEnter.open();
                ToolBox.connectOnce(dialogEnter.reply, function(accepted, entry){
                    if (accepted) {
                        var label = entry.trim();
                        popLocationInfo.isFavorite = createFavorite(popLocationInfo.placeLat,
                                                        popLocationInfo.placeLon,
                                                        (label.length > 0 ? label : popLocationInfo.placeLabel),
                                                        popLocationInfo.placeType);
                    }
                });
            } else {
                var favorite = FavoritesModel.getById(popLocationInfo.isFavorite);
                dialogAction.title = qsTr("Delete favorite ?");
                dialogAction.text = favorite.label;
                dialogAction.open();
                ToolBox.connectOnce(dialogAction.reply, function(accepted){
                    if (accepted && removeFavorite(popLocationInfo.isFavorite))
                        popLocationInfo.isFavorite = 0;
                });
            }
        }
    }
    // Search around place. The button is visible in state 'locationInfo'
    MapIcon {
        id: searchAroundPlace
        visible: false
        z: 1
        anchors.bottom: parent.bottom
        anchors.left: addFavorite.right
        anchors.margins: units.gu(1)
        source: "qrc:/images/trip/search.svg"
        color: "black"
        backgroundColor: "white"
        borderPadding: units.gu(1.0)
        opacity: 0.7
        height: units.gu(6)
        onClicked: {
            var page = stackView.push("qrc:/controls2/SearchPlace.qml", {
                               "searchCenterLat": popLocationInfo.placeLat,
                               "searchCenterLon": popLocationInfo.placeLon,
                               "acceptLabel": qsTr("Go"),
                               "acceptIcon" : "qrc:/images/trip/navigator.svg",
                               "showPositionEnabled": true
                           });
            ToolBox.connectOnce(page.selectLocation, function(location, lat, lon, label){
                 if (lat !== NaN && lon !== NaN && label !== "") {
                    popRouting.goTo(lat, lon, label);
                 }
            });
            ToolBox.connectOnce(page.showPosition, function(lat, lon){
                 if (lat !== NaN && lon !== NaN) {
                     map.showCoordinatesInstantly(lat, lon);
                     mark.showOverlay = true;
                     mark.lat = lat;
                     mark.lon = lon;
                     mark.screenX = map.width / 2;
                     mark.screenY = map.height / 2;
                     if (navigation)
                         navigation = false;
                     popLocationInfo.show();
                 }
            });
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
        anchors.bottomMargin: showToolbar && !edgeToolbar.visible ? 0 : - height
        anchors.left: parent.left
        anchors.right: parent.right
        z: 99

        Behavior on anchors.bottomMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutBack; }
        }

        Rectangle {
            id: horizontalToolBar
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
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            popMainMenu.show();
                        }
                    }

                    MapIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/trip/here.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            if (!lockRotation) {
                                map.lockToPosition = true;
                            } else if(positionSource._posValid) {
                                map.showCoordinatesInstantly(positionSource._lat, positionSource._lon);
                                showPositionInfo();
                            } else {
                                showPositionInfo();
                            }
                        }
                    }

                    MapIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/trip/pin.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            var page = stackView.push("qrc:/controls2/Favorites.qml");
                            ToolBox.connectOnce(page.showPosition, function(lat, lon){
                                 if (lat !== NaN && lon !== NaN) {
                                     map.showCoordinatesInstantly(lat, lon);
                                     mark.showOverlay = true;
                                     mark.lat = lat;
                                     mark.lon = lon;
                                     mark.screenX = map.width / 2;
                                     mark.screenY = map.height / 2;
                                     if (navigation)
                                         navigation = false;
                                     popLocationInfo.show();                                 }
                            });
                        }
                    }

                    MapIcon {
                        visible: true
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/trip/search.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            var page = stackView.push("qrc:/controls2/SearchPlace.qml", {
                                               "searchCenterLat": positionSource._lat,
                                               "searchCenterLon": positionSource._lon,
                                               "acceptLabel": qsTr("Go"),
                                               "acceptIcon" : "qrc:/images/trip/navigator.svg",
                                               "showPositionEnabled": true
                                           });
                            ToolBox.connectOnce(page.selectLocation, function(location, lat, lon, label){
                                 if (lat !== NaN && lon !== NaN && label !== "") {
                                    popRouting.goTo(lat, lon, label);
                                 }
                            });
                            ToolBox.connectOnce(page.showPosition, function(lat, lon){
                                 if (lat !== NaN && lon !== NaN) {
                                     map.showCoordinatesInstantly(lat, lon);
                                     mark.showOverlay = true;
                                     mark.lat = lat;
                                     mark.lon = lon;
                                     mark.screenX = map.width / 2;
                                     mark.screenY = map.height / 2;
                                     if (navigation)
                                         navigation = false;
                                     popLocationInfo.show();
                                 }
                            });
                        }
                    }

                    MapIcon {
                        visible: true
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/images/day-night.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            nightView = !nightView;
                            MapExtras.setDaylight(!nightView);
                        }
                    }
                }
            }
        }
    }

    Item {
        id: edgeToolbar
        width: units.gu(8)
        anchors.top: buttonRotate.bottom
        anchors.bottom: buttonZoomIn.top
        anchors.right: parent.right
        visible: !mainView.wideAspect && !popNavigatorInfo.visible
        z: 99

        Rectangle {
            id: verticalToolBar
            anchors.fill: parent
            color: "transparent"
            opacity: (!mainView.wideAspect && mapView.state === "view" ? 1.0 : 0.0)
            enabled: opacity > 0

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: units.gu(1)
                anchors.bottomMargin: units.gu(1)
                width: units.gu(8)
                color: "transparent"

                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: units.gu(1)

                    MapIcon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:/images/navigation-menu.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.5)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            popMainMenu.show();
                        }
                    }

                    MapIcon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:/images/trip/here.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            if (!lockRotation) {
                                map.lockToPosition = true;
                            } else if(positionSource._posValid) {
                                map.showCoordinatesInstantly(positionSource._lat, positionSource._lon);
                                showPositionInfo();
                            } else {
                                showPositionInfo();
                            }
                        }
                    }

                    MapIcon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:/images/trip/pin.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            var page = stackView.push("qrc:/controls2/Favorites.qml");
                            ToolBox.connectOnce(page.showPosition, function(lat, lon){
                                 if (lat !== NaN && lon !== NaN) {
                                     map.showCoordinatesInstantly(lat, lon);
                                     mark.showOverlay = true;
                                     mark.lat = lat;
                                     mark.lon = lon;
                                     mark.screenX = map.width / 2;
                                     mark.screenY = map.height / 2;
                                     if (navigation)
                                         navigation = false;
                                     popLocationInfo.show();                                 }
                            });
                        }
                    }

                    MapIcon {
                        visible: true
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:/images/trip/search.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            var page = stackView.push("qrc:/controls2/SearchPlace.qml", {
                                               "searchCenterLat": positionSource._lat,
                                               "searchCenterLon": positionSource._lon,
                                               "acceptLabel": qsTr("Go"),
                                               "acceptIcon" : "qrc:/images/trip/navigator.svg",
                                               "showPositionEnabled": true
                                           });
                            ToolBox.connectOnce(page.selectLocation, function(location, lat, lon, label){
                                 if (lat !== NaN && lon !== NaN && label !== "") {
                                    popRouting.goTo(lat, lon, label);
                                 }
                            });
                            ToolBox.connectOnce(page.showPosition, function(lat, lon){
                                 if (lat !== NaN && lon !== NaN) {
                                     map.showCoordinatesInstantly(lat, lon);
                                     mark.showOverlay = true;
                                     mark.lat = lat;
                                     mark.lon = lon;
                                     mark.screenX = map.width / 2;
                                     mark.screenY = map.height / 2;
                                     if (navigation)
                                         navigation = false;
                                     popLocationInfo.show();                                 }
                            });
                        }
                    }

                    MapIcon {
                        visible: true
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:/images/day-night.svg"
                        color: "black"
                        backgroundColor: "white"
                        borderPadding: units.gu(1.0)
                        opacity: 0.7
                        height: units.gu(6)
                        onClicked: {
                            nightView = !nightView;
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

    DialogAction {
        id: dialogAction
    }

    DialogEnter {
        id: dialogEnter
    }

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
            // exit navigation mode if the status of the position does not allow it
            if (mapView.positionState === 0) {
                mapView.navigation = false;
            }
        }
    }

    Connections {
        target: popNavigatorInfo
        function onVisibleChanged() {
            if (popNavigatorInfo.visible && !mapVoice.voiceValid) {
                popInfo.open(qsTr("The voice GPS driving directions is not activated"));
            }
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
            overlayManager.removeMark(0);
        }
        onShow: {
            if (mark.showOverlay) {
                map.interactiveIcons = false;
                overlayManager.addMark(0, mark.lat, mark.lon);
            }
            if (mark.screenY < widgetBottomY / 2)
                map.up();
            popLocationInfo.searchLocation(mark.lat, mark.lon);
            visible = true;
        }
        onVisibleChanged: {
            if (visible)
                mapView.pushState("locationInfo");
            else {
                map.interactiveIcons = true;
                mapView.popState("locationInfo");
            }
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
        maximumHeight: map.height - y /*- units.gu(8)*/
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
        height: map.height - y /*- units.gu(8)*/
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
        height: map.height - y /*- units.gu(8)*/
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

    Tracking {
        id: popTracking
        anchors {
            top: popInfo.bottom
            left: parent.left
            right: parent.right
        }
        height: map.height - y /*- units.gu(8)*/
        visible: false
        onClose: visible = false
        onShow: visible = true
        onVisibleChanged: {
            if (visible)
                mapView.pushState("Tracking");
            else
                mapView.popState("Tracking");
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Navigation
    ////

    Navigator {
        id: navigator
        position: positionSource
        overlayManager: overlayManager

        onStarted: {
            popNavigatorInfo.visible = Qt.binding(function() { return mapView.state === "view"; });
        }

        onTargetReached: {
            popInfo.open(qsTr("Target reached at %1 %2.").arg(Converter.readableDistance(targetDistance)).arg(Converter.readableBearing(targetBearing)));
        }
    }

    MapRotator {
        id: rotator
        map: map
    }

    property var overlayRecording: map.createOverlayWay("_track");
    property bool isRecording: false

    Connections {
        target: Tracker
        function onRecordingFailed() {
            popInfo.open(qsTr("Track recording failed"));
        }
        function onIsRecordingChanged() {
            // clear previous recording
            if (Tracker.isRecording && !mapView.isRecording) {
                overlayManager.removeRecording();
                overlayManager.removeMark(1);
                overlayRecording.clear();
            }
            mapView.isRecording = Tracker.isRecording;
        }
        function onTrackerPositionRecorded(lat, lon) {
            overlayRecording.addPoint(lat, lon);
            overlayManager.addRecording(overlayRecording);
        }
        function onTrackerPositionMarked(lat, lon, symbol, name) {
            overlayManager.addMark(1, lat, lon);
        }
    }

    Connections {
        target: mainView
        function onApplicationSuspendedChanged() {
            // On android disable navigation when the app is suspended
            if ((Android || DeviceMobile) && applicationSuspended && navigation)
                navigation = false;
        }
        function onShowFavoritesChanged() {
            if (showFavorites)
                overlayManager.showFavorites();
            else
                overlayManager.hideFavorites();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Courses
    ////

    // Store the selected course
    property int courseId: 0

    function addCourse(bid, overlays) {
        if (overlayManager.addCourse(bid, overlays)) {
            settings.courseId = courseId = bid;
        }
    }

    function removeCourse() {
        overlayManager.removeAllCourses();
        settings.courseId = courseId = 0;
    }

    GPXFileModel {
        id: courseFile
        onParseFinished: function(succeeded) {
            if (succeeded)
                loadData();
        }
        onProgressChanged: {
            popInfo.open(qsTr("Loading") + " " + (Math.round(courseFile.progress * 1000) / 10).toFixed(1) + " %");
        }
    }

    function loadGPX(bid) {
        GPXListModel.loadData();
        var file = GPXListModel.findFileById(bid);
        if (file !== "") {
            ToolBox.connectOnce(courseFile.loaded, function(succeeded){
                if (succeeded)
                    mapView.addCourse(bid, courseFile.createOverlayObjects());
            });
            courseFile.parseFile(file);
            return true;
        } else {
            return false;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Rotation
    ////

    onRotateEnabledChanged: {
        configureRotation(rotateEnabled);
    }

    function configureRotation(enabled) {
        if (enabled) {
            Tracker.trackerPositionChanged.connect(handleRotation);
        } else {
            Tracker.trackerPositionChanged.disconnect(handleRotation);
            mapView.rotation = 0.0;
        }
    }

    function handleRotation() {
        var pi2 = Math.PI*2.0;
        var d = pi2 - Tracker.bearing - map.view.angle;
        d = (d > Math.PI ? d - pi2 : d < -Math.PI ? d + pi2 : d);
        if (d > 0.14 || d < -0.14) {
            let r = map.view.angle + d;
            mapView.rotation = (r > pi2 ? r - pi2 : r < 0 ? r + pi2 : r);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// About the tracker service
    ////

    Connections {
        target: Service
        function onStatusChanged() {
            if (Service.status === Service.ServiceConnected) {
                popInfo.open(qsTr("Tracker service is connected"), "limegreen", "black");
                // clear all data before ping ALL
                Tracker.magneticDip = settings.magneticDip;
                overlayManager.removeRecording();
                overlayManager.removeMark(1);
                overlayRecording.clear();
                Service.ping("ALL");
            } else {
                popInfo.open(qsTr("Tracker service has been disconnected"));
            }
        }
    }
}
