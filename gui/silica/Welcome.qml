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
import "./components"

MapPage {
    id: welcomePage
    objectName: "welcomePage"
    pageTitle: qsTr("Welcome")
    pageMenuEnabled: true

    signal poppedAndNext(var next)
    onPopped: poppedAndNext("")

    Component { id: menuItemComp; MenuItem { } }
    property MenuItem menuItemAbout: null

    Component.onCompleted: {
        // create the menu item to open the dialog about.
        menuItemAbout = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("About")});
        menuItemAbout.onClicked.connect(function(){
            dialogAbout.open();
        });
    }

    // Overlay to show when no map is installed
    Rectangle {
        id: downloadLauncher
        anchors {
            fill: parent
        }
        color: "transparent"

        Column {
            anchors {
                centerIn: parent
            }
            spacing: units.gu(4)
            width: parent.width > units.gu(44) ? parent.width - units.gu(8) : units.gu(36)

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                antialiasing: true
                fillMode: Image.PreserveAspectFit
                height: units.gu(10)
                smooth: true
                source: "qrc:/images/osmin.png"
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                color: styleMap.view.foregroundColor
                font.pixelSize: units.fx("medium")
                horizontalAlignment: Text.AlignHCenter
                text: "OSMin " + VersionString
            }

            Label {
                color: styleMap.view.foregroundColor
                elide: Text.ElideRight
                font.pixelSize: units.fx("large")
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 6
                text: qsTr("First of all to use OSMin, you need to download the map of your region. " +
                           "Online maps are not activated with this software, in order to better assist " +
                           "you in navigating outside the areas covered by a network.")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            MapIcon {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:/images/download.svg"
                color: styleMap.view.foregroundColor
                height: units.gu(6)
                label.text: qsTr("Download Maps")
                label.color: styleMap.view.foregroundColor
                label.font.pixelSize: units.fx("medium")
                onClicked: {
                    poppedAndNext("qrc:/silica/MapDownloads.qml");
                    pageStack.pop();
                }
            }
        }
    }
}
