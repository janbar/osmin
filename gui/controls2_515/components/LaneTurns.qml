/*
 * Copyright (C) 2025
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
    id: laneTurnsComponent

    height: units.gu(8)

    property color color: "white"
    property alias bgColor: bg.color
    property alias bgOpacity: bg.opacity
    property var laneTurns: ["", "through", "through;right", "right"]
    property string laneTurn: "through"
    property int suggestedLaneFrom: 1
    property int suggestedLaneTo: 2

    property var iconObjects: []
    property real iconWidth: height * 0.5 // source icon sizes 5x10

    Component.onCompleted: {
        updateLane();
    }

    Rectangle {
        id: bg
        color: "black"
        opacity: 1.0
        anchors.fill: parent
    }

    Component {
        id: iconComponent
        LaneTurnIcon { }
    }

    onLaneTurnsChanged: {
        updateLane();
    }
    onSuggestedLaneFromChanged: {
        updateLane();
    }
    onSuggestedLaneToChanged: {
        updateLane();
    }

    function updateLane() {
        console.log("Update lane turns");
        if (iconComponent.status == Component.Error) {
            // Error Handling
            console.log("Error loading component:", iconComponent.errorString());
            return
        }
        if (iconComponent.status != Component.Ready) {
            console.log("Component is not ready:", iconComponent.errorString());
            return
        }

        iconObjects.forEach(function(e){ e.destroy(); });
        iconObjects = [];

        var parentObj = laneTurnsComponent;
        parentObj.width = 0;
        for (var i in laneTurns){
            var turn = laneTurns[i];
            var icon = iconComponent.createObject(
                        parentObj,
                        {
                            id: "turnIcon" + i,
                            turnType: turn,
                            suggestedTurn: laneTurn,
                            color: parentObj.color,
                            height: parentObj.height,
                            width: iconWidth,
                            suggested: ( i >= suggestedLaneFrom && i <= suggestedLaneTo)
                        });
            if (typeof icon === 'undefined') {
                // Error Handling
                console.log("Error creating icon object");
                return;
            }

            if (i > 0) {
                var prev = iconObjects[i-1];
                icon.anchors.left = prev.right;
            } else {
                icon.anchors.left = parentObj.left;
            }

            iconObjects.push(icon);
            parentObj.width += icon.width;
        }
    }

}
