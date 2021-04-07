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

import QtQuick 2.2
import QtQml 2.2
import QtQml.Models 2.3
import Sailfish.Silica 1.0
import Osmin 1.0

Item {
    id: routeOverview

    property RoutingListModel routingModel

    function show(step) {
        var d = step.distance;
        for (var i = 0; i < routingModel.count; ++i) {
            if (routingModel.get(i).distance === d) {
                stepsView.thisStep = i;
                break;
            }
        }
    }

    SilicaListView {
        id: stepsView
        width: parent.width
        height: parent.height //contentHeight
        interactive: true
        spacing: units.gu(1)
        model: routingModel
        clip: true

        property int thisStep: 0

        onThisStepChanged: {
            positionViewAtIndex(thisStep, ListView.Beginning);
        }

        delegate: Row {
            spacing: units.gu(2)
            width: parent.width
            height: Math.max(stepInfo.implicitHeight, icon.height)

            WAYIcon {
                id: icon
                anchors.verticalCenter: parent.verticalCenter
                color: (index === stepsView.thisStep ? styleMap.popover.highlightedColor : styleMap.popover.foregroundColor)
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
                    color: styleMap.popover.foregroundColor
                    text: Converter.panelDistance(model.distance) + " ~ " + Converter.panelDurationHM(model.time)
                    font.pixelSize: units.fx("small")
                    horizontalAlignment: Label.AlignRight
                }
                Label {
                    id: entryDescription
                    width: parent.width
                    color: (index === stepsView.thisStep ? styleMap.popover.highlightedColor : styleMap.popover.foregroundColor)
                    text: model.description
                    font.pixelSize: units.fx("small")
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}
