import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQml 2.2
import QtPositioning 5.2
import Osmin 1.0
import "./components"

MapPage {
    id: searchPage
    pageTitle: qsTr("Search Place")
    isRoot: true

    property bool acceptEnabled: true
    property string acceptLabel: qsTr("Accept")
    property string acceptIcon: "qrc:/images/trip/pin.svg"
    property bool addFavoriteEnabled: true
    property bool addMarkerEnabled: true
    property bool showPositionEnabled: false
    property double searchCenterLat
    property double searchCenterLon

    readonly property var distanceTypes: [ 1000.0, 3000.0, 6000.0, 10000.0, 15000.0, 20000.0 ]

    states: [
        State {
            name: "dialog"
            PropertyChanges { target: searchPage; pageTitle: qsTr("Search Place");  isRoot: true; }
            PropertyChanges { target: searchField; visible: true; }
            PropertyChanges { target: suggestionView; visible: true; }
            PropertyChanges { target: searchView; visible: false; model: null; }
            PropertyChanges { target: noData; visible: false; }
        },
        State {
            name: "poi"
            PropertyChanges { target: searchPage; pageTitle: qsTr(suggestionView.selectedItem.label); isRoot: false; }
            PropertyChanges { target: searchField; visible: false; }
            PropertyChanges { target: suggestionView; visible: false; }
            PropertyChanges { target: searchView; visible: true; model: poiModel; }
            PropertyChanges { target: noData; visible: poiModel.noDataFound; }
        },
        State {
            name: "place"
            PropertyChanges { target: searchPage; pageTitle: qsTr("Search Place");  isRoot: false; }
            PropertyChanges { target: searchField; visible: true; }
            PropertyChanges { target: suggestionView; visible: false; }
            PropertyChanges { target: searchView; visible: true; model: placeModel; }
            PropertyChanges { target: noData; visible: placeModel.noDataFound; }
        }
    ]

    state: "dialog"

    onStateChanged: {
        console.log("Search state changed: "+ state);
    }

    signal selectLocation(LocationEntry location, double lat, double lon, string label)
    signal showPosition(double lat, double lon)

    onPopped: {
        // a slot could be connected to signal, waiting a selection
        // trigger the signal for null
        selectLocation(null, NaN, NaN, "");
    }

    onGoUpClicked: {
        selectedLocation = null;
        searchPage.state = "dialog";
        searchField.clear();
    }

    header: Item {
        width: searchPage.width
        height: units.gu(8)

        SearchField {
            id: searchField
            width: parent.width
            anchors.centerIn: parent
            onDisplayTextChanged: {
                var pattern = displayText.trim();
                if (pattern.length > 0) {
                    if (searchPage.state === "dialog") {
                        // show the places
                        searchPage.state = "place";
                    }
                    searchPage.searchPattern = pattern;
                } else if (searchPage.state !== "dialog") {
                    // cancel search and show suggestions
                    searchPage.searchPattern = "";
                    searchPage.selectedLocation = null;
                    searchPage.state = "dialog";
                }
                postponeSearch.restart();
            }
            Component.onCompleted: {
                forceActiveFocus();
            }
            onAccepted: {
                if (searchPage.state !== "dialog") {
                    // accept the top item without preview
                    var selectedLocation = searchView.model.get(0);
                    if (selectedLocation !== null) {
                        //@TODO: save history
                        if (selectedLocation.label !== "") {
                            selectLocation(selectedLocation, selectedLocation.lat, selectedLocation.lon, selectedLocation.label);
                        } else {
                            selectLocation(selectedLocation, selectedLocation.lat, selectedLocation.lon,
                                           Converter.readableCoordinatesGeocaching(selectedLocation.lat, selectedLocation.lon));
                        }
                        stackView.pop();
                    }
                }
            }
        }

        Column {
            visible: !searchField.visible
            width: parent.width - units.gu(4)
            anchors.centerIn: parent
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Up to distance %1").arg(Converter.readableDistance(distanceTypes[distanceSelector.value]))
                color: styleMap.view.foregroundColor
                font.pointSize: units.fs("small")
            }
            Slider {
                id: distanceSelector
                width: parent.width
                height: units.gu(3)
                snapMode: Slider.SnapAlways
                stepSize : 1
                from: 0
                to: distanceTypes.length - 1
                value: 0
                onValueChanged: {
                    postponeMaxDistance.restart();
                }
                Timer {
                    id: postponeMaxDistance
                    interval: 1000
                    onTriggered: {
                        var distance = distanceTypes[distanceSelector.value];
                        if (poiModel.maxDistance !== distance) {
                            poiModel.maxDistance = distance;
                        }
                    }
                }
            }
        }
    }

    property string searchPattern: ""

    Timer {
        id: postponeSearch
        interval: 1000
        property string _old: ""
        onTriggered: {
            if (searchPattern !== _old) {
                _old = searchPattern;
                if (searchPattern.length > 2) {
                    console.log("Search expression: \"" + searchPattern + "\"");
                    placeModel.pattern = searchPattern;
                } else {
                    placeModel.pattern = "";
                }
                // reset current of the search view
                searchView.currentIndex = -1;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////
    ////
    //// Suggestions

    /**
     * Inspired by Search.qml in OSMScout
     *
     */
    ListModel {
        id: poiTypesModel
        // amenities
        ListElement {
            label: QT_TR_NOOP("Restaurant"); distanceId: 0; iconType: "amenity_restaurant";
            types: "amenity_restaurant amenity_restaurant_building"; }
        ListElement {
            label: QT_TR_NOOP("Fast Food"); distanceId: 0; iconType: "amenity_fast_food";
            types: "amenity_fast_food amenity_fast_food_building"; }
        ListElement {
            label: QT_TR_NOOP("Cafe"); distanceId: 0; iconType: "amenity_cafe";
            types: "amenity_cafe amenity_cafe_building"; }
        ListElement {
            label: QT_TR_NOOP("Pub"); distanceId: 0; iconType: "amenity_pub";
            types: "amenity_pub amenity_pub_building"; }
        ListElement {
            label: QT_TR_NOOP("Bar"); distanceId: 0; iconType: "amenity_bar";
            types: "amenity_bar amenity_bar_building"; }
        ListElement {
            label: QT_TR_NOOP("ATM"); distanceId: 0; iconType: "amenity_atm";
            types: "amenity_atm"; }
        ListElement {
            label: QT_TR_NOOP("Drinking water"); distanceId: 0; iconType: "amenity_drinking_water";
            types: "amenity_drinking_water"; }
        ListElement {
            label: QT_TR_NOOP("Toilets"); distanceId: 0; iconType: "amenity_toilets";
            types: "amenity_toilets"; }

        // public transport
        ListElement {
            label: QT_TR_NOOP("Public transport stop"); distanceId: 0; iconType: "railway_tram_stop";
            types: "railway_station railway_subway_entrance railway_tram_stop highway_bus_stop railway_halt amenity_ferry_terminal"; }

        ListElement {
            label: QT_TR_NOOP("Fuel"); distanceId: 3; iconType: "amenity_fuel";
            types: "amenity_fuel amenity_fuel_building"; }
        ListElement {
            label: QT_TR_NOOP("Pharmacy"); distanceId: 3; iconType: "amenity_pharmacy";
            types: "amenity_pharmacy"; }
        ListElement {
            label: QT_TR_NOOP("Accomodation"); distanceId: 3; iconType: "tourism_hotel";
            types: "tourism_hotel tourism_hotel_building tourism_hostel tourism_hostel_building tourism_motel tourism_motel_building tourism_alpine_hut tourism_alpine_hut_building"; }
        ListElement {
            label: QT_TR_NOOP("Camp"); distanceId: 3;
            iconType: "tourism_camp_site"; types: "tourism_camp_site tourism_caravan_site"; }
        ListElement {
            label: QT_TR_NOOP("Castle, Manor"); distanceId: 3; iconType: "historic_castle";
            types: "historic_castle historic_castle_building historic_manor historic_manor_building historic_ruins historic_ruins_building"; }
        ListElement {
            label: QT_TR_NOOP("Spring"); distanceId: 1; iconType: "natural_spring";
            types: "natural_spring"; }

        // and somethig for fun
        ListElement {
            label: QT_TR_NOOP("Via ferrata route"); distanceId: 5; iconType: "natural_peak";
            types: "highway_via_ferrata_easy highway_via_ferrata_moderate highway_via_ferrata_difficult highway_via_ferrata_extreme"; }
    }

    MapListView {
        id: suggestionView
        anchors.fill: parent
        height: contentHeight
        spacing: 0
        clip: true
        model: poiTypesModel
        delegate: MouseArea {
            height: rowPoiType.height + units.gu(1)
            width: parent.width
            Rectangle {
                anchors.fill: parent
                visible: parent.pressed
                color: styleMap.view.highlightedColor
                opacity: 0.2
            }
            Row {
                id: rowPoiType
                anchors.verticalCenter: parent.verticalCenter
                POIIcon {
                  id: poiIcon
                  anchors.verticalCenter: parent.verticalCenter
                  poiType: iconType
                  color: styleMap.view.foregroundColor
                  width: units.gu(6)
                  height: units.gu(6)
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Label {
                        id: labelLabel
                        width: suggestionView.width - poiIcon.width - units.gu(1)
                        font.pointSize: units.fs("medium")
                        color: styleMap.view.primaryColor
                        textFormat: Text.StyledText
                        text: qsTr(label)
                    }
                    Label {
                        id: descriptionLabel
                        width: suggestionView.width - poiIcon.width - units.gu(1)
                        font.pointSize: units.fs("small")
                        wrapMode: Text.WordWrap
                        visible: distanceId > 0
                        text: qsTr("Up to distance %1").arg(Converter.readableDistance(distanceTypes[distanceId]))
                        color: styleMap.view.highlightedColor
                    }
                }
            }
            onClicked: {
                suggestionView.selectedItem = poiTypesModel.get(index);
                poiModel.maxDistance = distanceTypes[distanceId];
                poiModel.types = types.split(" ");
                distanceSelector.value = distanceId; // reset the distance selector
                searchPage.state = "poi";
            }
        }
        property var selectedItem: null
    }


    /////////////////////////////////////////////////////////////////////
    ////
    //// Search location

    MapListView {
        id: searchView
        anchors.fill: parent
        anchors.bottomMargin: mapPreview.height
        height: contentHeight
        spacing: 0
        clip: true
        model: null
        delegate: MouseArea {
            height: rowEntry.implicitHeight + units.gu(1)
            width: parent.width
            Rectangle {
                anchors.fill: parent
                visible: parent.pressed || index === searchView.currentIndex
                color: styleMap.view.highlightedColor
                opacity: 0.2
            }
            Row {
                id: rowEntry
                anchors.verticalCenter: parent.verticalCenter
                POIIcon {
                  id: entryIcon
                  anchors.verticalCenter: parent.verticalCenter
                  poiType: type
                  color: styleMap.view.foregroundColor
                  width: units.gu(6)
                  height: units.gu(6)
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Label {
                        id: entryLabel
                        width: searchView.width - entryIcon.width - units.gu(1)
                        font.pointSize: units.fs("medium")
                        color: styleMap.view.primaryColor
                        textFormat: Text.StyledText
                        text: (type === "coordinate") ? Converter.readableCoordinatesGeocaching(lat, lon)
                                                      : label === "" ? qsTr("Unnamed")
                                                      : label
                    }
                    Label {
                        id: entryRegion
                        width: searchView.width - entryIcon.width - units.gu(1)
                        wrapMode: Text.WordWrap
                        text: {
                            var str = "";
                            if (region.length > 0) {
                                var start = 0;
                                while (start < region.length && region[start] === label) {
                                    start++;
                                }
                                if (start < region.length) {
                                    str = region[start];
                                    for (var i = start + 1; i < region.length; ++i) {
                                        str += ", "+ region[i];
                                    }
                                } else {
                                    str = region[0];
                                }
                            }
                            return str;
                        }
                        color: styleMap.view.secondaryColor
                        font.pointSize: units.fs("medium")
                        visible: region.length > 0
                    }
                    Label{
                        id: entryDistance
                        width: searchView.width - entryIcon.width - units.gu(1)
                        color: styleMap.view.highlightedColor
                        font.pointSize: units.fs("medium")
                        text: Converter.readableDistance(distance) + " " + Converter.readableBearing(bearing)
                    }
                }
            }
            onClicked: {
                forceActiveFocus();
                searchView.currentIndex = index;
                var selected = searchView.model.get(index);
                selectedIsFavorite = FavoritesModel.isFavorite(selected.lat, selected.lon);
                selectedLocation = selected;
            }
        }
    }

    NearPOIModel {
        id: poiModel
        lat: searchCenterLat
        lon: searchCenterLon
        resultLimit: 100

        property bool noDataFound: false
        onSearchingChanged: {
            if (searching) {
                noDataFound = false;
            } else {
                noDataFound = (rowCount() === 0);
            }
        }
    }

    LocationListModel {
        id: placeModel
        lat: searchCenterLat
        lon: searchCenterLon
        resultLimit: 100

        property bool noDataFound: false
        onSearchingChanged: {
            if (searching) {
                noDataFound = false;
            } else {
                noDataFound = (rowCount() === 0);
            }
        }

        /**
         * Inspired by Search.qml in OSMScout
         * compute rank for location, it should be in range 0~1
         */
        function locationRank(loc){
            var rank = 1;
            if (loc.type ===  "coordinate") {
                return rank;
            } else if (loc.type === "object") {
                if (loc.objectType === "boundary_country") {
                    rank *= 1;
                } else if (loc.objectType === "boundary_state") {
                    rank *= 0.93;
                } else if (loc.objectType === "boundary_administrative" ||
                           loc.objectType === "place_town") {
                    rank *= 0.9;
                } else if (loc.objectType === "highway_residential" ||
                           loc.objectType === "address") {
                    rank *= 0.8;
                } else if (loc.objectType === "railway_station" ||
                           loc.objectType === "railway_tram_stop" ||
                           loc.objectType === "railway_subway_entrance" ||
                           loc.objectType === "highway_bus_stop"
                          ) {
                    rank *= 0.7;
                } else {
                    rank *= 0.5;
                }
                var distance = loc.distanceTo(searchCenterLat, searchCenterLon);
                rank *= 1 / Math.log(distance / 1000 + Math.E);
                return rank;
            }
            return 0;
        }

        compare: function(a, b){
            return locationRank(b) - locationRank(a);
        }

        equals: function(a, b){
            if (a.objectType === b.objectType &&
                a.distanceTo(b.lat, b.lon) < 300 &&
                a.distanceTo(searchCenterLat, searchCenterLon) > 3000
                ) {
                return true;
            }
            return false;
        }
    }


    property LocationEntry selectedLocation: null
    property int selectedIsFavorite: 0

    Loader {
        id: mapPreview
         // active the preview on selection
        active: selectedLocation !== null && !searchField.fieldFocus
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

                onTap: {
                    // on tap change the location
                    preview.mark.selected = false;  // not the selected location
                    preview.mark.lat = lat;
                    preview.mark.lon = lon;
                    preview.mark.label = Converter.readableCoordinatesGeocaching(lat, lon);
                    preview.mark.type = "";
                    preview.isFavoriteMark = FavoritesModel.isFavorite(lat, lon);
                    locationInfoModel.setLocation(lat, lon);
                    showCoordinates(lat, lon);
                    addPositionMark(0, lat, lon);
                }
            }

            property int isFavoriteMark: 0
            property QtObject mark: QtObject {
                property bool selected: false   // is the selected location
                property double lat: 0.0        // latitude
                property double lon: 0.0        // longitude
                property string label: ""       // label
                property string type: ""        // type
            }

            LocationInfoModel{
                id: locationInfoModel
                onReadyChange: {
                    if (ready && rowCount() > 0) {
                        var str = "";
                        var mi = index(0, 0)
                        str = data(mi, LocationInfoModel.LabelRole);
                        if (str === "")
                            str = data(mi, LocationInfoModel.AddressRole);
                        if (str === "")
                            str = data(mi, LocationInfoModel.PoiRole);
                        if (str !== "")
                            preview.mark.label = str;
                        preview.mark.type = data(mi, LocationInfoModel.TypeRole);
                    }
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
                    selectedLocation = null; // deactivate the preview
                }
            }
            MapIcon {
                id: buttonAccept
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: units.gu(1)
                source: acceptIcon
                color: "black"
                backgroundColor: "white"
                borderPadding: units.gu(1.0)
                opacity: 0.7
                label.text: acceptLabel
                label.font.pointSize: units.fs("medium")
                label.color: "black"
                height: units.gu(6)
                onClicked: {
                    if (mark.selected) {
                        selectLocation(selectedLocation, preview.mark.lat, preview.mark.lon, preview.mark.label);
                    } else {
                        selectLocation(null, preview.mark.lat, preview.mark.lon, preview.mark.label);
                    }
                    stackView.pop();
                }
            }
            MapIcon {
                id: buttonShowPosition
                visible: showPositionEnabled
                anchors.top: parent.top
                anchors.left: buttonAccept.right
                anchors.margins: units.gu(1)
                source: "qrc:/images/trip/here.svg"
                color: "black"
                backgroundColor: "white"
                borderPadding: units.gu(1.0)
                opacity: 0.7
                height: units.gu(6)
                onClicked: {
                    showPosition(preview.mark.lat, preview.mark.lon);
                    stackView.pop();
                }
            }
            MapIcon {
                id: buttonAddFavorite
                visible: addFavoriteEnabled
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: units.gu(1)
                source: "qrc:/images/trip/favourite.svg"
                color: preview.isFavoriteMark > 0 ? "deepskyblue" : "black"
                backgroundColor: "white"
                borderPadding: units.gu(1.0)
                opacity: 0.7
                height: units.gu(6)
                onClicked: {
                    if (preview.isFavoriteMark === 0) {
                        preview.isFavoriteMark = createFavorite(preview.mark.lat,
                                                                preview.mark.lon,
                                                                preview.mark.label,
                                                                preview.mark.type);
                    } else if (removeFavorite(preview.isFavoriteMark)) {
                        preview.isFavoriteMark = 0;
                    }
                }
            }
        }

        onStatusChanged: {
            if (active && status === Loader.Ready) {
                console.log("Activate map preview");
                searchPage.selectedLocationChanged.connect(showSelectedLocation);
                showSelectedLocation();
            } else if (!active) {
                console.log("Deactivate map preview");
                searchPage.selectedLocationChanged.disconnect(showSelectedLocation);
            }
        }

        function showSelectedLocation() {
            console.log("Show selected location on map preview");
            item.map.showCoordinatesInstantly(selectedLocation.lat, selectedLocation.lon);
            item.map.addPositionMark(0, selectedLocation.lat, selectedLocation.lon);
            item.map.removeAllOverlayObjects();
            // setup mark
            item.mark.selected = true;
            item.mark.lat = selectedLocation.lat;
            item.mark.lon = selectedLocation.lon;
            item.mark.type = selectedLocation.objectType;
            item.isFavoriteMark = selectedIsFavorite;
            if (selectedLocation.label !== "")
                item.mark.label = selectedLocation.label;
            else
                item.mark.label = Converter.readableCoordinatesGeocaching(selectedLocation.lat, selectedLocation.lon);
            //console.log("Selected location: \"" + item.mark.label + "\", " + item.mark.type);
        }

        Behavior on height {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad;
                onStopped: {
                    console.log("Animate is stopped");
                    searchView.positionViewAtIndex(searchView.currentIndex, ListView.Contain);
                }
            }
        }
    }

    Text {
        id: noData
        anchors.top: parent.top
        anchors.topMargin: units.gu(8)
        anchors.horizontalCenter: parent.horizontalCenter
        color: styleMap.view.foregroundColor
        font.pointSize: units.fs("large")
        text: qsTr("No data")
        visible: false
    }

    ActivitySpinner {
        id: busyIndicator
        visible: poiModel.searching || placeModel.searching
        anchors.centerIn: null
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: units.gu(8)
    }
}
