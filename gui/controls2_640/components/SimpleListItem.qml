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
import "../"

MouseArea {
    id: area

    property color color: styleMap.view.backgroundColor
    property color colorAlt: styleMap.view.backgroundAltColor
    property color highlightedColor: styleMap.view.highlightedColor
    property bool highlighted: false
    property bool alternate: false

    property double paddingLeft: 0
    property double paddingRight: 0
    property alias column: row.column
    property alias action1: row.action1
    property alias action2: row.action2
    property alias menuVisible: row.menuVisible
    property alias menuItems: row.menuItems

    //anchors { left: parent.left; right: parent.right }
    //Qt5.15: Fix parent null on init
    Component.onCompleted: {
        anchors.left = parent.left;
        anchors.right = parent.right;
    }

    Rectangle {
        id: content
        anchors.fill: parent

        color: (area.alternate ? area.colorAlt : area.color)

        // highlight the current position
        Rectangle {
            anchors.fill: parent
            visible: area.highlighted
            color: area.highlightedColor
            opacity: 0.2
        }

        SimpleRow {
            id: row
            anchors.fill: parent
            anchors.leftMargin: paddingLeft
            anchors.rightMargin: paddingRight

            // Animate margin changes so it isn't noticible
            Behavior on anchors.leftMargin {
                NumberAnimation {

                }
            }

            Behavior on anchors.rightMargin {
                NumberAnimation {

                }
            }
        }
    }
}
