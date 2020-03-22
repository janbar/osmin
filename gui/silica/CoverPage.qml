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

import QtQuick 2.0
import Sailfish.Silica 1.0
import "../"

CoverBackground {

    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectCrop
        source: "qrc:/images/osmin.png"
        width: Math.min(parent.height, parent.width) / 2
        height: width
        sourceSize.height: height
        sourceSize.width: width
        opacity: 0.50
    }

}
