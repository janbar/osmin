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
import QtQml 2.2
import QtQml.Models 2.3
import Osmin 1.0

Item {
    id: routeOverview

    property RoutingListModel routingModel

    ListView {
        id: stepsView
        anchors.fill: parent
        spacing: units.gu(1)
        model: routingModel
        clip: true

        delegate: Row {
            spacing: units.gu(2)
            width: parent.width
            height: Math.max(stepInfo.implicitHeight, icon.height)

            WAYIcon {
                id: icon
                anchors.verticalCenter: parent.verticalCenter
                color: styleMap.popover.foregroundColor
                stepType: model.type
                roundaboutExit: model.roundaboutExit
                roundaboutClockwise: model.roundaboutClockwise
                width: units.gu(7)
                height: width
            }
            Column {
                id: stepInfo
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - icon.width - units.gu(3)
                Label {
                    id: distance
                    width: parent.width
                    color: styleMap.popover.highlightedColor
                    text: model.distance === undefined ? "" : Converter.readableDistance(model.distance)
                    font.pointSize: units.fs("medium")
                    horizontalAlignment: Label.AlignRight
                }
                Label {
                    id: entryDescription
                    width: parent.width
                    color: styleMap.popover.foregroundColor
                    text: model.description
                    font.pointSize: units.fs("small")
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}
