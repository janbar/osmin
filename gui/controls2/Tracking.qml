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
                    text: Converter.readableSpeed(tracker.duration < 1 ? 0 : 3.6 * tracker.distance/tracker.duration)
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
                    text: Converter.panelDurationHMS(tracker.duration)
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
                    text: Converter.panelDistance(tracker.distance)
                    font.pointSize: units.fs("medium")
                    color: foregroundColor
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/edit-clear.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Reset tracking")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    tracking.close();
                    tracker.reset();
                }
            }
        }

    }
}
