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
import Osmin 1.0
import "components"
import "components/toolbox.js" as ToolBox

PopOver {
    id: configureMap

    title: qsTr("Configure Map")
    contents: Column {
        spacing: units.gu(1)

        Column {
            width: parent.width
            MapCheckBox {
                id: renderingType
                width: parent.width
                color: styleMap.popover.foregroundColor
                text: qsTr("Tiled rendering")
                checked: settings.renderingTypeTiled
                onClicked: {
                    settings.renderingTypeTiled = !settings.renderingTypeTiled;
                }
            }
            Label {
                text: qsTr("It supports map rotating, but labels are rotated too. Rendering may be more responsive, due to tile caching in memory.")
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pixelSize: units.fs("x-small")
                font.weight: Font.Normal
            }
        }

        Column {
            width: parent.width
            MapCheckBox {
                id: hillShading
                width: parent.width
                color: styleMap.popover.foregroundColor
                text: qsTr("Hill Shades")
                checked: settings.hillShadesEnabled && (hillshadeProvider != null)
                onClicked: {
                    settings.hillShadesEnabled = !settings.hillShadesEnabled;
                }
                enabled: (hillshadeProvider != null)
            }
            Label {
                visible: (hillshadeProvider == null)
                text: qsTr("To activate the functionality, please configure the tile server file from the resources folder.")
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignJustify
                maximumLineCount: 4
                wrapMode: Text.Wrap
                color: foregroundColor
                font.pixelSize: units.fs("x-small")
                font.weight: Font.Normal
            }
        }

        MapCheckBox {
            id: seaRendering
            width: parent.width
            color: styleMap.popover.foregroundColor
            text: qsTr("Render Sea")
            checked: mapSettings.renderSea
            onClicked: {
                mapSettings.renderSea = !mapSettings.renderSea;
            }
        }

        MapCheckBox {
            id: favorites
            width: parent.width
            color: styleMap.popover.foregroundColor
            text: qsTr("Favorite Places")
            checked: settings.showFavorites
            onClicked: {
                settings.showFavorites = !settings.showFavorites;
                mainView.showFavorites = settings.showFavorites;
            }
        }

        MapCheckBox {
            id: showAltLanguage
            width: parent.width
            color: styleMap.popover.foregroundColor
            text: qsTr("Prefer English names")
            checked: mapSettings.showAltLanguage
            onClicked: {
                mapSettings.showAltLanguage = !mapSettings.showAltLanguage;
            }
        }

        Label {
            text: qsTr("Font name")
            color: styleMap.popover.foregroundColor
            font.pixelSize: units.fs("small")
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
            onActivated: mapSettings.fontName = model[currentIndex].text
            Component.onCompleted: currentIndex = indexOfValue(mapSettings.fontName)
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
            font.pixelSize: units.fs("small")
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
            onActivated: mapSettings.fontSize = model[currentIndex].value
            Component.onCompleted: currentIndex = indexOfValue(mapSettings.fontSize)
            model: [
                { value: 2.0, text: qsTr("Small") },
                { value: 3.0, text: qsTr("Normal") },
                { value: 4.0, text: qsTr("Big") },
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

        Label {
            text: qsTr("Style")
            color: styleMap.popover.foregroundColor
            font.pixelSize: units.fs("small")
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
                mapSettings.styleSheetFile = stylesheet;
                settings.styleFlags = "[]"; // clear settings
                // reset flags after style is loaded
                ToolBox.connectWhileFalse(map.onFinishedChanged, function(finished){
                    if (finished) {
                        flags.resetData();
                        MapExtras.setDaylight(!mapView.nightView);
                    }
                    return finished;
                });
            }
            MapStyleModel { id: mapStyle }
            Component.onCompleted: {
                var tab = []
                for (var i = 0; i < mapStyle.rowCount(); ++i) {
                    tab.push({ "text": mapStyle.data(mapStyle.index(i, 0), MapStyleModel.NameRole), "value": i });
                }
                model = tab.slice();
                var stylesheet = mapStyle.style;
                currentIndex = mapStyle.indexOf(stylesheet);
            }
        }

        ListModel { // list of flags available for the style
            id: flags
            Component.onCompleted: {
                resetData();
            }
            function resetData() {
                clear();
                var v = MapExtras.getStyleFlags();
                for (var i = 0; i < v.length; ++i) {
                    if (v[i].name !== "daylight")
                        append({ "name": v[i].name, "value": v[i].value });
                }
            }
        }

        ListView { // view of available flags for the style
            width: parent.width
            height: contentHeight
            model: flags
            interactive: false
            delegate: MapCheckBox {
                width: parent ? parent.width : 0
                color: styleMap.popover.foregroundColor
                text: model.name
                checked: model.value
                onClicked: {
                    model.value = checked;
                    MapExtras.setStyleFlag(model.name, checked);
                    saveStyleFlags();
                }

                function saveStyleFlags() {
                    var flags = [];
                    var v = MapExtras.getStyleFlags();
                    for (var i = 0; i < v.length; ++i) {
                        if (v[i].name !== "daylight")
                            flags.push({"name": v[i].name, "value": v[i].value});
                    }
                    settings.styleFlags = JSON.stringify(flags);
                }
            }
        }
    }
}
