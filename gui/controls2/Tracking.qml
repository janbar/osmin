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
import Osmin 1.0
import "./components"

PopOver {
    id: tracking

    title: qsTr("Tracking")
    contents: Column {
        spacing: units.gu(1)

        Column {
            width: parent.width
            spacing: units.gu(1)
            Row {
                spacing: units.gu(2)
                Label {
                    text: qsTr("Average Speed")
                  font.pointSize: units.fs("medium")
                  color: styleMap.popover.highlightedColor
                }
                Label {
                    id: avgSpeed
                    text: Converter.readableSpeed(Tracker.duration < 1 ? 0 : 3.6 * Tracker.distance/Tracker.duration)
                    font.pointSize: units.fs("medium")
                    color: foregroundColor
                }
            }
            Row {
                spacing: units.gu(2)
                Label {
                    text: qsTr("Duration")
                  font.pointSize: units.fs("medium")
                  color: styleMap.popover.highlightedColor
                }
                Label {
                    id: duration
                    text: Converter.panelDurationHMS(Tracker.duration)
                    font.pointSize: units.fs("medium")
                    color: foregroundColor
                }
            }
            Row {
                spacing: units.gu(2)
                Label {
                    text: qsTr("Distance")
                  font.pointSize: units.fs("medium")
                  color: styleMap.popover.highlightedColor
                }
                Label {
                    id: distance
                    text: Converter.readableDistance(Tracker.distance)
                    font.pointSize: units.fs("medium")
                    color: foregroundColor
                }
            }
            Row {
                spacing: units.gu(2)
                Label {
                    text: qsTr("Ascent")
                  font.pointSize: units.fs("medium")
                  color: styleMap.popover.highlightedColor
                }
                Label {
                    id: ascent
                    text: Converter.readableDistance(Tracker.ascent)
                    font.pointSize: units.fs("medium")
                    color: foregroundColor
                }
            }
            Row {
                spacing: units.gu(2)
                Label {
                    text: qsTr("Descent")
                  font.pointSize: units.fs("medium")
                  color: styleMap.popover.highlightedColor
                }
                Label {
                    id: descent
                    text: Converter.readableDistance(Tracker.descent)
                    font.pointSize: units.fs("medium")
                    color: foregroundColor
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/reset.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Reset statistics")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    Tracker.reset();
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/record.svg"
                color: Tracker.recording !== "" ? "red" : foregroundColor
                height: units.gu(6)
                label.text: Tracker.recording !== "" ? qsTr("Cut track recording") : qsTr("Start track recording")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    Tracker.startRecording();
                }
                animationRunning: tracking.visible && Tracker.recording !== ""
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/save.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Close track recording")
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
    }
}
