/*
 * Copyright (C) 2020-2021
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
import QtQml.Models 2.3
import "./components"
import "../toolbox.js" as ToolBox

PopOver {
    id: mainMenu
    title: qsTr("Main Menu")
    contentEdgeMargins: 0

    ListModel {
        id: listmodel
        ListElement {
            art: "qrc:/images/trip/favourite.svg"
            name: qsTr("My Places")
            comment: qsTr("Manage your favorite places.")
            foo: function() {
                var page = stackView.push("qrc:/controls2/Favorites.qml");
                ToolBox.connectOnce(page.showPosition, function(lat, lon){
                    if (lat !== NaN && lon !== NaN) {
                         map.showCoordinatesInstantly(lat, lon);
                         mark.lat = lat;
                         mark.lon = lon;
                         mark.screenX = map.width / 2;
                         mark.screenY = map.height / 2;
                         if (navigation)
                             navigation = false;
                         popLocationInfo.show();                                 }
                });
            }
        }
        ListElement {
            art: "qrc:/images/trip/route.svg"
            name: qsTr("Routes")
            comment: qsTr("Navigate to a destination.")
            foo: function() {
                popRouting.show();
            }
        }
        ListElement {
            art: "qrc:/images/trip/track.svg"
            name: qsTr("Tracks")
            comment: qsTr("Manage the collection of saved tracks.")
            foo: function() {
                var page = stackView.push("qrc:/controls2/TrackCollection.qml", { "mapView": mapView });
            }
        }
        ListElement {
            art: "qrc:/images/trip/navigation.svg"
            name: qsTr("Tracking")
            comment: qsTr("Statistics of the current track.")
            foo: function() {
                popTracking.show();
            }
        }
        ListElement {
            art: "qrc:/images/map.svg"
            name: qsTr("Configure Map")
            comment: qsTr("Rendering and style of the map view.")
            foo: function() {
                popConfigureMap.show();
            }
        }
        ListElement {
            art: "qrc:/images/download.svg"
            name: qsTr("Download Maps")
            comment: qsTr("Manage the map database.")
            foo: function() {
                var page = stackView.push("qrc:/controls2/MapDownloads.qml");
            }
        }
        ListElement {
            art: "qrc:/images/voice.svg"
            name: qsTr("Configure Voice")
            comment: qsTr("Choose a voice for driving directions.")
            foo: function() {
                var page = stackView.push("qrc:/controls2/ConfigureVoice.qml");
            }
        }
//        ListElement {
//            art: "qrc:/images/trip/measuring.svg"
//            name: qsTr("Measure distance")
//            comment: qsTr("")
//            foo: function() {
//            }
//        }
        ListElement {
            art: "qrc:/images/cogs.svg"
            name: qsTr("Settings")
            comment: qsTr("General settings, units system and more ...")
            foo: function() {
                var page = stackView.push("qrc:/controls2/Settings.qml");
            }
        }
//        ListElement {
//            art: "qrc:/images/help.svg"
//            name: qsTr("Help")
//            comment: qsTr("More about features and usage.")
//            foo: function() {
//            }
//        }
    }

    contents: Column {
        Repeater {
            model: listmodel
            delegate: SimpleListItem {
                id: listitem
                height: units.gu(8)
                color: "transparent"
                paddingLeft: units.gu(0)
                highlighted: pressed

                column: Column {
                    Row {
                        height: listitem.height
                        spacing: units.gu(1)

                        MapIcon {
                           id: icon
                           visible: true
                           enabled: false
                           anchors.verticalCenter: parent.verticalCenter
                           width: units.gu(8)
                           iconSize: units.gu(5)
                           color: styleMap.view.foregroundColor
                           source: art
                        }

                        Column {
                            width: parent.width - icon.width - units.gu(1)
                            anchors.verticalCenter: parent.verticalCenter
                            Label {
                                width: parent.width - units.gu(1)
                                color: styleMap.view.primaryColor
                                font.pointSize: units.fs("large")
                                font.bold: true
                                text: name
                                elide: Text.ElideRight
                            }
                            Label {
                                width: parent.width - units.gu(1)
                                color: styleMap.view.secondaryColor
                                font.pointSize: units.fs("small")
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                                maximumLineCount: 2
                                wrapMode: Text.Wrap
                                text: comment
                            }
                        }
                    }
                }

                onClicked: {
                    mainMenu.close();
                    model.foo();
                }
            }
        }
    }
}
