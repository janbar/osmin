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

import QtCore
import QtQuick 2.9
import QtQuick.Window 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Controls.Universal 2.2
import Qt5Compat.GraphicalEffects 6.0
import Osmin 1.0 as Osmin
import "../toolbox.js" as ToolBox
import "./components"

ApplicationWindow {
    id: mainView
    visible: true

    // Design stuff
    width: 360
    height: 640

    property bool wideAspect: height < units.gu(64)

    // the azimuth magnetic should be flipped depending on screen orientation
    function flipAzimuth(dip) {
        var angle = 0.0;
        if (Android) { // tested only on android
            switch(screen.primaryOrientation) {
            case Qt.LandscapeOrientation:
                angle = -90.0; break;
            case Qt.InvertedLandscapeOrientation:
                angle = +90.0; break;
            case Qt.InvertedPortraitOrientation:
                angle = 180.0; break;
            case Qt.PortraitOrientation:
                angle = 0.0; break;
            default:
                break;
            }
        }
        console.log("flip azimuth: screen=" + angle + "deg , dip=" + dip + "deg");
        Osmin.Tracker.magneticDip = dip - angle;
    }

    Screen.onPrimaryOrientationChanged: flipAzimuth(settings.magneticDip)

    Settings {
        id: settings
        // General settings
        property string style: "Material"
        property int theme: 0
        property real scaleFactor: 1.0
        property real fontScaleFactor: 1.0
        property bool firstRun: true
        property int tabIndex: -1
        property int widthGU: Math.round(mainView.width / units.gridUnit)
        property int heightGU: Math.round(mainView.height / units.gridUnit)

        // Navigation settings
        property string systemOfUnits: "SI"
        property bool hillShadesEnabled: false
        property bool renderingTypeTiled: true
        property string lastVehicle: "car"
        property int maximumRouteStep: 255
        property int courseId: 0
        property real magneticDip: 0.0
        property string voiceName: ""
        property bool routeStepDelta: false

        // Tracker settings
        property string trackerRecording: ""

        // Extra settings
        property string styleFlags: "[]"
        property bool showFavorites: true
    }

    Material.accent: Material.Grey
    Universal.accent: "grey"

    //@FIXME: declare the property 'palette' that is missing in QtQuick.controls 2.2 (Qt-5.9)
    Item {
        id: palette
        property color base: {
            if (settings.style === "Material") {
                return Material.background
            } else if (settings.style === "Universal") {
                return Universal.background
            } else return "white"
        }
        property color text: {
            if (settings.style === "Material") {
                return Material.foreground
            } else if (settings.style === "Universal") {
                return Universal.foreground
            } else return "black"
        }
        property color highlight: "gray"
        property color shadow: "black"
        property color brightText: "dimgray"
        property color button: "darkgray"
        property color link: "green"
        property color toolTipBase: "black"
        property color toolTipText: "white"
    }

    StyleLight {
        id: styleMap
    }

    Universal.theme: settings.theme
    Material.theme: settings.theme

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

    // Constants
    readonly property int queueBatchSize: 100
    readonly property real minSizeGU: 44

    // show/hide favorite POIs
    property bool showFavorites: false

    minimumHeight: units.gu(minSizeGU)
    minimumWidth: units.gu(minSizeGU)

    Connections {
        target: Qt.application
        function onStateChanged() {
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

    header: Rectangle {
        id: mainToolBar
        Material.foreground: styleMap.view.foregroundColor
        Material.background: styleMap.view.backgroundColor
        height: units.gu(6)
        width: parent.width
        color: styleMap.view.backgroundColor
        visible: stackView.currentItem && stackView.currentItem.showHeader ? true : false

        state: "default"
        states: [
            State {
                name: "default"
            }
        ]

        RowLayout {
            spacing: 0
            anchors.fill: parent

            Item {
                width: units.gu(6)
                height: width

                MapIcon {
                    width: units.gu(5)
                    height: width
                    anchors.centerIn: parent
                    source: {
                        if (stackView.depth > 1) {
                            if (stackView.currentItem.isRoot)
                                "qrc:/images/go-previous.svg"
                            else
                                "qrc:/images/go-up.svg"
                        } else {
                            "qrc:/images/navigation-menu.svg"
                        }
                    }

                    onClicked: {
                        if (stackView.depth > 1) {
                            if (stackView.currentItem.isRoot) {
                                stackView.currentItem.popped();
                                stackView.pop();
                            } else
                                stackView.currentItem.goUpClicked();
                        } else {
                            //drawer.open();
                        }
                    }

                    visible: true
                    enabled: !mainView.jobRunning
                }
            }

            Label {
                id: titleLabel
                text: stackView.currentItem != null ? stackView.currentItem.pageTitle : ""
                font.pixelSize: units.fs("large")
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }

            Item {
                width: units.gu(6)
                height: width

                MapIcon {
                    width: units.gu(5)
                    height: width
                    anchors.centerIn: parent
                    source: "qrc:/images/home.svg"

                    onClicked: {
                        stackView.currentItem.popped();
                        stackView.pop()
                    }

                    visible: (stackView.currentItem != null && !stackView.currentItem.isRoot)
                    enabled: visible
                }
            }
        }

        PopInfo {
            id: mainInfo
            anchors.fill: parent
            backgroundRadius: 0
            edgeMargins: 0
            font.pixelSize: units.fs("medium");
            backgroundColor: styleMap.view.backgroundColor
            foregroundColor: styleMap.view.foregroundColor
        }
    }

    property alias stackView: stackView

    StackView {
        id: stackView
        anchors {
            bottom: parent.bottom
            fill: undefined
            left: parent.left
            right: parent.right
            top: parent.top
        }
        initialItem: "qrc:/controls2/Banner.qml"
        pushEnter: Transition { }
        pushExit: Transition { }
        popEnter: Transition { }
        popExit: Transition { }
    }

    property var mapPage: null
    property var hillshadeProvider: null
    property int bonjour: 0

    Component.onCompleted: {
        // resize main view according to user settings
        if (!Android) {
            mainView.width = (settings.widthGU >= minSizeGU ? units.gu(settings.widthGU) : units.gu(minSizeGU));
            mainView.height = (settings.heightGU >= minSizeGU ? units.gu(settings.heightGU) : units.gu(minSizeGU));
        }
        // dump map settings
        console.log("Settings: devDPI=" + mapSettings.physicalDPI.toFixed(0))
        console.log("Settings: mapDPI=" + mapSettings.mapDPI.toFixed(0))
        console.log("Settings: fontName=" + mapSettings.fontName)
        console.log("Settings: fontSize=" + mapSettings.fontSize.toFixed(1))
        console.log("Settings: units=" + mapSettings.units)
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

        // reset magnetic dip according to the screen orientation
        flipAzimuth(settings.magneticDip);

        showFavorites = settings.showFavorites;
        positionSource.active = true;
        launcher.start();
    }

    // The initial stacked page (banner) will set the status of installed maps
    // Don't use a signal for that, but loop until the change.
    Timer {
        id: launcher
        interval: 500
        onTriggered: {
            if (bonjour === 0) {
                restart();
            } else {
                stackView.clear();
                mapPage = stackView.push("qrc:/controls2/MapView.qml");
                if (bonjour === 1) {
                    stackView.push("qrc:/controls2/Welcome.qml");
                }
                stop();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global keyboard shortcuts
    ////

    // Child can connect this signal to handle the event
    signal keyBackPressed

    Timer {
        id: postponeKeyBackPressed
        interval: 100
        onTriggered: keyBackPressed()
    }

    // On android catch the key PgUp, which is returned by the callback
    // On other platform catch the standard key 'ESC'
    Shortcut {
        sequences: ["Esc", StandardKey.MoveToPreviousPage]
        onActivated: {
            if (stackView.depth > 1) {
                if (stackView.currentItem.isRoot)
                    stackView.pop();
                else
                    stackView.currentItem.goUpClicked();
            } else {
                 postponeKeyBackPressed.start();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global requests
    ////

    // Add a new favorite and return its id
    function createFavorite(lat, lon, label, type) {
        var id = Osmin.FavoritesModel.append(lat, lon, label, type);
        if (id > 0) {
            if (Osmin.FavoritesModel.storeData())
                return id;
            Osmin.FavoritesModel.remove(id); // rollback
        } else {
            console.log("Failed to append new favorite: count " + Osmin.FavoritesModel.count + " item(s)");
        }
        mainInfo.open(qsTr("Saving change failed"));
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

    MapVoice {
        id: mapVoice
        Component.onCompleted: {
            // bind the setting
            settings.voiceName = Qt.binding(function foo(){return voiceName;});
        }
    }
}
