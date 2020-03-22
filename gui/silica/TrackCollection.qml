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
import QtQml.Models 2.2
import Osmin 1.0
import "./components"
import "../toolbox.js" as ToolBox

MapPage {
    id: trackCollection
    pageTitle: qsTr("Track Collection")
    pageFlickable: availableList
    isRoot: (availableList.tree.length === 0)
    pageMenuEnabled: !isRoot
    onGoUpClicked: {
        var v = availableList.tree.slice(1, availableList.tree.length);
        availableList.tree = v;
    }

    property var mapView: null

    MapListView {
        id: availableList
        contentHeight: units.gu(8)
        anchors.fill: parent
        clip: true

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
                        if (succeeded)
                            loadData();
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
                                font.pixelSize: units.fx("medium")
                                text: name
                                elide: Text.ElideRight
                            }
                            Label {
                                visible: !dir
                                width: parent.width
                                height: visible ? implicitHeight : 0
                                color: styleMap.view.secondaryColor
                                font.pixelSize: units.fx("x-small")
                                text: timestamp.toLocaleDateString() + " " + timestamp.toLocaleTimeString()
                            }
                            Label {
                                visible: text !== ""
                                height: visible ? implicitHeight : 0
                                color: styleMap.view.secondaryColor
                                font.pixelSize: units.fx("x-small")
                                text: fileModel.description
                                wrapMode: Text.WordWrap
                            }
                        }

                        Timer {
                            id: delayDelete
                            interval: 5000
                            onTriggered: {
                                var index = null;
                                if (availableList.tree.length > 0)
                                    index = GPXListModel.index(model.index, 0, availableList.tree[0].index);
                                else
                                    index = GPXListModel.index(model.index, 0);
                                GPXListModel.removeItem(index);
                            }
                        }
                        MapIcon {
                            id: deleteAction
                            anchors.verticalCenter: parent.verticalCenter
                            width: units.gu(5)
                            height: width
                            source: "image://theme/icon-m-delete"
                            visible: (path !== "." || name !== "TRACKER")
                            onClicked: {
                                if (delayDelete.running)
                                    delayDelete.stop();
                                else {
                                    if (display.checked)
                                        mapView.removeCourse();
                                    delayDelete.start();
                                }
                            }
                            animationRunning: delayDelete.running
                            animationInterval: 200
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
                                             : "qrc:/images/poi/marker.svg"
                        }
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            width: fileView.width
                            spacing: 0

                            Label {
                                text: name
                                color: styleMap.view.primaryColor
                                font.pixelSize: units.fx("medium")
                            }
                            Label {
                                visible: text !== ""
                                height: visible ? implicitHeight : 0
                                text: type === 0 ? Converter.readableDistance(length) : "" /*symbol*/
                                color: styleMap.view.secondaryColor
                                font.pixelSize: units.fx("medium")
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

                onPressAndHold: {
                    if (mapView.courseId === model.bigId)
                        mapView.removeCourse();
                    dialogEdit.model = model;
                    dialogEdit.show();
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

                BusyIndicator {
                    id: loadingIndicator
                    running: fileModel.parsing
                    size: BusyIndicatorSize.Small
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

    Component {
        id: menuItemComp
        MenuItem {
        }
    }

    property MenuItem menuItemPageUp: null
    onIsRootChanged: menuItemPageUp.visible = !isRoot // enable the menu when the content isn't root
    pageMenuQuickSelect: true

    Component.onCompleted: {
        // create the menu item to navigate back. Starting from root it isn't visible
        menuItemPageUp = menuItemComp.createObject(pageMenuContent, {"text" : qsTr("Page Up"), "visible" : false});
        menuItemPageUp.onClicked.connect(goUpClicked);
        GPXListModel.loadData();
    }

    PopOver {
        id: dialogEdit
        title: qsTr("Rename")
        visible: false
        width: units.gu(minSizeGU)
        height: units.gu(24)
        anchors.centerIn: parent

        property var model: null

        signal requestUpdate(var model, var newValue)

        contents: Column {
            spacing: units.gu(2)

            TextField {
                id: inputLabel
                width: parent.width
                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fx("medium")
                placeholderText: qsTr("Enter the name")
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
            }

            MapIcon {
                id: acceptButton
                source: "image://theme/icon-m-accept"
                height: units.gu(5)
                anchors.horizontalCenter: parent.horizontalCenter
                color: styleMap.popover.foregroundColor
                label.text: qsTr("Ok")
                onClicked: {
                    var newValue = inputLabel.text.trim();
                    if (newValue.length > 0) {
                        if (!dialogEdit.model.dir)
                            newValue += ".gpx";
                        dialogEdit._model = dialogEdit.model;
                        dialogEdit._newValue = newValue;
                    }
                    dialogEdit.close();
                }
            }
        }

        property var _model: null
        property string _newValue: ""
        onShow: {
            _model = null;
            _newValue = "";
            var name = model.name;
            var p = name.lastIndexOf(".");
            if (p >= 0)
                name = name.substr(0, p);
            inputLabel.text = name;
            visible = true;
        }
        onClose: {
            requestUpdate(_model, _newValue);
            visible = false;
            focus = false;
        }
    }
}
