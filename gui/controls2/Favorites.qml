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
import QtQml.Models 2.3
import Osmin 1.0
import "./components"
import "../toolbox.js" as ToolBox

MapPage {
    id: favorites

    states: [
        State {
            name: "default"
        },
        State {
            name: "selection"
            PropertyChanges { target: favorites; pageTitle: qsTr("Select a Place"); }
            PropertyChanges { target: helpText; visible: false; }
        }
    ]

    state: "default"

    pageTitle: qsTr("Favorite Places")
    pageFlickable: favoritesList

    signal selectPOI(var poi)

    onPopped: {
        // a slot could be connected to signal, waiting a selection
        // trigger the signal for null
        if (state === "selection")
            selectPOI(null);
    }

    MapListView {
        id: favoritesList
        contentHeight: contentHeight
        anchors.fill: parent

        model: SortFilterModel {
            model: FavoritesModel
            sort.property: "label"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            //filter.property: "normalized"
            //filter.pattern: new RegExp(Utils.normalizedInputString(filter.displayText), "i")
            //filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: SimpleListItem {
            id: favoriteItem
            height: units.gu(8)
            color: "transparent"

            column: Column {
                Row {
                    POIIcon {
                      id: poi
                      anchors.verticalCenter: parent.verticalCenter
                      poiType: type
                      color: styleMap.view.foregroundColor
                      width: units.gu(6)
                      height: units.gu(6)
                    }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - poi.width
                        Label {
                            width: parent.width
                            color: styleMap.view.primaryColor
                            font.pointSize: units.fs("medium")
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            text: label
                        }
                        Label{
                            font.pointSize: units.fs("x-small")
                            color: styleMap.view.secondaryColor
                            text: Converter.readableCoordinatesGeocaching(lat, lon)
                        }
                    }
                }
            }

            onPressAndHold: {
                if (favorites.state === "default") {
                    mapPage.navigateTo(lat, lon, label);
                    stackView.pop();
                }
            }

            onClicked: {
                if (favorites.state === "selection") {
                    selectPOI({ "lat": lat, "lon": lon, "label": label, "type": type});
                    stackView.pop();
                }
            }

            menuItems: [
                MenuItem {
                    visible: favorites.state === "default"
                    height: (visible ? implicitHeight : 0)
                    text: qsTr("Go there")
                    font.pointSize: units.fs("medium")
                    onTriggered: {
                        mapPage.navigateTo(lat, lon, label);
                        stackView.pop();
                    }
                },
                MenuItem {
                    text: qsTr("Rename")
                    font.pointSize: units.fs("medium")
                    onTriggered: {
                        dialogEdit.model = model;
                        dialogEdit.open();
                        ToolBox.connectOnce(dialogEdit.editRequested, updateModel);
                    }
                    function updateModel(model) {
                        createFavorite(model.lat, model.lon, model.label);
                        removeFavorite(model.id);
                    }
                },
                MenuItem {
                    text: qsTr("Delete")
                    font.pointSize: units.fs("medium")
                    onTriggered: removeFavorite(model.id);
                }
            ]

            menuVisible: true

            highlighted: pressed
        }
    }

    Column {
        id: helpText
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - units.gu(2)
        Text {
            color: styleMap.view.highlightedColor
            text: qsTr("Press and hold the desired item to go there.")
            wrapMode: Text.WordWrap
            maximumLineCount: 4
            font.pointSize: units.fs("x-small")
        }
    }

    DialogBase {
        id: dialogEdit
        title: qsTr("Rename")

        property var model: null

        signal editRequested(var model)
        onAccepted: {
            model.label = inputLabel.text;
            editRequested(model);
        }

        footer: Row {
            leftPadding: units.gu(1)
            rightPadding: units.gu(1)
            bottomPadding: units.gu(1)
            spacing: units.gu(1)
            layoutDirection: Qt.RightToLeft

            Button {
                flat: true
                text: qsTr("Ok")
                onClicked: {
                    dialogEdit.accept();
                }
            }
        }

        TextField {
            id: inputLabel
            font.pointSize: units.fs("medium")
            placeholderText: qsTr("Enter the label")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
            EnterKey.type: Qt.EnterKeyDone
        }

        onOpened: {
            inputLabel.text = model.label;
        }
    }
}
