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
import QtQml 2.2
import QtQml.Models 2.3
import QtQuick.Layouts 1.3
import Osmin 1.0
import "./components"

MapPage {
    id: settingsPage
    pageTitle: qsTr("Settings")
    pageFlickable: body

    onPopped: {
//        var needRestart = (settings.systemOfUnits !== Converter.system);
//        if (needRestart) {
//            mainView.jobRunning = true;
//            Qt.exit(16);
//        }
    }

    property string mapsDirectory: ""
    Component.onCompleted: {
        if (MapsDirectories.length > 1)
            mapsDirectory = MapsDirectories[1]; // external storage
        else
            mapsDirectory = MapsDirectories[0]; // home maps store
    }

    Flickable {
        id: body
        anchors.fill: parent
        contentHeight: settingsColumn.implicitHeight
        ScrollBar.vertical: ScrollBar {}

        Column {
            id: settingsColumn
            width: parent.width - units.gu(4)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: units.gu(1)

            RowLayout {
                spacing: 0
                MapIcon {
                    height: units.gu(6)
                    source: "qrc:/images/font-scalling.svg"
                    hoverEnabled: false
                }
                SpinBox {
                    id: fontScaleBox
                    enabled: !Android
                    from: 50
                    value: settings.fontScaleFactor * 100
                    to: 150
                    stepSize: 10
                    font.pixelSize: units.fs("medium");
                    Layout.fillWidth: true

                    property int decimals: 2
                    property real realValue: value / 100
                    property real acceptedValue: settings.fontScaleFactor

                    validator: DoubleValidator {
                        bottom: Math.min(fontScaleBox.from, fontScaleBox.to)
                        top:  Math.max(fontScaleBox.from, fontScaleBox.to)
                    }

                    textFromValue: function(value, locale) {
                        return Number(value / 100).toLocaleString(locale, 'f', fontScaleBox.decimals)
                    }

                    valueFromText: function(text, locale) {
                        return Number.fromLocaleString(locale, text) * 100
                    }

                    onValueModified: {
                        settings.fontScaleFactor = realValue
                    }
                }
            }

            RowLayout {
                spacing: 0
                MapIcon {
                    height: units.gu(6)
                    source: "qrc:/images/graphic-scalling.svg"
                    hoverEnabled: false
                }
                SpinBox {
                    id: scaleBox
                    enabled: !Android
                    from: 50
                    value: settings.scaleFactor * 100
                    to: 400
                    stepSize: 10
                    font.pixelSize: units.fs("medium");
                    Layout.fillWidth: true

                    property int decimals: 2
                    property real realValue: value / 100
                    property real acceptedValue: settings.scaleFactor

                    validator: DoubleValidator {
                        bottom: Math.min(scaleBox.from, scaleBox.to)
                        top:  Math.max(scaleBox.from, scaleBox.to)
                    }

                    textFromValue: function(value, locale) {
                        return Number(value / 100).toLocaleString(locale, 'f', scaleBox.decimals)
                    }

                    valueFromText: function(text, locale) {
                        return Number.fromLocaleString(locale, text) * 100
                    }

                    onValueModified: {
                        mainView.width = Math.round(realValue * mainView.width / settings.scaleFactor);
                        mainView.height = Math.round(realValue * mainView.height / settings.scaleFactor);
                        settings.scaleFactor = realValue
                    }
                }
            }

            RowLayout {
                spacing: units.gu(1)
                Layout.fillWidth: true
                Label {
                    text: qsTr("Theme")
                    font.pixelSize: units.fs("medium");
                }
                ComboBox {
                    id: themeBox
                    flat: true
                    property int acceptedValue: settings.theme
                    model: [
                        qsTr("Light"),
                        qsTr("Dark")
                    ]

                    currentIndex: settings.theme
                    onActivated: {
                        settings.theme = index
                    }

                    Layout.fillWidth: true
                    font.pixelSize: units.fs("medium");
                    Component.onCompleted: {
                        popup.font.pixelSize = units.fs("medium");
                    }
                }
            }

            RowLayout {
                spacing: units.gu(1)
                Layout.fillWidth: true
                Label {
                    text: qsTr("System of Units")
                    font.pixelSize: units.fs("medium");
                }
                ComboBox {
                    id: unitsBox
                    flat: true
                    model: [
                        qsTr("SI"),
                        qsTr("Imperial")
                    ]
                    property string selected: Converter.system
                    currentIndex: (settings.systemOfUnits === "Imperial" ? 1 : 0)
                    Layout.fillWidth: true
                    font.pixelSize: units.fs("medium");
                    onActivated: {
                        settings.systemOfUnits = Converter.systems()[index]
                    }
                    Component.onCompleted: {
                        popup.font.pixelSize = units.fs("medium");
                    }
                }
            }

            RowLayout {
                spacing: 0
                MapIcon {
                    height: units.gu(6)
                    source: "qrc:/images/compass.svg"
                    hoverEnabled: false
                    rotation: (-1.0) * magdipBox.realValue
                }
                SpinBox {
                    id: magdipBox
                    from: -300
                    value: settings.magneticDip * 10
                    to: 300
                    stepSize: 10
                    font.pixelSize: units.fs("medium");
                    Layout.fillWidth: true

                    property int decimals: 1
                    property real realValue: 0.1 * value

                    validator: DoubleValidator {
                        bottom: Math.min(magdipBox.from, magdipBox.to)
                        top:  Math.max(magdipBox.from, magdipBox.to)
                    }

                    textFromValue: function(value, locale) {
                        return Number(0.1 * value).toLocaleString(locale, 'f', magdipBox.decimals)
                    }

                    valueFromText: function(text, locale) {
                        return Number.fromLocaleString(locale, text) * 10
                    }

                    onValueModified: {
                        // save settings
                        settings.magneticDip = realValue;
                        // setup tracker
                        Tracker.magneticDip = realValue;
                    }
                }
            }

            Label {
                text: qsTr("The change will be effective after restart.")
                font.pixelSize: units.fs("medium")
                color: "red"
                visible: settings.systemOfUnits !== Converter.system
                maximumLineCount: 2
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                text: qsTr("Data directory")
                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fs("medium")
            }
            Label {
                text: qsTr("%1 free").arg(Converter.readableBytes(Utils.storageBytesFree(DataDirectory)))
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fs("small")
            }
            Label {
                text: DataDirectory
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fs("small")
                maximumLineCount: 3
                width: parent.width
                wrapMode: Text.WrapAnywhere
            }
            Label {
                text: qsTr("Maps directory")
                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fs("medium")
            }
            Label {
                text: qsTr("%1 free").arg(Converter.readableBytes(Utils.storageBytesFree(mapsDirectory)))
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fs("small")
            }
            Label {
                text: mapsDirectory
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fs("small")
                maximumLineCount: 3
                width: parent.width
                wrapMode: Text.WrapAnywhere
            }
            Label {
                text: qsTr("Hillshade provider")
                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fs("medium")
            }
            Label {
                text: (hillshadeProvider != null ? hillshadeProvider.name : qsTr("Not configured"))
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fs("small")
            }
            Label {
                text: (hillshadeProvider != null ? hillshadeProvider.copyright : "")
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fs("small")
                maximumLineCount: 3
                width: parent.width
                wrapMode: Text.WrapAnywhere
            }
        }
    }
}
