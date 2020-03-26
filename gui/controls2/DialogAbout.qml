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

DialogBase {
    id: dialog
    title: qsTr("About")

    footer: Row {
        leftPadding: units.gu(1)
        rightPadding: units.gu(1)
        bottomPadding: units.gu(1)
        spacing: units.gu(1)
        layoutDirection: Qt.RightToLeft

        Button {
            flat: true
            text: qsTr("Close")
            onClicked: dialog.reject()
        }
    }

    contentSpacing: units.gu(2)

    Text {
        color: styleMap.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("OSMin is a GPS Navigator based on a fork of OSMScout. It allows on-road routing and off-road navigation with OpenStreetMap® data.")
        wrapMode: Label.Wrap
        font.pointSize: units.fs("medium")
    }
    Text {
        color: styleMap.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("Author: %1").arg("Jean-Luc Barriere")
        font.pointSize: units.fs("medium")
    }
    Text {
        color: styleMap.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("Version: %1").arg(VersionString)
        font.pointSize: units.fs("medium")
    }
    Text {
        id: donate
        color: styleMap.dialog.foregroundColor
        width: dialog.availableWidth
        font.pointSize: units.fs("medium")
        text: "<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=jlbarriere68%40gmail%2ecom&lc=US&item_name=OSMin&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted'>Donate with Paypal</a>"
        onLinkHovered: {
            if (hoveredLink)
                font.bold = true;
            else
                font.bold = false;
        }
        onLinkActivated: Qt.openUrlExternally(link)
        linkColor: styleMap.view.linkColor
    }
}
