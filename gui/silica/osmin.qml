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
import Osmin 1.0 as MapEngine
import "components"

ApplicationWindow {
    id: mainView

    initialPage: Component {
        MapView {
        }
    }

    //cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    ConfigurationGroup {
        id: settings
        path: "/io/github/janbar/osmin"
        synchronous: true

        property real scaleFactor: 1.0
        property real fontScaleFactor: 1.0
        property bool firstRun: true
        property int tabIndex: -1
    }

    StyleLight {
        id: styleMap
    }

    Units {
        id: units
        scaleFactor: 1.0
        fontScaleFactor: 1.0
    }

    // Variables
    property string appName: "Mappy"    // My name
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

    ListModel {
        id: tabs
        ListElement { title: qsTr("Map View"); source: "qrc:/silica/MapView.qml"; visible: true }
        ListElement { title: qsTr("Download Maps"); source: "qrc:/silica/MapDownloads.qml"; visible: true }

        function initialIndex() {
            return (settings.tabIndex === -1 ? 0
            : settings.tabIndex > tabs.count - 1 ? tabs.count - 1
            : settings.tabIndex);
        }
    }

    Component.onCompleted: {
        // setup Converter
        MapEngine.Converter.meters = qsTr("meters");
        MapEngine.Converter.km = qsTr("km");
        MapEngine.Converter.feet = qsTr("feet");
        MapEngine.Converter.miles = qsTr("miles");
        MapEngine.Converter.north = qsTr("north");
        MapEngine.Converter.south = qsTr("south");
        MapEngine.Converter.west = qsTr("west");
        MapEngine.Converter.east = qsTr("east");
        MapEngine.Converter.northwest = qsTr("northwest");
        MapEngine.Converter.northeast = qsTr("northeast");
        MapEngine.Converter.southwest = qsTr("southwest");
        MapEngine.Converter.southeast = qsTr("southeast");
        MapEngine.Converter.system = "SI";
        positionSource.active = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Dialog
    ////

    /*DialogAbout {
        id: dialogAbout
    }

    DialogSettings {
        id: dialogSettings
    }*/

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Spinner
    ////

    property bool jobRunning: false

    ActivitySpinner {
        id: spinner
        visible: jobRunning
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Map
    ////

    MapEngine.Settings {
        id: mapEngineSettings
        //double   mapDPI
        //bool     onlineTiles
        //QString  onlineTileProviderId
        //bool     offlineMap
        //bool     renderSea
        //QString  styleSheetDirectory
        //QString  styleSheetFile
        //QString  fontName
        //double   fontSize
        //bool     showAltLanguage
        //QString  units                    metrics|imperial
        onlineTiles: false
        offlineMap: true
    }

    QtObject {
        id: mapUserSettings
        property bool rotateEnabled: false
        property bool hillShadesEnabled: false
        property bool renderingTypeTiled: false
        property string lastVehicle: "car"
        property int maximumRouteStep: 255
    }

    MapPosition {
      id: positionSource
    }
}
