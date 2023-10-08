/*
 * Copyright (C) 2021
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

MapPage {
    id: configureVoice
    pageTitle: qsTr("Configure Voice")
    pageFlickable: availableList

    AvailableVoicesModel {
        id: availableVoicesModel
        property string msg: ""
        onLoadingChanged: {
            if (!loading && fetchError.length > 0) {
                msg = qsTranslate("message", message);
                console.log("Download available voices failed: " + msg);
            } else if (loading) {
                msg = "";
            }
        }
    }

    InstalledVoicesModel {
        id: installedVoicesModel
    }

    MapListView {
        id: availableList

        header: Item {
            height: statusSection.height
            anchors {
                left: parent.left
                right: parent.right
            }

            Row {
                id: statusSection
                leftPadding: units.gu(2)
                rightPadding: units.gu(2)
                height: visible ? units.gu(3) : 0
                spacing: units.gu(2)

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    color: "red"
                    font.pointSize: units.fs("large")
                    text: availableVoicesModel.msg
                }

                visible: (availableVoicesModel.msg.length > 0)
            }
        }

        contentHeight: units.gu(8)
        anchors.fill: parent

        model: DelegateModel {
            id: delegateModel
            model: availableVoicesModel

            delegate: SimpleListItem {
                id: availableVoiceItem
                height: units.gu(8)
                color: "transparent"
                paddingLeft: units.gu(0)
                alternate: ((index & 1) === 0)

                column: Column {
                    Row {
                        height: availableVoiceItem.height
                        spacing: units.gu(1)

                        MapCheckBox {
                           id: selector
                           visible: true
                           enabled: (rowState === AvailableVoicesModel.Downloaded)
                           anchors.verticalCenter: parent.verticalCenter
                           width: units.gu(5)
                           color: styleMap.view.foregroundColor
                           checked: false
                           opacity: (enabled ? 1.0 : 0.1)
                           onClicked: {
                               if (!selector.checked) {
                                   mapVoice.select("");
                               } else {
                                   if (mapVoice.select(model.name))
                                       mapVoice.testVoice();
                               }
                           }
                           Component.onCompleted: {
                               selector.checked = (mapVoice.voiceName === model.name);
                           }
                           Connections {
                               target: mapVoice
                               function onVoiceSelected() {
                                   selector.checked = (mapVoice.voiceName === model.name);
                               }
                           }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            Label {
                                color: styleMap.view.primaryColor
                                font.pointSize: units.fs("medium")
                                font.bold: true
                                text: name
                                elide: Text.ElideRight
                            }
                            Label {
                                color: styleMap.view.primaryColor
                                font.pointSize: units.fs("medium")
                                text: qsTranslate("resource", lang) + " , " + qsTranslate("resource", gender)
                            }
                            Label {
                                font.pointSize: units.fs("x-small")
                                color: styleMap.view.secondaryColor
                                text: qsTr("Author: %1").arg(author)
                            }
                        }
                    }
                }

                Timer {
                    id: delayAction
                    interval: 2000
                    onTriggered: {
                        var rowIndex = availableVoicesModel.index(index, 0);
                        if (model.state === AvailableVoicesModel.Downloaded) {
                            if (mapVoice.voiceName === model.name)
                                mapVoice.select("")
                            availableVoicesModel.remove(rowIndex);
                            rowState = AvailableVoicesModel.Available;
                        } else {
                            availableVoicesModel.download(rowIndex);
                            rowState = Qt.binding(function foo(){return model.state;});
                        }
                    }
                }

                //@FIXME: signal on state update failed
                property int rowState: AvailableVoicesModel.Available
                Component.onCompleted: {
                    rowState = Qt.binding(function foo(){return model.state;});
                }

                action1 {
                    visible: (rowState !== AvailableVoicesModel.Downloading)
                    source: (rowState === AvailableVoicesModel.Downloaded ? "qrc:/images/delete.svg"
                                                                             : "qrc:/images/download.svg")
                    onClicked: {
                        if (delayAction.running)
                            delayAction.stop();
                        else
                            delayAction.start();
                    }
                    animationRunning: delayAction.running
                    animationInterval: 150
                }

                BusyIndicator {
                    id: loadingIndicator
                    running: (rowState === AvailableVoicesModel.Downloading)
                    height: units.gu(6)
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    visible: running
                }
            }
        }
    }
}
