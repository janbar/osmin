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
import Osmin 1.0
import "./components"

PopOver {
    id: tracking

    title: qsTr("Tracking")
    contents: Column {
        spacing: units.gu(2)

        Column {
            width: parent.width
            spacing: units.gu(1)
            Row {
                width: parent.width
                spacing: units.gu(2)
                Label {
                    width: parent.width / 2
                    text: qsTr("Average Speed")
                    font.pointSize: units.fs("medium")
                    color: styleMap.popover.highlightedColor
                }
                Label {
                    width: parent.width / 2
                    text: qsTr("Maximum Speed")
                    font.pointSize: units.fs("medium")
                    color: styleMap.popover.highlightedColor
                }
            }
            Row {
                width: parent.width
                spacing: units.gu(2)
                Label {
                    id: avgSpeed
                    width: parent.width / 2
                    text: Converter.readableSpeed(Tracker.duration < 1 ? 0 : 3.6 * Tracker.distance/Tracker.duration)
                    font.pointSize: units.fs("large")
                    color: foregroundColor
                }
                Label {
                    id: maxSpeed
                    width: parent.width / 2
                    text: Converter.readableSpeed(Tracker.maxSpeed)
                    font.pointSize: units.fs("large")
                    color: foregroundColor
                }
            }
            Row {
                width: parent.width
                spacing: units.gu(2)
                Label {
                    width: parent.width / 2
                    text: qsTr("Distance")
                    font.pointSize: units.fs("medium")
                    color: styleMap.popover.highlightedColor
                }
                Label {
                    width: parent.width / 2
                    text: qsTr("Duration")
                    font.pointSize: units.fs("medium")
                    color: styleMap.popover.highlightedColor
                }
            }
            Row {
                width: parent.width
                spacing: units.gu(2)
                Label {
                    id: distance
                    width: parent.width / 2
                    text: Converter.readableDistance(Tracker.distance)
                    font.pointSize: units.fs("large")
                    color: foregroundColor
                }
                Label {
                    id: duration
                    width: parent.width / 2
                    text: Converter.panelDurationHMS(Tracker.duration)
                    font.pointSize: units.fs("large")
                    color: foregroundColor
                }
            }
            Row {
                width: parent.width
                spacing: units.gu(2)
                Label {
                    width: parent.width / 2
                    text: qsTr("Ascent")
                    font.pointSize: units.fs("medium")
                    color: styleMap.popover.highlightedColor
                }
                Label {
                    width: parent.width / 2
                    text: qsTr("Descent")
                    font.pointSize: units.fs("medium")
                    color: styleMap.popover.highlightedColor
                }
            }
            Row {
                width: parent.width
                spacing: units.gu(2)
                Label {
                    id: ascent
                    width: parent.width / 2
                    text: Converter.readableDistance(Tracker.ascent)
                    font.pointSize: units.fs("large")
                    color: foregroundColor
                }
                Label {
                    id: descent
                    width: parent.width / 2
                    text: Converter.readableDistance(Tracker.descent)
                    font.pointSize: units.fs("large")
                    color: foregroundColor
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/record.svg"
                color: Tracker.recording !== "" ? "red" : foregroundColor
                height: units.gu(6)
                label.text: "  " + (Tracker.recording !== "" ? qsTr("Cut track recording") : qsTr("Start track recording"))
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    Tracker.startRecording();
                }
                enabled: !Tracker.processing
                animationRunning: Tracker.processing
                animationInterval: 200
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/save.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Close track recording")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    Tracker.stopRecording();
                }
                enabled: Tracker.recording !== ""
                opacity: enabled ? 1.0 : 0.5
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/reset.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Reset statistics")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    Tracker.reset();
                }
                enabled: Tracker.recording === ""
                opacity: enabled ? 1.0 : 0.5
            }
        }

        Column {
            Label {
                text: qsTr("Coordinates")
                color: styleMap.popover.highlightedColor
                font.pointSize: units.fs("medium")
            }
            Label {
                text: Converter.readableCoordinatesGeocaching(positionSource._lat, positionSource._lon)
                font.pointSize: units.fs("large")
                color: foregroundColor
            }
        }
        Column {
            Label {
                text: qsTr("Elevation")
                color: styleMap.popover.highlightedColor
                font.pointSize: units.fs("medium")
            }
            Label {
                text: Converter.readableElevation(positionSource._alt)
                font.pointSize: units.fs("large")
                color: foregroundColor
            }
        }
        Column {
            Label {
                text: qsTr("Bearing")
                color: styleMap.popover.highlightedColor
                font.pointSize: units.fs("medium")
            }
            Label {
                text: Converter.readableDegreeGeocaching(180.0 * Tracker.bearing / Math.PI)
                font.pointSize: units.fs("large")
                color: foregroundColor
            }
        }
    }
}
