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
    id: trackCollection
    pageTitle: qsTr("Track Collection")
    pageFlickable: availableList
    isRoot: (availableList.tree.length === 0)
    onGoUpClicked: {
        var v = availableList.tree.slice(1, availableList.tree.length);
        availableList.tree = v;
    }

    property var mapView: null

    signal showPosition(double lat, double lon)

    Component.onCompleted: {
        GPXListModel.loadData();
    }

    MapListView {
        id: availableList
        contentHeight: units.gu(8)
        width: parent.width
        height: parent.height - mapPreview.height

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
                    onParseFinished: function(succeeded) {
                        // caller wait for signal loaded(bool)
                        loadData();
                        // on failure show alert
                        if (!succeeded) {
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
                                           else
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
                               function onCourseIdChanged() {
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
                                font.pixelSize: units.fs("medium")
                                font.bold: dir
                                text: name
                                elide: Text.ElideRight
                            }
                            Label {
                                visible: !dir
                                width: parent.width
                                height: visible ? visibleHeight : 0
                                color: styleMap.view.secondaryColor
                                font.pixelSize: units.fs("x-small")
                                clip: true
                                text: fileModel.parsing ? (Math.round(fileModel.progress * 1000) / 10).toFixed(1) + " %"
                                                        : timestamp.toLocaleDateString() + " " + timestamp.toLocaleTimeString()

                                // break binding loop
                                property real visibleHeight: 0
                                onImplicitHeightChanged: { fixHeight(); }
                                function fixHeight() {
                                    visibleHeight = implicitHeight;
                                }
                            }
                            Label {
                                visible: text !== ""
                                height: visible ? visibleHeight : 0
                                color: styleMap.view.secondaryColor
                                font.pixelSize: units.fs("x-small")
                                text: fileModel.description
                                wrapMode: Text.WordWrap

                                // break binding loop
                                property real visibleHeight: 0
                                onImplicitHeightChanged: { fixHeight(); }
                                function fixHeight() {
                                    visibleHeight = implicitHeight;
                                }
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
                                    font.pixelSize: units.fs("medium")
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
                                    font.pixelSize: units.fs("medium")
                                    onTriggered: {
                                        dialogAction.title = model.dir ? qsTr("Delete folder ?") : qsTr("Delete file ?");
                                        dialogAction.text = model.name;
                                        dialogAction.open();
                                        ToolBox.connectOnce(dialogAction.reply, deleteItem);
                                    }
                                    function deleteItem(accepted) {
                                        if (accepted) {
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
                    }

                    delegate: Item {
                        width: fileRow.implicitWidth
                        height: fileRow.implicitHeight
                        Row {
                            id: fileRow
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
                                    font.pixelSize: units.fs("medium")
                                }
                                Label {
                                    visible: text !== ""
                                    height: visible ? implicitHeight : 0
                                    text: type === 0 ? Converter.readableDistance(length) : "" /*symbol*/
                                    color: styleMap.view.secondaryColor
                                    font.pixelSize: units.fs("medium")
                                }
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                // show the map preview
                                if (type === 1) // waypoint
                                    selectedPOI = { "lat": lat, "lon": lon, "label": name, "elevation": elevation };
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    visible: ((index & 1) === 1)
                    color: styleMap.view.highlightedColor
                    opacity: 0.05
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

    property var selectedPOI: null

    Loader {
        id: mapPreview
         // active the preview on selection
        active: selectedPOI !== null
        height: active ? parent.height / 2 : 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        asynchronous: true
        sourceComponent: Item {
            id: preview
            anchors.fill: parent
            property alias map: map
            Map {
                id: map
                showCurrentPosition: true
                anchors.fill: parent
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                color: "white"
                opacity: 0.7
                height: about.height
                Column {
                    id: about
                    anchors.left: parent.left
                    anchors.right: parent.right
                    padding: units.gu(1)
                    Row {
                        width: parent.width
                        Label {
                            width: parent.width - units.gu(10)
                            font.pixelSize: units.fs("medium")
                            font.bold: true
                            color: "black"
                            elide: Text.ElideRight
                            text: selectedPOI.label
                            horizontalAlignment: Label.AlignLeft
                        }
                        Label {
                            width: units.gu(8)
                            font.pixelSize: units.fs("small")
                            color: "black"
                            text: (selectedPOI.elevation > 0 ? " Δ " + Converter.panelElevation(selectedPOI.elevation) : "")
                            horizontalAlignment: Label.AlignRight
                        }
                    }
                    Row {
                        width: parent.width
                        Label {
                            width: parent.width / 2 - units.gu(1)
                            font.pixelSize: units.fs("small")
                            color: "black"
                            text: Converter.readableCoordinates(selectedPOI.lat, selectedPOI.lon)
                            horizontalAlignment: Label.AlignLeft
                        }
                        Label {
                            width: parent.width / 2 - units.gu(1)
                            font.pixelSize: units.fs("small")
                            color: "black"
                            text: Converter.readableCoordinatesGeocaching(selectedPOI.lat, selectedPOI.lon)
                            horizontalAlignment: Label.AlignRight
                        }
                    }
                }
            }
            MapIcon {
                id: buttonShowPosition
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: units.gu(1)
                source: "qrc:/images/trip/here.svg"
                color: "black"
                backgroundColor: "white"
                borderPadding: units.gu(1.0)
                opacity: 0.7
                height: units.gu(6)
                onClicked: {
                    showPosition(selectedPOI.lat, selectedPOI.lon);
                    popped();
                    stackView.pop();
                }
            }
            MapIcon {
                id: buttonClose
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: units.gu(1)
                source: "qrc:/images/close.svg"
                color: "black"
                backgroundColor: "white"
                borderPadding: units.gu(1.5)
                opacity: 0.7
                height: units.gu(6)
                onClicked: {
                    selectedPOI = null; // deactivate the preview
                }
            }
        }

        onStatusChanged: {
            if (active && status === Loader.Ready) {
                console.log("Activate map preview");
                trackCollection.selectedPOIChanged.connect(showSelectedLocation);
                showSelectedLocation();
            } else if (!active) {
                console.log("Deactivate map preview");
                trackCollection.selectedPOIChanged.disconnect(showSelectedLocation);
            }
        }

        function showSelectedLocation() {
            console.log("Show selected location on map preview");
            item.map.showCoordinatesInstantly(selectedPOI.lat, selectedPOI.lon);
            item.map.addPositionMark(0, selectedPOI.lat, selectedPOI.lon);
            item.map.removeAllOverlayObjects();
        }

        Behavior on height {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
        }
    }

    DialogBase {
        id: dialogEdit
        title: qsTr("Rename")

        property var model: null

        signal requestUpdate(var model, var newValue)

        onClosed: {
            if (result === Dialog.Accepted) {
                var newValue = inputLabel.text.trim();
                if (newValue.length > 0) {
                    if (!model.dir)
                        newValue += ".gpx";
                    requestUpdate(model, newValue);
                } else
                    requestUpdate(null, "");
            } else {
                // caller is waiting the signal
                requestUpdate(null, "");
            }
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
            font.pixelSize: units.fs("medium")
            placeholderText: qsTr("Enter the name")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
            EnterKey.type: Qt.EnterKeyDone
        }

        onOpened: {
            result = Dialog.Rejected;
            var name = model.name;
            var p = name.lastIndexOf(".");
            if (p >= 0)
                name = name.substr(0, p);
            inputLabel.text = name;
            inputLabel.forceActiveFocus();
        }
    }

    DialogAction {
        id: dialogAction
    }

    DialogAlert {
        id: dialogAlert
    }
}
