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
import QtQml 2.2
import Osmin 1.0

Item {
    id: profile
    property var model: null

    Component.onCompleted: {
        canvas.onAvailableChanged.connect(function(){
            if (canvas.available === true) {
                canvas.getContext("2d");
                profile.onModelChanged.connect(redraw);
                redraw();
            } else {
                profile.onModelChanged.disconnect(redraw);
            }
        });
    }
    onWidthChanged: function() { redraw(); }
    onHeightChanged: function() { redraw(); }

    function redraw()
    {
        if (!model)
            return;
        console.log("start drawing");
        canvas.clear();
        canvas.pointCurves[1] = [];
        // reset all movables
        labelLevel1.visible = false;
        labelLevel2.visible = false;
        labelLevel3.visible = false;
        labelDuration.visible = false;
        labelMax.anchors.bottomMargin = 0;

        let b = units.fs("small");
        // show at least 100 meters and not less
        let delta = Math.max(100, model.maxElevation - model.minElevation);
        let ratio_h = (canvas.height - 2 * b) / delta;
        let ratio_w = canvas.width / model.distance;

        // trace min and max
        canvas.traceHorizontal(b, 1, "green");
        labelMin.text = Converter.panelElevation(model.minElevation);
        labelMax.anchors.bottomMargin = b + ratio_h * (model.maxElevation - model.minElevation);
        canvas.traceHorizontal(labelMax.anchors.bottomMargin, 1, "red");
        labelMax.text = Converter.panelElevation(model.maxElevation);

        if (!isNaN(model.duration)) {
            labelDuration.text = Converter.panelDurationHMS(model.duration);
            labelDuration.visible = true;
        }
        labelStart.text = "0";
        labelEnd.text = Converter.panelDistance(model.distance);

        // trace intermediates
        let sub = 5000;
        if (delta < 50)
            sub = 10;
        else if (delta < 100)
            sub = 20;
        else if (delta < 200)
            sub = 50;
        else if (delta < 500)
            sub = 100;
        else if (delta < 1000)
            sub = 200;
        else if (delta < 2000)
            sub = 500;
        else if (delta < 5000)
            sub = 1000;
        else if (delta < 10000)
            sub = 2000;

        let ele = sub + sub * Math.floor(model.minElevation / sub);
        let y = b + ratio_h * (ele - model.minElevation);
        if (y > 2*b) {
            canvas.traceHorizontal(y, 1, "grey");
            labelLevel1.anchors.bottomMargin = y;
            labelLevel1.text = Converter.panelElevation(ele);
            labelLevel1.visible = true;
        }
        y += ratio_h * sub;
        ele += sub;
        if (y > 2*b && y < canvas.height - 2*b) {
            canvas.traceHorizontal(y, 1, "grey");
            labelLevel2.anchors.bottomMargin = y;
            labelLevel2.text = Converter.panelElevation(ele);
            labelLevel2.visible = true;
        }
        y += ratio_h * sub;
        ele += sub;
        if (y < canvas.height - 2*b) {
            canvas.traceHorizontal(y, 1, "grey");
            labelLevel3.anchors.bottomMargin = y;
            labelLevel3.text = Converter.panelElevation(ele);
            labelLevel3.visible = true;
        }

        // trace elevation curve
        for (var i = 0; i < model.dataX.length; ++i) {
            canvas.pointCurves[1].push({"x": ratio_w * model.dataX[i], "y": (b + ratio_h * (model.dataY[i] - model.minElevation))});
        }
        canvas.traceCurve(1, styleMap.popover.highlightedColor, 2);
        console.log("drawing finished");
    }

    Rectangle {
        id: area
        anchors.fill:parent
        color: styleMap.view.backgroundColor

        Label {
            id: labelStart
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            font.pixelSize: units.fs("x-small")
        }
        Label {
            id: labelEnd
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            font.pixelSize: units.fs("x-small")
        }
        Label {
            id: labelDuration
            anchors.top: parent.top
            anchors.right: parent.right
            font.pixelSize: units.fs("x-small")
        }

        Label {
            id: labelMin
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: units.fs("small")
        }
        Label {
            id: labelLevel1
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: units.fs("x-small")
            color: "grey"
        }
        Label {
            id: labelLevel2
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: units.fs("x-small")
            color: "grey"
        }
        Label {
            id: labelLevel3
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: units.fs("x-small")
            color: "grey"
        }
        Label {
            id: labelMax
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: units.fs("small")
        }

        Canvas {
            id: canvas
            anchors.fill: parent
            transform: Rotation { origin.x: area.x; origin.y: area.height/2; angle: 180; axis { x: 1; y: 0; z: 0 }}
            property int origX: 0
            property int origY: 0 /* height * (ymin / (ymin - ymax)) */
            property int maxX: width
            property int maxY: height

            property var pointCurves: [] // [ [ { x , y } ] ]

            function clear(){
                if (context){
                    pointCurves.splice(0, pointCurves.length)
                    context.reset()
                    requestPaint()
                }
            }

            function traceHorizontal(y, w, c) {
                context.beginPath();
                context.strokeStyle = c;
                context.lineWidth = w;
                context.moveTo(origX, y);
                context.lineTo(maxX, y);
                context.stroke();
                context.closePath();
            }

            function traceVertical(x, w, c) {
                context.beginPath();
                context.strokeStyle = c;
                context.lineWidth = w;
                context.moveTo(x, origY);
                context.lineTo(x, maxY);
                context.stroke();
                context.closePath();
            }

            function point(q, h){
                context.reset()
                context.beginPath()
                context.lineWidth = 1
                context.moveTo(origX, h+origY)
                context.lineTo(maxX, h+origY)
                context.moveTo(q+origX, origY)
                context.lineTo(q+origX, maxY)
                context.strokeStyle = Qt.rgba(1,0,0);
                context.stroke()
                context.closePath()
                requestPaint()
            }

            function traceCurve(id, color, lw) {
                if (context) {
                    context.beginPath()
                    context.lineWidth = lw
                    for (var i = 1; i < pointCurves[id].length; i++) {
                        var posiX = origX + pointCurves[id][i].x
                        var posiY = origY + pointCurves[id][i].y
                        context.lineTo(posiX, posiY)
                    }
                    context.moveTo(origX, origY)
                    context.strokeStyle = color;
                    context.stroke()
                    context.closePath()
                }

                requestPaint()
            }

            onPaint: {
                getContext("2d")
            }
        }
    }
}
