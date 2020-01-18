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

Item {
    property alias column: columnComponent.sourceComponent
    property alias action1: action1
    property alias action2: action2

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
        anchors.right: parent.right
        anchors.rightMargin: units.gu(2)
        anchors.verticalCenter: parent.verticalCenter
        visible: false
        width: visible ? units.gu(5) : 0
        height: width
    }
}

