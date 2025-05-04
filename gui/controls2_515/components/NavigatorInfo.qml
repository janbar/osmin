/*
 * Copyright (C) 2021
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
import Osmin 1.0

Item {
    id: navigationInfo
    property Navigator navigator

    property color backgroundColor: styleMap.popover.backgroundColor
    property color foregroundColor: styleMap.popover.foregroundColor
    property real edgeMargins: 0.0
    property real backgroundOpacity: 1.0
    property real contentEdgeMargins: units.gu(1)
    property real contentSpacing: units.gu(1)
    readonly property real headerHeight: units.gu(6)
    readonly property real minimumHeight: contentsColumn.height + headerHeight

    signal show
    signal close // triggered on button close pressed

    z: 99
    focus: true
    height: minimumHeight

    function minimize() {
        height = Qt.binding(function(){ return minimumHeight; });
    }

    function maximize() {
        height = Qt.binding(function(){ return parent.height; });
    }

    onVisibleChanged: {
        if (visible) {
            minimize();
        }
    }

    states: [
        State {
            name: "running"
        },
        State {
            name: "suspended"
        }
    ]
    state: "running"

    Connections {
        target: navigator
        function onStarted() {
            state = "running"
        }
        function onRerouteRequested() {
            navigator.suspended = true;
            state = "suspended"
        }
        function onTargetReached() {
            navigator.suspended = true;
            state = "suspended"
        }
        function onNextRouteStepChanged() {
            switch(state) {
            case "running":
                if (overview.active && overview.status === Loader.Ready) {
                    overview.item.show(navigator.nextRouteStep);
                }
                break;
            default:
                break;
            }
        }
    }

    // background
    Rectangle {
        id: popover
        anchors.fill: parent
        anchors.margins: edgeMargins
        color: backgroundColor
        opacity: backgroundOpacity
    }

    // content box
    Rectangle {
        id: contentsBox
        anchors.top: popover.top
        anchors.topMargin: navigationInfo.headerHeight
        anchors.left: popover.left
        anchors.right: popover.right
        height: navigationInfo.minimumHeight - navigationInfo.headerHeight
        color: styleMap.view.highlightedColor
        opacity: overview.active ? 0.2 : 0.0
    }

    Flickable {
        id: contents
        anchors.fill: contentsBox
        anchors.leftMargin: contentEdgeMargins
        anchors.rightMargin: contentEdgeMargins
        contentHeight: contentsColumn.height
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        Column {
            id: contentsColumn
            spacing: 0
            width: parent.width
            Row {
                id: rowStep
                opacity: navigationInfo.state === "running" ? 1.0 : 0.0
                spacing: units.gu(1)
                width: parent.width
                height: Math.max(distanceToNextStep.implicitHeight + contentSpacing + nextStepDescription.implicitHeight, icon.height)

                WAYIcon {
                    id: icon
                    anchors.verticalCenter: parent.verticalCenter
                    color: styleMap.popover.foregroundColor
                    stepType: navigator.nextRouteStep.type
                    roundaboutExit: navigator.nextRouteStep.roundaboutExit
                    roundaboutClockwise: navigator.nextRouteStep.roundaboutClockwise
                    width: units.gu(8)
                    height: width
                    label.text: navigator.nextRouteStep.type === "leave-roundabout" ? navigator.nextRouteStep.roundaboutExit : ""
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: contentSpacing
                    Label {
                        id: distanceToNextStep
                        text: Converter.readableDistance(navigator.nextRouteStep.distanceTo)
                        color: styleMap.popover.foregroundColor
                        font.pixelSize: units.fs("large")
                    }
                    Text {
                        id: nextStepDescription
                        text: navigator.nextRouteStep.shortDescription
                        font.pixelSize: units.fs("large")
                        color: styleMap.popover.foregroundColor
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }

    BusyIndicator{
        id: reroutingIndicator
        running: navigator.routeRunning
        height: units.gu(7)
        anchors.verticalCenter: contentsBox.verticalCenter
        anchors.right: contentsBox.right
        visible: running
    }

    Item {
        id: header
        anchors.verticalCenter: closeButton.verticalCenter
        anchors.left: popover.left
        anchors.right: closeButton.left
        anchors.leftMargin: contentEdgeMargins
        anchors.rightMargin: contentEdgeMargins
        height: headerHeight
        Row {
            id: headInfo
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            spacing: 0

            Label {
                id: speed
                anchors.verticalCenter: parent.verticalCenter
                text: Converter.readableSpeed(navigator.currentSpeed > 0 ? navigator.currentSpeed : 0.0)
                color: (navigator.maximumSpeed > 0 && navigator.maximumSpeed < (navigator.currentSpeed - 10.0)
                        ? "red" : styleMap.popover.foregroundColor)
                font.pixelSize: units.fs("x-large")
            }
            MapIcon {
                anchors.verticalCenter: parent.verticalCenter
                source: {
                    switch(navigator.vehicle) {
                    case "bicycle":
                        return "qrc:/images/trip/bike.svg"
                    case "foot":
                        return "qrc:/images/trip/walk.svg"
                    default:
                        return "qrc:/images/trip/car.svg"
                    }
                }
                height: units.gu(5)
                width: height
                color: styleMap.popover.foregroundColor
                enabled: false
            }
            Label {
                id: eta
                anchors.verticalCenter: parent.verticalCenter
                text: navigator.arrivalEstimate + " ~ " + Converter.panelDistance(navigator.remainingDistance)
                color: styleMap.popover.foregroundColor
                font.pixelSize: units.fs("large")
            }
        }
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
            console.log("close navigator info");
            if (overview.active)
                overview.active = false;
            close();
        }
    }

    // show/hide overview
    MouseArea {
        id: showHideOverview
        anchors.fill: contentsBox
        onClicked: {
            if (!overview.active) {
                navigationInfo.maximize();
                overview.active = true;
            } else {
                overview.active = false;
                navigationInfo.minimize();
            }
        }
    }

    MapIcon {
        id: resume
        source: "qrc:/images/trip/route.svg"
        height: units.gu(7)
        anchors.centerIn: contentsBox
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: contentEdgeMargins
        anchors.rightMargin: contentEdgeMargins
        color: styleMap.popover.foregroundColor
        onClicked: {
            navigator.suspended = false;
            navigator.reroute();
            navigationInfo.state = "running";
        }
        label.text: qsTr("Resume navigation")
        visible: navigationInfo.state === "suspended"
        enabled: visible
    }

    Loader {
        id: overview
        active: false
        height: navigationInfo.height - navigationInfo.headerHeight - contentsBox.height;
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contentsBox.bottom
        asynchronous: true
        sourceComponent: RouteOverview {
            routingModel: navigator.routingModel
            anchors.fill: overview
        }
        onLoaded: {
            item.show(navigator.nextRouteStep);
        }
    }
}
