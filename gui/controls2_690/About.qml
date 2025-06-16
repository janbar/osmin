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
import "components"

PopOver {
    id: dialog

    title: qsTr("About")
    contents: Column {
        spacing: units.gu(1)
        width: parent.width
        Text {
            color: styleMap.dialog.foregroundColor
            text: qsTr("OSMin is a GPS Navigator based on a fork of OSMScout. It allows on-road routing and off-road navigation with OpenStreetMap® data.")
            width: parent.width
            wrapMode: Text.WordWrap
            font.pixelSize: units.fs("medium")
        }
        Text {
            color: styleMap.dialog.foregroundColor
            text: qsTr("Author: %1").arg("Jean-Luc Barriere")
            width: parent.width
            font.pixelSize: units.fs("medium")
        }
        Text {
            color: styleMap.dialog.foregroundColor
            text: "OSMin " + qsTr("Version %1").arg(VersionString)
            width: parent.width
            font.pixelSize: units.fs("medium")
        }
        Text {
            color: styleMap.dialog.foregroundColor
            text: "Libosmscout " + qsTr("Format %1").arg(MapExtras.getLibraryFormatVersion());
            width: parent.width
            font.pixelSize: units.fs("medium")
        }
        Text {
            color: styleMap.dialog.foregroundColor
            text: "Qt® " + qsTr("Version %1").arg(QtVersion);
            width: parent.width
            font.pixelSize: units.fs("medium")
        }
        Button {
            id: donate
            width: parent.width
            font.pixelSize: units.fs("small")
            text: "Donate with Paypal"
            onClicked: Qt.openUrlExternally("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=jlbarriere68%40gmail%2ecom&lc=US&item_name=OSMin&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted")
        }
    }
}
