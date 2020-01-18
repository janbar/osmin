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

Item {
    property alias column: columnComponent.sourceComponent
    property alias action1: action1
    property alias action2: action2
    property bool menuVisible: false
    property alias menuItems: optionsMenu.contentData

    anchors.fill: parent

    Loader {
        id: columnComponent
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: action2.left
        }
        width: parent.width - parent.spacing

        onSourceComponentChanged: {
            for (var i=0; i < item.children.length; i++) {
                // binds to width so it is updated when screen size changes
                item.children[i].width = Qt.binding(function () { return width; })
            }
        }
    }

    MapIcon {
        id: action2
        anchors.right: action1.left
        anchors.verticalCenter: parent.verticalCenter
        visible: false
        width: visible ? units.gu(5) : 0
        height: width
    }

    MapIcon {
        id: action1
        anchors.right: menu.left
        anchors.rightMargin: units.gu(1)
        anchors.verticalCenter: parent.verticalCenter
        visible: false
        width: visible ? units.gu(5) : 0
        height: width
    }

    MapIcon {
        id: menu
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        anchors.verticalCenter: parent.verticalCenter
        visible: menuVisible
        width: visible ? units.gu(5) : 0
        height: width
        source: "qrc:/images/contextual-menu.svg"
        onClicked: optionsMenu.open()

        Menu {
            id: optionsMenu
            width: implicitWidth * units.scaleFactor
            x: parent.width - width
            transformOrigin: Menu.TopRight
        }
    }
}

