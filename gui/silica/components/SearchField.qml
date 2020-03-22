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
import QtQuick.Layouts 1.1

Rectangle {
    id: filter
    visible: true
    height: visible ? units.gu(8) : 0
    color: "transparent"

    property alias displayText: field.text
    property alias fieldFocus: field.focus

    function forceActiveFocus() {
        field.forceActiveFocus();
    }

    function fill(text) {
        field.text = text;
    }

    function clear() {
        field.text = "";
    }

    signal accepted(string text)

    RowLayout {
        spacing: 0
        anchors.fill: parent
        anchors.leftMargin: units.gu(2)
        anchors.rightMargin: units.gu(1)

        TextField {
            id: field
            Layout.fillWidth: true
            font.pixelSize: units.fx("large")
            inputMethodHints: Qt.ImhNoPredictiveText
            placeholderText: qsTr("Search")
            EnterKey.enabled: true
            EnterKey.onClicked: {
                filter.accepted(displayText.trim());
            }
        }
        Item {
            anchors.verticalCenter: parent.verticalCenter
            width: units.gu(6)
            height: width

            MapIcon {
                width: units.gu(5)
                height: width
                anchors.centerIn: parent
                source: "qrc:/images/edit-clear.svg"
                onClicked: {
                    field.text = "";
                }
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            field.enabled = true;
            field.forceActiveFocus();
        } else {
            field.text = "";
            field.enabled = false;
        }
    }
}
