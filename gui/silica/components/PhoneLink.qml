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
    property string phone: ""
    property color foregroundColor: styleMap.view.primaryColor
    property color highlightedColor: styleMap.view.highlightedColor

    visible: phone != ""
    height: visible ? row.implicitHeight : 0
    width: row.implicitWidth

    Row {
        id: row
        MapIcon {
            id: phoneIcon
            anchors.verticalCenter: parent.verticalCenter
            source: "qrc:/images/dialer.svg"
            color: foregroundColor
            width: units.gu(4)
            height: units.gu(4)
        }
        Label {
            id: phoneLabel
            anchors.verticalCenter: parent.verticalCenter
            text: phone
            color: highlightedColor
            elide: Text.ElideRight
            font.pixelSize: units.fx("medium")
        }
    }

    MouseArea {
        anchors.fill: row
        onClicked: Qt.openUrlExternally("tel:%1".arg(phone))
    }
}
