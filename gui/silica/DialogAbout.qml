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

import QtQuick 2.2
import Sailfish.Silica 1.0
import Osmin 1.0
import "./components"

DialogBase {
    id: dialog
    title: qsTr("About")

    cancelText: qsTr("Close")
    acceptText: ""
    canAccept: false

    contentSpacing: units.gu(2)

    Text {
        color: styleMap.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("OSMin is a GPS Navigator based on a fork of OSMScout. It allows on-road routing and off-road navigation with OpenStreetMap® data.")
        wrapMode: Label.Wrap
        font.pixelSize: units.fx("medium")
    }
    Text {
        color: styleMap.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Author: %1").arg("Jean-Luc Barriere")
        font.pixelSize: units.fx("medium")
    }
    Text {
        color: styleMap.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Version: %1").arg(VersionString)
        font.pixelSize: units.fx("medium")
    }
    Text {
        id: donate
        color: styleMap.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        font.pixelSize: units.fx("small")
        text: "<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=jlbarriere68%40gmail%2ecom&lc=US&item_name=OSMin&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted'>Donate with Paypal</a>"
        onLinkActivated: Qt.openUrlExternally(link)
        linkColor: styleMap.view.linkColor
    }
}
