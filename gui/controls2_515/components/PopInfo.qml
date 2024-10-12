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
    property string message: ""
    property color backgroundColor: "red"
    property color foregroundColor: "white"
    property real edgeMargins: 0.0
    property real backgroundRadius: 0.0
    property real backgroundOpacity: 1.0
    property real contentEdgeMargins: units.gu(0.5)
    property alias font: label.font

    height: area.visible ? label.paintedHeight + 3 * contentEdgeMargins : 0
    z: 99

    MouseArea {
        id: area
        anchors.fill: parent
        visible: false
        enabled: visible
        anchors.topMargin: edgeMargins
        anchors.leftMargin: edgeMargins
        anchors.rightMargin: edgeMargins

        // background
        Rectangle {
            id: popinfo
            anchors.fill: parent
            color: backgroundColor
            opacity: backgroundOpacity
            radius: backgroundRadius
        }

        // message
        Label {
            id: label
            text: message
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:parent.left
            anchors.right: parent.right
            anchors.topMargin: contentEdgeMargins / 2
            anchors.bottomMargin: contentEdgeMargins / 2
            anchors.leftMargin: 2 * contentEdgeMargins
            anchors.rightMargin: 2 * contentEdgeMargins
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignJustify
            maximumLineCount: 4
            wrapMode: Text.Wrap
            color: foregroundColor
            font.pixelSize: units.fs("small")
            font.weight: Font.Normal
        }

        onClicked: {
            if (timer.running)
                timer.stop();
            visible = false;
        }
    }

    // auto closing
    Timer {
        id: timer
        interval: 5000
        onTriggered: {
            close();
        }
    }

    function open(msgtxt, bg, fg) {
        message = msgtxt;
        popinfo.color = (bg !== undefined ? bg : backgroundColor);
        label.color = (fg !== undefined ? fg : foregroundColor)
        if (area.visible) {
            timer.restart();
        } else {
            area.visible = true;
            timer.interval = 1000 * (5 + Math.floor(msgtxt.length / 10));
            timer.start()
        }
    }

    function close() {
        area.visible = false;
        message = "";
    }
}
