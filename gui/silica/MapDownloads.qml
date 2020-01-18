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

MapPage {
    id: mapDownload
    pageTitle: "Map Downloads"
    pageFlickable: availableList

    MapDownloadsModel{
        id:mapDownloadsModel
        property string msg: ""
        onMapDownloadFails: {
            var msg = qsTranslate("message", message);
            console.log("Map download failed: " + msg);
        }
        property string preferedDirectory: ""
        Component.onCompleted: {
            var dirs = mapDownloadsModel.getLookupDirectories();
            if (dirs.length > 1)
                preferedDirectory = dirs[1]; // external storage
            else
                preferedDirectory = dirs[0]; // home maps store
        }
    }

    AvailableMapsModel{
        id: availableMapsModel
    }

    InstalledMapsModel{
        id: installedMapsModel
    }

    MapListView {
        id: availableList
        clip: true

        header: Item {
            height: downloadSection.height + downloadList.height +
                    installedSection.height + installedList.height +
                    availableSection.height
            anchors {
                left: parent.left
                right: parent.right
            }

            Item {
                id: downloadSection
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(2)
                anchors.rightMargin: units.gu(2)
                height: visible ? units.gu(5) : 0
                Row {
                    height: parent.height
                    spacing: units.gu(2)

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        color: styleMap.view.highlightedColor
                        text: qsTr("Downloads")
                        font.pixelSize: units.fx("x-large")
                    }
                }
            }

            ListView {
                id: downloadList
                anchors.top: downloadSection.bottom
                height: contentHeight
                width: parent.width
                interactive: false
                model: mapDownloadsModel

                delegate: SimpleListItem {
                    id: donwloadMapItem
                    height: units.gu(6)
                    color: "transparent"
                    paddingLeft: units.gu(2)

                    column: Column {
                        Row {
                            spacing: units.gu(2)
                            Label {
                                color: styleMap.view.primaryColor
                                font.pixelSize: units.fx("medium")
                                text: mapName
                            }
                            Label {
                                color: styleMap.view.primaryColor
                                font.pixelSize: units.fx("medium")
                                text: (Math.round(progressRole * 1000) / 10).toFixed(1) + " %";
                            }
                        }
                        Label{
                            color: styleMap.view.secondaryColor
                            font.pixelSize: units.fx("x-small")
                            text: errorString.length > 0 ? qsTranslate("message", errorString) : progressDescription
                            elide: Text.ElideRight
                        }
                    }

                    Timer {
                        id: delayCancel
                        interval: 5000
                        onTriggered: {
                            mapDownloadsModel.cancel(model.index);
                        }
                    }
                    action1 {
                        visible: true
                        source: "image://theme/icon-m-cancel"
                        onClicked: {
                            if (delayCancel.running)
                                delayCancel.stop();
                            else
                                delayCancel.start();
                        }
                        animationRunning: delayCancel.running
                        animationInterval: 200
                    }
                }

                function onModelReset() {
                    console.log("mapDownloadsModel rows: " + mapDownloadsModel.rowCount());
                    downloadSection.visible = mapDownloadsModel.rowCount() > 0;
                }

                Component.onCompleted: {
                    mapDownloadsModel.modelReset.connect(onModelReset);
                    onModelReset();
                }

                /*menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Cancel")
                        onClicked: {
                            Remorse.itemAction(donwloadMapItem,
                                               qsTr("Canceling"),
                                               function() { mapDownloadsModel.cancel(model.index) });
                        }
                    }
                }*/
            }

            Item {
                id: installedSection
                anchors.top: downloadList.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(2)
                anchors.rightMargin: units.gu(2)
                height: visible ? units.gu(5) : 0
                Row {
                    height: parent.height
                    spacing: units.gu(2)

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        color: styleMap.view.highlightedColor
                        text: qsTr("Installed Maps")
                        font.pixelSize: units.fx("x-large")
                    }
                }
            }

            ListView {
                id: installedList
                anchors.top: installedSection.bottom
                height: contentHeight
                width: parent.width
                interactive: false
                model: installedMapsModel

                delegate: SimpleListItem {
                    id: installedMapItem
                    height: units.gu(6)
                    color: "transparent"
                    paddingLeft: units.gu(2)

                    column: Column {
                        Label {
                            color: styleMap.view.primaryColor
                            font.pixelSize: units.fx("medium")
                            text: name
                        }
                        Label{
                            font.pixelSize: units.fx("x-small")
                            color: styleMap.view.secondaryColor
                            text: Qt.formatDate(time)
                            visible: time != null
                        }
                    }

                    Timer {
                        id: delayDelete
                        interval: 5000
                        onTriggered: {
                            installedMapsModel.deleteMap(model.index);
                        }
                    }
                    action1 {
                        visible: true
                        source: "image://theme/icon-m-delete"
                        onClicked: {
                            if (delayDelete.running)
                                delayDelete.stop();
                            else
                                delayDelete.start();
                        }
                        animationRunning: delayDelete.running
                        animationInterval: 200
                    }

                    Timer {
                        id: delayUpdate
                        interval: 2000
                        onTriggered: updateMap()
                    }
                    action2 {
                        visible: updateAvailable
                        source: "image://theme/icon-m-cloud-download"
                        onClicked: {
                            if (delayUpdate.running)
                                delayUpdate.stop();
                            else
                                delayUpdate.start();
                        }
                        animationRunning: delayUpdate.running
                        animationInterval: 150
                    }

                    property bool updateAvailable: false

                    function updateMap() {
                        var map = availableMapsModel.mapByPath(path);
                        if (map) {
                            var baseDir = directory.substring(0, directory.lastIndexOf("/"));
                            var dir = mapDownloadsModel.suggestedDirectory(map, baseDir);
                            mapDownloadsModel.downloadMap(map, dir);
                            console.log("Downloading to " + dir);
                        }
                    }

                    function checkUpdate() {
                        console.log("checking updates for map " + name + " (" + path + ")");
                        if (path.length > 0) {
                            var latestReleaseTime = availableMapsModel.timeOfMap(path);
                            console.log("map time: " + time + " latestReleaseTime: " + latestReleaseTime +" (" + typeof(latestReleaseTime) + ")");
                            updateAvailable = latestReleaseTime !== null && latestReleaseTime > time;
                        } else {
                            updateAvailable = false;
                        }
                    }

                    Component.onCompleted: {
                        checkUpdate();
                        availableMapsModel.modelReset.connect(checkUpdate);
                        availableMapsModel.loadingChanged.connect(checkUpdate);
                    }

                    /*menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Update")
                            visible: updateAvailable
                            onClicked: updateMap()
                        }
                        MenuItem {
                            text: qsTr("Delete")
                            onClicked: {
                                Remorse.itemAction(installedMapItem,
                                                   qsTr("Deleting"),
                                                   function() { installedMapsModel.deleteMap(model.index) });
                            }
                        }
                    }*/
                }

                function onModelChange() {
                    console.log("installedMapsModel rows: " + installedMapsModel.rowCount());
                    installedSection.visible = installedMapsModel.rowCount() > 0;
                }

                Component.onCompleted: {
                    installedMapsModel.modelReset.connect(onModelChange);
                    installedMapsModel.databaseListChanged.connect(onModelChange);
                    onModelChange();
                }
            }

            Item {
                id: availableSection
                anchors.top: installedList.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: units.gu(2)
                anchors.rightMargin: units.gu(2)
                height: units.gu(5)
                Row {
                    height: parent.height
                    spacing: units.gu(2)

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        color: styleMap.view.highlightedColor
                        text: availableList.tree.length > 0 ? availableList.tree[0].name : qsTr("Available Maps")
                        font.pixelSize: units.fx("x-large")
                    }

                    MapIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        height: units.gu(5)
                        source: "image://theme/icon-m-left"
                        label.text: "Back"
                        label.font.pixelSize: units.fx("small")
                        visible: availableList.tree.length > 0
                        onClicked: {
                            availableList.tree = availableList.tree.slice(1, availableList.tree.length);
                        }
                    }
                }
            }
        }

        contentHeight: units.gu(6)
        anchors.fill: parent

        property var tree: []

        model: DelegateModel {
            id: delegateModel
            model: availableMapsModel
            rootIndex : availableList.tree.length > 0 ? availableList.tree[0].index : null

            delegate: SimpleListItem {
                id: availableMapItem
                height: units.gu(6)
                color: "transparent"
                paddingLeft: units.gu(2)

                column: Column {
                    Label {
                        color: styleMap.view.primaryColor
                        font.pixelSize: units.fx("medium")
                        text: name
                    }
                    Label{
                        font.pixelSize: units.fx("x-small")
                        color: styleMap.view.secondaryColor
                        text: size
                        visible: size != ""
                    }
                }

                Timer {
                    id: delayDownload
                    interval: 2000
                    onTriggered: {
                        var map = availableMapsModel.map(availableMapsModel.index(model.index, 0, delegateModel.rootIndex));
                        if (map) {
                            var dir = mapDownloadsModel.suggestedDirectory(map, mapDownloadsModel.preferedDirectory);
                            mapDownloadsModel.downloadMap(map, dir);
                        }
                    }
                }
                action1 {
                    visible: !model.dir
                    source: "image://theme/icon-m-cloud-download"
                    onClicked: {
                        if (delayDownload.running)
                            delayDownload.stop();
                        else
                            delayDownload.start();
                    }
                    animationRunning: delayDownload.running
                    animationInterval: 150
                }

                //highlighted: model.dir && pressed

                onClicked: {
                    if (model.dir) {
                        var v = availableList.tree.slice();
                        if (availableList.tree.length > 0) {
                            v.splice(0, 0, { "index": availableMapsModel.index(model.index, 0, availableList.tree[0].index), "name": model.name });
                        } else {
                            v.splice(0, 0, { "index": availableMapsModel.index(model.index, 0), "name": model.name });
                        }
                        availableList.tree = v;
                    }
                }
            }
        }
    }
}
