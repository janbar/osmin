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

Item {
    id: scaleIndicator

    readonly property double maxSize: units.gu(10)
    property double pixelSize: 0 // meters
    property double indicatorWidth: 0
    property string label: ""
    property color color: "black"

    height: caption.height + units.gu(1)
    onPixelSizeChanged: updateIndicator()

    function updateIndicator() {
        // 1,2,5,10,20,50,100,200,500
        var _label = "";
        var _width = 0.0;
        var area = maxSize * pixelSize;
        var base = Math.floor(Math.log(area) / Math.LN10);
        if (base > 6) {
            scaleIndicator.label = "";
            scaleIndicator.indicatorWidth = 0;
        }
        var m = area / Math.pow(10, base);
        var s = (m < 2 ? 1 : m < 5 ? 2 : 5);
        if (base < 3) {
            var sm = s * Math.pow(10, base);
            scaleIndicator.label = sm.toFixed() + "m";
            scaleIndicator.width = sm / pixelSize;
        } else {
            var skm = s * Math.pow(10, base - 3);
            scaleIndicator.label = skm.toFixed() + "km";
            scaleIndicator.width = skm * 1000 / pixelSize;
        }
    }

    Rectangle{
        id: bar
        width: parent.width
        height: units.gu(0.25)
        color: parent.color
        anchors.bottom: parent.bottom
    }

    Rectangle{
        id: borderLeft
        width: units.gu(0.25)
        height: units.gu(1)
        color: parent.color
        anchors.right: bar.left
        anchors.verticalCenter: bar.verticalCenter
    }

    Rectangle{
        id: borderRight
        width: units.gu(0.25)
        height: units.gu(1)
        color: parent.color
        anchors.left: bar.right
        anchors.verticalCenter: bar.verticalCenter
    }

    Label{
        id: caption
        text: scaleIndicator.label
        font.pixelSize: units.fs("small")
        color: parent.color
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
