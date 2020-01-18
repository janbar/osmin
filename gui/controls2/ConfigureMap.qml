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
import Osmin 1.0
import "./components"

PopOver {
    id: configureMap

    title: "Configure Map"
    contents: Column {
        spacing: units.gu(1)

        MapCheckBox {
            id: rotate
            width: parent.width
            color: styleMap.popover.foregroundColor
            text: qsTr("Map rotation")
            checked: mapUserSettings.rotateEnabled
            onClicked: {
                mapUserSettings.rotateEnabled = !mapUserSettings.rotateEnabled;
            }
        }

        MapCheckBox {
            id: hillShading
            width: parent.width
            color: styleMap.popover.foregroundColor
            text: qsTr("Hill Shades")
            checked: mapUserSettings.hillShadesEnabled
            onClicked: {
                mapUserSettings.hillShadesEnabled = !mapUserSettings.hillShadesEnabled;
            }
        }

        MapCheckBox {
            id: seaRendering
            width: parent.width
            color: styleMap.popover.foregroundColor
            text: qsTr("Render Sea")
            checked: mapEngineSettings.renderSea
            onClicked: {
                mapEngineSettings.renderSea = !mapEngineSettings.renderSea;
            }
        }

        MapCheckBox {
            id: showAltLanguage
            width: parent.width
            color: styleMap.popover.foregroundColor
            text: qsTr("Prefer English names")
            checked: mapEngineSettings.showAltLanguage
            onClicked: {
                mapEngineSettings.showAltLanguage = !mapEngineSettings.showAltLanguage;
            }
        }

        Label {
            text: qsTr("Style")
            color: styleMap.popover.foregroundColor
            font.pointSize: units.fs("small")
        }
        ComboBox {
            id: mapStyleSheet
            width: parent.width
            height: units.gu(4)
            flat: true
            background: Rectangle {
                color: styleMap.view.backgroundColor
                anchors.fill: parent
            }
            textRole: "text"
            onActivated: {
                var stylesheet = mapStyle.file(currentIndex);
                mapStyle.style = stylesheet;
            }
            MapStyleModel { id: mapStyle }
            Component.onCompleted: {
                var tab = []
                for (var i = 0; i < mapStyle.rowCount(); ++i) {
                    tab.push({ "text": mapStyle.data(mapStyle.index(i,0),256), "value": i });
                }
                model = tab.slice();
                var stylesheet = mapStyle.style;
                currentIndex = mapStyle.indexOf(stylesheet);
            }
        }

        Label {
            text: qsTr("Font name")
            color: styleMap.popover.foregroundColor
            font.pointSize: units.fs("small")
        }
        ComboBox {
            id: fontName
            width: parent.width
            height: units.gu(4)
            flat: true
            background: Rectangle {
                color: styleMap.view.backgroundColor
                anchors.fill: parent
            }
            textRole: "text"
            onActivated: mapEngineSettings.fontName = model[currentIndex].text
            Component.onCompleted: currentIndex = indexOfValue(mapEngineSettings.fontName)
            model: [
                { text: qsTr("DejaVu Sans") },
                { text: qsTr("Droid Serif") },
                { text: qsTr("Liberation Sans") }
            ]
            function indexOfValue(val) {
                if (val === "DejaVu Sans")
                    return 0;
                if (val === "Droid Serif")
                    return 1;
                if (val === "Liberation Sans" )
                    return 2;
                return 0;
            }
        }

        Label {
            text: qsTr("Font size")
            color: styleMap.popover.foregroundColor
            font.pointSize: units.fs("small")
        }
        ComboBox {
            id: fontSize
            width: parent.width
            height: units.gu(4)
            flat: true
            background: Rectangle {
                color: styleMap.view.backgroundColor
                anchors.fill: parent
            }
            textRole: "text"
            onActivated: mapEngineSettings.fontSize = model[currentIndex].value
            Component.onCompleted: currentIndex = indexOfValue(mapEngineSettings.fontSize)
            model: [
                { value: 2.0, text: qsTr("Normal") },
                { value: 3.0, text: qsTr("Big") },
                { value: 4.0, text: qsTr("Bigger") },
                { value: 6.0, text: qsTr("Huge") }
            ]
            function indexOfValue(val) {
                if (val <= 2.0)
                    return 0;
                if (val <= 3.0)
                    return 1;
                if (val <= 4.0)
                    return 2;
                return 3;
            }
        }

        Column {
            width: parent.width
            MapCheckBox {
                id: renderingType
                width: parent.width
                color: styleMap.popover.foregroundColor
                text: qsTr("Tiled rendering (lower quality)")
                checked: mapUserSettings.renderingTypeTiled
                onClicked: {
                    mapUserSettings.renderingTypeTiled = !mapUserSettings.renderingTypeTiled;
                }
            }
            Label {
                text: "It supports map rotating, but labels are rotated too. Rendering may be more responsive, due to tile caching in memory."
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
    }
}
