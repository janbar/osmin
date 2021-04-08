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

DialogBase {
    id: dialog
    title: qsTr("Mark position")

    // { symbol, name }
    property var model: ({})

    signal reply(var model)

    onAccepted: {
        var txt = inputName.text.trim();
        if (txt.length > 0)
            model.name = txt;
        model.symbol = markersModel.get(symbols.currentIndex).symbol;
        reply(model);
    }
    onRejected: {
        // caller is waiting the signal
        reply(null);
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
                dialog.accept();
            }
        }
    }

    TextField {
        id: inputName
        font.pointSize: units.fs("medium")
        placeholderText: qsTr("Enter the name")
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
        EnterKey.type: Qt.EnterKeyDone
    }

    ListModel {
        id: markersModel
        ListElement { symbol: "Pin, Red"; iconSource: "qrc:/images/trip/marker.svg"; iconColor: "red" }
        ListElement { symbol: "Pin, Green"; iconSource: "qrc:/images/trip/marker.svg"; iconColor: "green" }
        ListElement { symbol: "Pin, Blue"; iconSource: "qrc:/images/trip/marker.svg"; iconColor: "blue" }
        ListElement { symbol: "Skull and Crossbones"; iconSource: "qrc:/images/poi/danger.svg"; }
        ListElement { symbol: "Gas Station"; iconSource: "qrc:/images/poi/fuel.svg"; }
        ListElement { symbol: "Restaurant"; iconSource: "qrc:/images/poi/restaurant.svg"; }
        ListElement { symbol: "Bar"; iconSource: "qrc:/images/poi/bar.svg"; }
        ListElement { symbol: "Lodging"; iconSource: "qrc:/images/poi/lodging.svg" }
        ListElement { symbol: "Campground"; iconSource: "qrc:/images/poi/campsite.svg" }
        ListElement { symbol: "Car Repair"; iconSource: "qrc:/images/poi/car.svg"; }
    }

    Rectangle {
        width: parent.width
        height: units.gu(8)
        color: "transparent"
        ComboBox {
            id : symbols
            height: units.gu(8)
            width: units.gu(14)
            anchors.centerIn: parent
            flat: true
            model : markersModel

            onActivated: {
                updateIcon(index);
            }
            Component.onCompleted: {
                updateIcon(currentIndex);
            }

            function updateIcon(idx) {
                var item = symbols.model.get(idx);
                selected.source = item.iconSource;
                selected.color = item.iconColor !== undefined ? item.iconColor : styleMap.dialog.foregroundColor;
            }

            MapIcon {
                id: selected
                anchors.centerIn: parent
                enabled: false
                anchors.fill: parent
            }

            delegate: ItemDelegate {
                height: units.gu(6)
                width: parent.width
                contentItem: MapIcon {
                    enabled: false
                    anchors.fill: parent
                    source: iconSource
                    color: iconColor !== undefined ? iconColor : styleMap.dialog.foregroundColor
                }
            }
        }
    }

    onOpened: {
        if (model.name !== undefined)
            inputName.text = model.name + " ";
        if (model.symbol !== undefined) {
            for (var i = 0; i < markersModel.count; ++i) {
                if (markersModel.get(i).symbol === model.symbol) {
                    symbols.currentIndex = i;
                    symbols.updateIcon(i)
                    break;
                }
            }
        }
        inputName.forceActiveFocus();
    }
}
