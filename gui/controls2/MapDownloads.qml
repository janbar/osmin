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

MapPage {
    id: mapDownload
    pageTitle: qsTr("Map Downloads")
    pageFlickable: availableList

    MapDownloadsModel {
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

    AvailableMapsModel {
        id: availableMapsModel
    }

    InstalledMapsModel {
        id: installedMapsModel
    }

    MapListView {
        id: availableList

        header: Item {
            height: downloadSection.height + downloadList.height +
                    installedSection.height + installedList.height +
                    availableSection.height
            anchors {
                left: parent.left
                right: parent.right
            }

            Row {
                id: downloadSection
                leftPadding: units.gu(2)
                rightPadding: units.gu(2)
                height: visible ? units.gu(5) : 0
                spacing: units.gu(2)

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: styleMap.view.highlightedColor
                    text: qsTr("Downloads")
                    font.pointSize: units.fs("x-large")
                }
            }

            MapListView {
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
                                font.pointSize: units.fs("medium")
                                text: mapName
                            }
                            Label {
                                color: styleMap.view.primaryColor
                                font.pointSize: units.fs("medium")
                                text: (Math.round(progressRole * 1000) / 10).toFixed(1) + " %";
                            }
                        }
                        Label {
                            color: styleMap.view.secondaryColor
                            font.pointSize: units.fs("x-small")
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
                        source: "qrc:/images/close.svg"
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
            }

            Row {
                id: installedSection
                leftPadding: units.gu(2)
                rightPadding: units.gu(2)
                anchors.top: downloadList.bottom
                height: visible ? units.gu(5) : 0
                spacing: units.gu(2)

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: styleMap.view.highlightedColor
                    text: qsTr("Installed Maps")
                    font.pointSize: units.fs("x-large")
                }
            }

            MapListView {
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
                            width: parent.width - (updateAvailable ? units.gu(12) : units.gu(6))
                            color: styleMap.view.primaryColor
                            font.pointSize: units.fs("medium")
                            text: name
                            elide: Text.ElideRight
                        }
                        Label{
                            font.pointSize: units.fs("x-small")
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
                        source: "qrc:/images/delete.svg"
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
                        source: "qrc:/images/download.svg"
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

            Row {
                id: availableSection
                leftPadding: units.gu(2)
                rightPadding: units.gu(2)
                anchors.top: installedList.bottom
                height: units.gu(5)
                spacing: units.gu(2)

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: styleMap.view.highlightedColor
                    text: availableList.tree.length > 0 ? availableList.tree[0].name : qsTr("Available Maps")
                    font.pointSize: units.fs("x-large")
                }

                MapIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    height: units.gu(5)
                    source: "qrc:/images/go-previous.svg"
                    label.text: qsTr("Back")
                    label.font.pointSize: units.fs("small")
                    visible: availableList.tree.length > 0
                    onClicked: {
                        availableList.tree = availableList.tree.slice(1, availableList.tree.length);
                    }
                }

                BusyIndicator {
                    id: loadingIndicator
                    running: availableMapsModel.loading
                    height: units.gu(5)
                    anchors.verticalCenter: parent.verticalCenter
                    visible: running
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
                        width: parent.width - units.gu(6)
                        color: styleMap.view.primaryColor
                        font.pointSize: units.fs("medium")
                        text: name
                        elide: Text.ElideRight
                    }
                    Label {
                        font.pointSize: units.fs("x-small")
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
                    source: "qrc:/images/download.svg"
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
