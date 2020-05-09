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
import "./components"

PopOver {
    id: mainMenu

    title: qsTr("Main Menu")
    contents: Column {
        spacing: units.gu(1)

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/favourite.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("My Places")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    var page = stackView.push("qrc:/controls2/Favorites.qml");
                }
            }
            Label {
                text: qsTr("Manage Favorite Places.")
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pointSize: units.fs("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/route.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Routes")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    popRouting.show();
                }
            }
            Label {
                text: qsTr("Navigate to a destination.")
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pointSize: units.fs("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/track.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Tracks")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    stackView.push("qrc:/controls2/TrackCollection.qml", { "mapView": mapView });
                }
            }
            Label {
                text: qsTr("Manage your tracks.")
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pointSize: units.fs("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/trip/navigation.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Tracking")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    popTracking.show();
                }
            }
            Label {
                text: qsTr("Follow your track.")
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pointSize: units.fs("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/map.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Configure Map")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    popConfigureMap.show();
                }
            }
        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/download.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Download Maps")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    var page = stackView.push("qrc:/controls2/MapDownloads.qml");
                }
            }
        }

//        Column {
//            width: parent.width
//            MapIcon {
//                source: "qrc:/images/trip/measuring.svg"
//                color: foregroundColor
//                height: units.gu(6)
//                label.text: "  " + qsTr("Measure distance")
//                label.color: foregroundColor
//                label.font.pointSize: units.fs("medium")
//                label.elide: Text.ElideRight
//                label.width: parent.width - units.gu(7)
//                onClicked: {
//                    mainMenu.close();
//                }
//            }
//        }

        Column {
            width: parent.width
            MapIcon {
                source: "qrc:/images/cogs.svg"
                color: foregroundColor
                height: units.gu(6)
                label.text: "  " + qsTr("Settings")
                label.color: foregroundColor
                label.font.pointSize: units.fs("medium")
                label.elide: Text.ElideRight
                label.width: parent.width - units.gu(7)
                onClicked: {
                    mainMenu.close();
                    var page = stackView.push("qrc:/controls2/Settings.qml");
                }
            }
        }

//        Column {
//            width: parent.width
//            MapIcon {
//                enabled: false
//                source: "qrc:/images/help.svg"
//                color: foregroundColor
//                height: units.gu(6)
//                label.text: "  " + qsTr("Help")
//                label.color: foregroundColor
//                label.font.pointSize: units.fs("medium")
//                label.elide: Text.ElideRight
//                label.width: parent.width - units.gu(7)
//                onClicked: {
//                    mainMenu.close();
//                }
//            }
//        }

    }
}
