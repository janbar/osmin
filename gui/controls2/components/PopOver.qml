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
import "../../toolbox.js" as ToolBox

Item {
    property string title: ""
    property color backgroundColor: styleMap.popover.backgroundColor
    property color foregroundColor: styleMap.popover.foregroundColor
    property real edgeMargins: 0.0
    property real backgroundRadius: 0.0
    property real backgroundOpacity: 1.0
    readonly property real contentTopMargin: units.gu(7) // title bar + 1
    property real contentEdgeMargins: units.gu(2)
    property real contentSpacing: units.gu(1)
    readonly property real minimumHeight: units.gu(7)

    signal show
    signal close // triggered on button close pressed

    property bool keyBackEnabled: true

    // qmlproperty list<Object> contents
    // Content will be put inside a column in the foreround of the pop.
    default property alias contents: contentsColumn.data

    z: 99
    focus: true
    height: minimumHeight

    // background
    Rectangle {
        id: popover
        anchors.fill: parent
        anchors.margins: edgeMargins
        color: backgroundColor
        opacity: backgroundOpacity
        radius: backgroundRadius
    }

    Flickable {
        id: body
        anchors.fill: popover
        anchors.topMargin: contentTopMargin
        anchors.leftMargin: contentEdgeMargins
        anchors.rightMargin: contentEdgeMargins
        anchors.bottomMargin: contentEdgeMargins
        contentHeight: contentsColumn.height
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        Column {
            id: contentsColumn
            spacing: contentSpacing
            width: parent.width
            onWidthChanged: updateChildrenWidths();

            onChildrenChanged: updateChildrenWidths()

            function updateChildrenWidths() {
                for (var i = 0; i < children.length; i++) {
                    children[i].width = contentsColumn.width;
                }
            }
        }
    }

    // title
    Text {
        anchors.verticalCenter: closeButton.verticalCenter
        anchors.left: popover.left
        anchors.right: popover.right
        anchors.leftMargin: closeButton.width
        anchors.rightMargin: closeButton.width
        horizontalAlignment: Text.AlignHCenter
        text: title
        font.pointSize: units.fs("large")
        color: foregroundColor
        wrapMode: Text.Wrap
        visible: (text !== "")
    }

    // close button
    MapIcon {
        id: closeButton
        source: "qrc:/images/close.svg"
        height: units.gu(5)
        anchors.top: popover.top
        anchors.right: popover.right
        anchors.rightMargin: units.gu(0.5)
        anchors.topMargin: units.gu(0.5)
        anchors.bottomMargin: units.gu(0.5)
        color: foregroundColor
        onClicked: {
            console.log("close popover");
            close();
        }
    }

    function onKeyBack() {
        console.log("onKeyBack: close " + title);
        close();
    }
    onShow: {
        if (keyBackEnabled)
            ToolBox.connectOnce(mainView.keyBackPressed, onKeyBack);
    }
}
