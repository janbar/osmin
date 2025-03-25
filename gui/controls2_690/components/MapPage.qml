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
import QtQuick.Layouts 1.3
import QtQml 2.2

// generic page for map
Page {
    id: thisPage

    property string pageTitle: ""
    property Flickable pageFlickable: null
    property bool isRoot: true // by default this page is root
    property bool showFooter: false // by default don't show the footer bar
    property bool showHeader: true // by default show the header bar

    property alias optionsMenuVisible: optionsMenu.visible
    property alias optionsMenuContentItems: optionsMenuPopup.contentData

    signal popped           // action for root page
    signal goUpClicked      // action for a non-root page
    signal searchClicked

    Item {
        id: footer
        height: units.gu(6)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: showFooter ? 0 : - height
        anchors.left: parent.left
        anchors.right: parent.right
        z: 99

        Behavior on anchors.bottomMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutBack; }
        }

        Rectangle {
            id: footerBar
            anchors.fill: parent
            color: "transparent"

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(1)
                anchors.rightMargin: units.gu(1)
                height: units.gu(5)
                color: "transparent"

                Item {
                    id: optionsMenu
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: units.gu(4)
                    height: parent.height
                    visible: false

                    MapIcon {
                        width: units.gu(5)
                        height: width
                        anchors.centerIn: parent
                        source: "qrc:/images/contextual-menu.svg"

                        onClicked: optionsMenuPopup.open()
                        enabled: parent.visible

                        Menu {
                            id: optionsMenuPopup
                            x: parent.width - width
                            transformOrigin: Menu.TopRight
                        }
                    }
                }
            }
        }
    }
}
