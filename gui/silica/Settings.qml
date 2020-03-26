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
import Sailfish.Silica 1.0
import QtQml.Models 2.2
import QtQuick.Layouts 1.1
import Osmin 1.0
import "./components"

MapPage {
    id: settingsPage
    pageTitle: qsTr("Settings")
    pageFlickable: body
    pageMenuEnabled: true

    onPopped: {
//        var needRestart = (settings.systemOfUnits !== Converter.system);
//        if (needRestart) {
//            mainView.jobRunning = true;
//            Qt.exit(16);
//        }
    }

    Component { id: menuItemComp; MenuItem { } }
    property MenuItem menuItemAbout: null
    property string mapsDirectory: ""

    Component.onCompleted: {
        // create the menu item to open the dialog about.
        menuItemAbout = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("About")});
        menuItemAbout.onClicked.connect(function(){
            dialogAbout.open();
        });
        // select current map directory
        if (MapsDirectories.length > 1)
            mapsDirectory = MapsDirectories[1]; // external storage
        else
            mapsDirectory = MapsDirectories[0]; // home maps store
    }

    SilicaFlickable {
        id: body
        anchors.fill: parent
        contentHeight: settingsColumn.implicitHeight

        Column {
            id: settingsColumn
            width: parent.width - units.gu(4)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: units.gu(1)

            ComboBox {
                id: unitsBox
                width: parent.width
                label: qsTr("System of Units")
                labelColor: styleMap.popover.foregroundColor
                leftMargin: 0
                rightMargin: 0
                menu: ContextMenu {
                    MenuItem { text: qsTr("SI") }
                    MenuItem { text: qsTr("Imperial") }
                }
                onCurrentItemChanged: {
                    settings.systemOfUnits = Converter.systems()[currentIndex]
                }
                Component.onCompleted: currentIndex = (settings.systemOfUnits === "Imperial" ? 1 : 0)
            }

            Label {
                text: qsTr("The change will be effective after restart.")
                font.pixelSize: units.fx("medium")
                color: "red"
                visible: settings.systemOfUnits !== Converter.system
                maximumLineCount: 2
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                text: qsTr("Data directory")
                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fx("medium")
            }
            Label {
                text: qsTr("%1 free").arg(Converter.readableBytes(Utils.storageBytesFree(DataDirectory)))
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fx("small")
            }
            Label {
                text: DataDirectory
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fx("small")
                maximumLineCount: 3
                width: parent.width
                wrapMode: Text.WrapAnywhere
            }
            Label {
                text: qsTr("Maps directory")
                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fx("medium")
            }
            Label {
                text: qsTr("%1 free").arg(Converter.readableBytes(Utils.storageBytesFree(mapsDirectory)))
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fx("small")
            }
            Label {
                text: mapsDirectory
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fx("small")
                maximumLineCount: 3
                width: parent.width
                wrapMode: Text.WrapAnywhere
            }
            Label {
                text: qsTr("Hillshade provider")
                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fx("medium")
            }
            Label {
                text: (hillshadeProvider != null ? hillshadeProvider.name : qsTr("Not configured"))
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fx("small")
            }
            Label {
                text: (hillshadeProvider != null ? hillshadeProvider.copyright : "")
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fx("small")
                maximumLineCount: 3
                width: parent.width
                wrapMode: Text.WrapAnywhere
            }
        }
    }
}
