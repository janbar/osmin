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

// generic page for map
Page {
    id: thisPage

    property string pageTitle: ""
    property Flickable pageFlickable: null
    property bool isRoot: true // by default this page is root
    property bool showPageHeader: true

    property alias optionsMenuEnabled: optionsMenu.enabled
    property alias optionsMenuContent: optionsMenu._contentColumn
    property alias pageMenuEnabled: pageMenu.enabled
    property alias pageMenuContent: pageMenu._contentColumn
    property alias pageMenuQuickSelect: pageMenu.quickSelect

    default property alias _content: _contentItem.data

    signal popped           // action for root page
    signal goUpClicked      // action for a non-root page
    signal searchClicked

    SilicaFlickable {
        id: body
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            id: pageMenu
            enabled: false
        }

        Column {
            id: column
            width: thisPage.width
            height: thisPage.height

            PageHeader {
                id: pageHeader
                visible: showPageHeader
                title: pageTitle
            }

            Item {
                id: _contentItem
                width: parent.width
                height: parent.height - (showPageHeader ? pageHeader.height : 0)
            }
        }

        // options menu
        PushUpMenu {
            id: optionsMenu
            enabled: false
            MenuItem {
                text: qsTr("About")
                onClicked: {
                    dialogAbout.open();
                }
            }
        }
    }
}
