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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Osmin 1.0 as Osmin
import "../toolbox.js" as ToolBox
import "components"

ApplicationWindow {
    id: mainView

    initialPage: Component {
        Banner {
        }
    }

    cover: "qrc:/silica/CoverPage.qml"
    allowedOrientations: defaultAllowedOrientations

    ConfigurationGroup {
        id: settings
        path: "/io/github/janbar/osmin"
        synchronous: true

        // General settings
        property string style: "Material"
        property int theme: 0
        property real scaleFactor: 1.0
        property real fontScaleFactor: 1.0
        property bool firstRun: true

        // Navigation settings
        property string systemOfUnits: "SI"
        property bool hillShadesEnabled: false
        property bool renderingTypeTiled: false
        property string lastVehicle: "car"
        property int maximumRouteStep: 255
        property int courseId: 0
        property real magneticDip: 0.0

        // Tracker settings
        property string trackerRecording: ""
    }

    StyleLight {
        id: styleMap
    }

    Units {
        id: units
        scaleFactor: settings.scaleFactor
        fontScaleFactor: settings.fontScaleFactor
    }

    // Variables
    property int debugLevel: 2          // My debug level
    property bool startup: true         // is running the cold startup ?

    // Property to store the state of the application (active or suspended)
    property bool applicationSuspended: false

    // setting alias to check first run
    property alias firstRun: settings.firstRun

    // property to detect if the UI has finished
    property bool loadedUI: false
    property bool wideAspect: width >= units.gu(100) && loadedUI

    // Constants
    readonly property int queueBatchSize: 100
    readonly property real minSizeGU: 42

    Connections {
        target: Qt.application
        onStateChanged: {
            switch (Qt.application.state) {
            case Qt.ApplicationSuspended:
            case Qt.ApplicationInactive:
                if (!applicationSuspended) {
                    console.log("Application state changed to suspended");
                    applicationSuspended = true;
                }
                break;
            case Qt.ApplicationHidden:
            case Qt.ApplicationActive:
                if (applicationSuspended) {
                    console.log("Application state changed to active");
                    applicationSuspended = false;
                }
                break;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Application main view
    ////

    PopInfo {
        id: mainInfo
        anchors.fill: parent
        boxRadius: 0
        boxMargins: 0
        font.pixelSize: units.fx("medium");
        backgroundColor: styleMap.view.backgroundColor
        foregroundColor: styleMap.view.foregroundColor
    }

    ListModel {
        id: tabs
        ListElement { title: qsTr("Download Maps"); source: "qrc:/silica/MapDownloads.qml"; visible: true }
        ListElement { title: qsTr("Search Place"); source: "qrc:/silica/SearchPlace.qml"; visible: true }
        ListElement { title: qsTr("Favorite Places"); source: "qrc:/silica/Favorites.qml"; visible: true }
    }

    property var mapPage: null
    property var hillshadeProvider: null
    property int launcherMode: 0

    Component.onCompleted: {
        // setup hillshade provider
        try {
            var hsprovider = JSON.parse(HillshadeProvider);
            if (hsprovider.id)
                hillshadeProvider = hsprovider;
        } catch(e) {
            console.log("HillshadeProvider: " + e.name);
        }
        // setup Converter
        Osmin.Converter.meters = qsTr("meters");
        Osmin.Converter.km = qsTr("km");
        Osmin.Converter.feet = qsTr("feet");
        Osmin.Converter.miles = qsTr("miles");
        Osmin.Converter.north = qsTr("north");
        Osmin.Converter.south = qsTr("south");
        Osmin.Converter.west = qsTr("west");
        Osmin.Converter.east = qsTr("east");
        Osmin.Converter.northwest = qsTr("northwest");
        Osmin.Converter.northeast = qsTr("northeast");
        Osmin.Converter.southwest = qsTr("southwest");
        Osmin.Converter.southeast = qsTr("southeast");
        Osmin.Converter.system = settings.systemOfUnits;
        positionSource.active = true;
        launcher.start();
    }

    Timer {
        id: launcher
        interval: 500
        onTriggered: {
            if (launcherMode === 0)
                restart();
            else {
                pageStack.clear();
                mapPage = pageStack.push("qrc:/silica/MapView.qml");
                if (launcherMode === 1) {
                    var welcome = pageStack.push("qrc:/silica/Welcome.qml");
                    ToolBox.connectOnce(welcome.poppedAndNext, function(next){
                        if (next !== "") {
                            popAndPush.popped = welcome;
                            popAndPush.next = next;
                            popAndPush.start();
                        }
                    });
                }
                stop();
            }
        }
    }

    Timer {
        id: popAndPush
        interval: 50
        property Page popped: null
        property string next: ""
        onTriggered: {
            if (popped !== null && popped.status !== PageStatus.Inactive)
                restart();
            else {
                pageStack.push(next);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global requests
    ////

    // Add a new favorite
    function createFavorite(lat, lon, label, type) {
        var index = Osmin.FavoritesModel.append();
        var id = Osmin.FavoritesModel.data(index, Osmin.FavoritesModel.IdRole)
        Osmin.FavoritesModel.setData(index, lat, Osmin.FavoritesModel.LatRole);
        Osmin.FavoritesModel.setData(index, lon, Osmin.FavoritesModel.LonRole);
        Osmin.FavoritesModel.setData(index, 0.0, Osmin.FavoritesModel.AltRole);
        Osmin.FavoritesModel.setData(index, new Date(), Osmin.FavoritesModel.TimestampRole);
        Osmin.FavoritesModel.setData(index, label, Osmin.FavoritesModel.LabelRole);
        if (type) // optional
             Osmin.FavoritesModel.setData(index, type, Osmin.FavoritesModel.TypeRole);
        if (Osmin.FavoritesModel.storeData())
            return id;
        mainInfo.open(qsTr("Saving change failed"));
        Osmin.FavoritesModel.remove(id);
        return 0;
    }

    // Remove a favorite
    function removeFavorite(id) {
        Osmin.FavoritesModel.remove(id);
        if (Osmin.FavoritesModel.storeData())
            return true;
        mainInfo.open(qsTr("Saving change failed"));
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Dialog
    ////

    DialogAbout {
        id: dialogAbout
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Spinner
    ////

    property bool jobRunning: false

    BusyIndicator {
        id: spinner
        running: jobRunning
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Map
    ////

    Osmin.Settings {
        id: mapSettings
        //double   mapDPI
        //bool     onlineTiles
        //QString  onlineTileProviderId
        //bool     offlineMap
        //QString  styleSheetFile
        //bool     renderSea
        //QString  fontName
        //double   fontSize
        //bool     showAltLanguage
        //QString  units                    metrics|imperial
        offlineMap: true
        onlineTiles: false
    }

    MapPosition {
      id: positionSource
    }

    CompassSensor {
        id: compass
        active: false
        magneticDip: settings.magneticDip
        signal polled(real azimuth, real rotation)
        onAzimuthChanged: {
            if (!poll.running) poll.start();
        }
        Timer {
            id: poll
            interval: 500
            onTriggered: {
                compass.polled(compass.azimuth, (360 - compass.azimuth) * Math.PI / 180.0);
            }
        }
    }
}
