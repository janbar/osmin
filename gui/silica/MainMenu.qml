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
import "./components"

PopOver {
    id: mainMenu

    title: "Main Menu"
    contents: Column {
        spacing: units.gu(1)

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/pin.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Map Markers")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                }
            }
            Label {
                text: "Create map markers. Long tap 'Places', then tap the marker flag button."
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pixelSize: units.fx("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/favourite.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("My Places")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                }
            }
            Label {
                text: "Import Favorites, or add via marking points on the map."
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pixelSize: units.fx("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/route.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Routes")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                   mainMenu.close();
                   popRouting.show();
                }
            }
            Label {
                text: "Navigate to a destination."
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pixelSize: units.fx("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/map.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Configure Map")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    popConfigureMap.visible = true;
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "image://theme/icon-m-cloud-download"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Download Maps")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    pageStack.push(tabs.get(1).source);
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/measuring.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Measure distance")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/screen-settings.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Configure screen")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/cogs.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Settings")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/help.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: qsTr("Help")
                label.color: foregroundColor
                label.font.pixelSize: units.fx("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                }
            }
        }

    }
}
