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
    id: locationInfo
    title: qsTr("Informations")

    property real maximumHeight: parent.height

    property MapPosition position

    function resizeBox(contentHeight) {
        height = Math.min(Math.max(minimumHeight, contentHeight + contentTopMargin + contentEdgeMargins), maximumHeight);
    }

    function searchLocation(lat, lon) {
        sourceValid = position._posValid;
        sourceLat = position._lat;
        sourceLon = position._lon;
        placeLat = lat;
        placeLon = lon;
        placeLabel = Converter.readableCoordinatesGeocaching(lat, lon);
        placeType = "";
        isFavorite = FavoritesModel.isFavorite(lat, lon);
        locationInfoModel.setLocation(lat, lon);
    }

    property bool sourceValid: false
    property double sourceLat: 0.0
    property double sourceLon: 0.0
    property double placeLat: 0.0
    property double placeLon: 0.0
    property string placeLabel: ""
    property string placeType: ""
    property int isFavorite: 0

    LocationInfoModel{
        id: locationInfoModel
        onReadyChange: function(ready) {
            // expose the place info
            if (ready && rowCount() > 0) {
                var mi = index(0, 0);
                placeType = data(mi, LocationInfoModel.TypeRole);
                var str = data(mi, LocationInfoModel.PoiRole);
                if (str.length === 0)
                    str = data(mi, LocationInfoModel.AddressRole);
                if (str.length > 0)
                    placeLabel = str;
            }
        }
    }

    contents: Column {
        id: locationInfoBody
        spacing: units.gu(1)

        ListView {
            id: locationInfoView
            width: parent.width
            height: contentHeight
            interactive: false
            spacing: units.gu(1)
            model: locationInfoModel

            onCountChanged: {
                var h = locationInfoView.contentHeight + locationInfoView.spacing;
                resizeBox(h);
            }

            opacity: locationInfoModel.ready ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation {} }

            header: Label {
                id: locationDistanceLabel
                text: locationInfoModel.distance(sourceLat, sourceLon, placeLat, placeLon) < 5 ?
                          qsTr("You are here") :
                          qsTr("%1 %2 from you")
                              .arg(Converter.readableDistance(locationInfoModel.distance(sourceLat, sourceLon, placeLat, placeLon)))
                              .arg(Converter.readableBearing(locationInfoModel.bearing(sourceLat, sourceLon, placeLat, placeLon)))

                color: styleMap.popover.highlightedColor
                font.pixelSize: units.fs("small")
                bottomPadding: locationInfoView.spacing
                visible: sourceValid
            }

            delegate: Column {
                spacing: 0

                Label {
                    id: entryDistanceLabel

                    width: locationInfoView.width

                    text: qsTr("%1 %2 from").arg(Converter.readableDistance(distance)).arg(Converter.readableBearing(bearing))

                    color: styleMap.popover.highlightedColor
                    font.pixelSize: units.fs("small")
                    visible: !inPlace
                }
                Row {
                    POIIcon {
                      id: poiIcon
                      poiType: type
                      color: styleMap.popover.foregroundColor
                      width: units.gu(6)
                      height: units.gu(6)
                    }
                    Column {
                        Label {
                            id: entryPoi
                            width: locationInfoView.width - poiIcon.width - units.gu(1)
                            text: poi // label
                            color: styleMap.popover.foregroundColor
                            font.pixelSize: units.fs("large")
                            visible: poi != ""
                        }
                        Label {
                            id: entryAddress
                            width: locationInfoView.width - poiIcon.width - units.gu(1)
                            text: address
                            color: styleMap.popover.foregroundColor
                            font.pixelSize: units.fs("medium")
                            visible: address != ""
                        }
                        Label {
                            id: entryRegion
                            width: locationInfoView.width - poiIcon.width - units.gu(1)
                            wrapMode: Text.WordWrap
                            text: {
                                if (region.length > 0) {
                                    var str = region[0];
                                    if (postalCode != "") {
                                        str += ", "+ postalCode;
                                    }
                                    if (region.length > 1) {
                                        for (var i = 1; i < region.length; ++i){
                                            str += ", "+ region[i];
                                        }
                                    }
                                    return str;
                                } else if (postalCode != ""){
                                    return postalCode;
                                }
                            }
                            color: styleMap.popover.foregroundColor
                            font.pixelSize: units.fs("medium")
                            visible: region.length > 0 || postalCode != ""
                        }
                        PhoneLink {
                            id: phoneLink
                            phone: model.phone
                            foregroundColor: styleMap.popover.foregroundColor
                            highlightedColor: styleMap.popover.highlightedColor
                        }
                        WebLink {
                            id: webLink
                            url: model.website
                            foregroundColor: styleMap.popover.foregroundColor
                            highlightedColor: styleMap.popover.highlightedColor
                        }
                    }
                }
            }
        }
    }
}
