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
import QtQml.Models 2.3
import Osmin 1.0
import "./components"
import "../toolbox.js" as ToolBox

MapPage {
    id: trackCollection
    pageTitle: qsTr("Track Collection")
    pageFlickable: availableList
    isRoot: (availableList.tree.length === 0)
    onGoUpClicked: {
        var v = availableList.tree.slice(1, availableList.tree.length);
        availableList.tree = v;
    }

    property var mapView: null

    Component.onCompleted: {
        GPXListModel.loadData();
    }

    MapListView {
        id: availableList
        contentHeight: units.gu(8)
        anchors.fill: parent

        property var tree: []

        model: DelegateModel {
            id: delegateModel
            model: GPXListModel
            rootIndex : availableList.tree.length > 0 ? availableList.tree[0].index : null

            delegate: MouseArea {
                id: availableMapItem
                width: availableList.width
                height: fileView.height

                GPXFileModel {
                    id: fileModel
                    onParseFinished: {
                        if (succeeded) {
                            loadData();
                        } else {
                            dialogAlert.title = name
                            dialogAlert.text = qsTr("Parsing file has failed. The format is not supported or data are corrupted.");
                            dialogAlert.open();
                        }
                    }
                }

                ListView {
                    id: fileView
                    width: parent.width - units.gu(6)
                    height: contentHeight
                    interactive: false
                    model: fileModel

                    header: Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width
                        height: Math.max(units.gu(8), implicitHeight)

                        MapCheckBox {
                           id: display
                           visible: !model.dir
                           anchors.verticalCenter: parent.verticalCenter
                           width: units.gu(5)
                           color: styleMap.view.foregroundColor
                           checked: false
                           onClicked: {
                               if (display.checked) {
                                   mapView.removeCourse();
                                   if (!fileModel.fileValid) {
                                       ToolBox.connectOnce(fileModel.loaded, function(succeeded){
                                           if (succeeded)
                                               mapView.addCourse(bigId, fileModel.createOverlayObjects());
                                       });
                                       ToolBox.connectOnce(fileModel.parseFinished, function(succeeded){
                                           if (!succeeded)
                                               display.checked = false;
                                       });
                                       fileModel.parseFile(model.absoluteFilePath);
                                    } else {
                                       fileModel.loadData();
                                       mapView.addCourse(bigId, fileModel.createOverlayObjects());
                                   }
                               } else {
                                   mapView.removeCourse();
                               }
                           }
                           Component.onCompleted: {
                               display.checked = (mapView.courseId === bigId);
                           }
                           Connections {
                               target: mapView
                               onCourseIdChanged: {
                                   display.checked = (mapView.courseId === bigId);
                               }
                           }
                        }

                        MapIcon {
                            id: folderIcon
                            visible: model.dir
                            anchors.verticalCenter: parent.verticalCenter
                            height: units.gu(5)
                            source: "qrc:/images/go-next.svg"
                            onClicked: optionsMenu.open()
                            enabled: false
                        }

                        Column {
                            width: parent.width - units.gu(5)
                            anchors.verticalCenter: parent.verticalCenter
                            Label {
                                width: parent.width
                                color: styleMap.view.primaryColor
                                font.pointSize: units.fs("medium")
                                font.bold: dir
                                text: name
                                elide: Text.ElideRight
                            }
                            Label {
                                visible: !dir
                                width: parent.width
                                height: visible ? implicitHeight : 0
                                color: styleMap.view.secondaryColor
                                font.pointSize: units.fs("x-small")
                                text: timestamp.toLocaleDateString() + " " + timestamp.toLocaleTimeString()
                            }
                            Label {
                                visible: text !== ""
                                height: visible ? implicitHeight : 0
                                color: styleMap.view.secondaryColor
                                font.pointSize: units.fs("x-small")
                                text: fileModel.description
                                wrapMode: Text.WordWrap
                            }
                        }

                        MapIcon {
                            id: menu
                            anchors.verticalCenter: parent.verticalCenter
                            width: units.gu(5)
                            height: width
                            source: "qrc:/images/contextual-menu.svg"
                            onClicked: optionsMenu.open()
                            visible: (path !== "." || name !== "TRACKER")

                            Menu {
                                id: optionsMenu
                                width: implicitWidth * units.scaleFactor
                                x: parent.width - width
                                transformOrigin: Menu.TopRight
                                MenuItem {
                                    text: qsTr("Rename")
                                    font.pointSize: units.fs("medium")
                                    onTriggered: {
                                        if (display.checked)
                                            mapView.removeCourse();
                                        dialogEdit.model = model;
                                        dialogEdit.open();
                                        ToolBox.connectOnce(dialogEdit.requestUpdate, renameItem);
                                    }
                                    function renameItem(model, newValue) {
                                        if (model) {
                                            var index = null;
                                            if (availableList.tree.length > 0)
                                                index = GPXListModel.index(model.index, 0, availableList.tree[0].index);
                                            else
                                                index = GPXListModel.index(model.index, 0);
                                            GPXListModel.renameItem(newValue, index);
                                        }
                                    }
                                }
                                MenuItem {
                                    text: qsTr("Delete")
                                    font.pointSize: units.fs("medium")
                                    onTriggered: {
                                        dialogAction.title = model.dir ? qsTr("Delete folder ?") : qsTr("Delete file ?");
                                        dialogAction.text = model.name;
                                        dialogAction.open();
                                        ToolBox.connectOnce(dialogAction.accepted, deleteItem);
                                    }
                                    function deleteItem() {
                                        if (display.checked)
                                            mapView.removeCourse();
                                        var index = null;
                                        if (availableList.tree.length > 0)
                                            index = GPXListModel.index(model.index, 0, availableList.tree[0].index);
                                        else
                                            index = GPXListModel.index(model.index, 0);
                                        GPXListModel.removeItem(index);
                                    }
                                }
                            }
                        }
                    }

                    delegate: Row {
                        MapIcon {
                          id: poi
                          anchors.verticalCenter: parent.verticalCenter
                          color: type === 0 && displayColor !== "" ? displayColor : styleMap.view.foregroundColor
                          width: units.gu(6)
                          height: units.gu(6)
                          enabled: false
                          source: type === 0 ? "qrc:/images/trip/segment.svg"
                                             : symbol === "Bar" ? "qrc:/images/poi/bar.svg"
                                             : symbol === "Campground" ? "qrc:/images/poi/campsite.svg"
                                             : symbol === "Gas Station" ? "qrc:/images/poi/fuel.svg"
                                             : symbol === "Lodging" ? "qrc:/images/poi/lodging.svg"
                                             : symbol === "Restaurant" ? "qrc:/images/poi/restaurant.svg"
                                             : symbol === "Skull and Crossbones" ? "qrc:/images/poi/danger.svg"
                                             : symbol === "Car Repair" ? "qrc:/images/poi/car.svg"
                                             : "qrc:/images/poi/marker.svg"
                        }
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            width: fileView.width
                            spacing: 0

                            Label {
                                text: name
                                color: styleMap.view.primaryColor
                                font.pointSize: units.fs("medium")
                            }
                            Label {
                                visible: text !== ""
                                height: visible ? implicitHeight : 0
                                text: type === 0 ? Converter.readableDistance(length) : "" /*symbol*/
                                color: styleMap.view.secondaryColor
                                font.pointSize: units.fs("medium")
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    visible: (index % 2 === 0)
                    color: styleMap.view.highlightedColor
                    opacity: 0.1
                }

                BusyIndicator {
                    id: loadingIndicator
                    running: fileModel.parsing
                    height: units.gu(6)
                    anchors.centerIn: parent
                    visible: running
                }

                onClicked: {
                    if (model.dir) {
                        var v = availableList.tree.slice();
                        if (availableList.tree.length > 0) {
                            v.splice(0, 0, { "index": GPXListModel.index(model.index, 0, availableList.tree[0].index), "name": model.name });
                        } else {
                            v.splice(0, 0, { "index": GPXListModel.index(model.index, 0), "name": model.name });
                        }
                        availableList.tree = v;
                    } else {
                        // show/hide details
                        if (!fileModel.fileValid) {
                            fileModel.parseFile(model.absoluteFilePath);
                        } else if (fileModel.count === 0) {
                            fileModel.loadData();
                        } else {
                            fileModel.clearData();
                        }
                    }
                }
            }
        }
    }

    DialogBase {
        id: dialogEdit
        title: qsTr("Rename")

        property var model: null

        signal requestUpdate(var model, var newValue)
        onAccepted: {
            var newValue = inputLabel.text.trim();
            if (newValue.length > 0) {
                if (!model.dir)
                    newValue += ".gpx";
                requestUpdate(model, newValue);
            } else
                requestUpdate(null, "");
        }
        onRejected: {
            // caller is waiting the signal
            requestUpdate(null, "");
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
            placeholderText: qsTr("Enter the name")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
            EnterKey.type: Qt.EnterKeyDone
        }

        onOpened: {
            var name = model.name;
            var p = name.lastIndexOf(".");
            if (p >= 0)
                name = name.substr(0, p);
            inputLabel.text = name;
        }
    }

    DialogAction {
        id: dialogAction
    }

    DialogAlert {
        id: dialogAlert
    }
}
