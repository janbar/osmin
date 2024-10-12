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

MapIcon {
    id: poiIcon

    property string poiType: "unknown"
    property string unknownTypeIcon: "marker"

    /*
     * This is mapping libosmscout types (defined in *.ost file used for import)
     * and poi icons.
     */
    property variant iconMapping: {
        'address': 'marker',

        'amenity_atm': 'bank',
        'amenity_bank_building': 'bank',
        'amenity_bank': 'bank',
        'amenity_cafe_building': 'cafe',
        'amenity_cafe': 'cafe',
        'amenity_pub': 'pub',
        'amenity_pub_building': 'pub',
        'amenity_bar': 'bar',
        'amenity_bar_building': 'bar',
        'amenity_fast_food_building': 'fast-food',
        'amenity_fast_food': 'fast-food',
        'amenity_fuel_building': 'fuel',
        'amenity_fuel': 'fuel',
        'amenity_charging_station': 'charging_station',
        'amenity_grave_yard': 'religious-christian',
        'amenity_hospital_building': 'hospital',
        'amenity_hospital': 'hospital',
        'amenity_kindergarten_building': 'playground',
        'amenity_kindergarten': 'playground',
        'amenity_parking_building': 'parking-garage',
        'amenity_parking': 'parking',
        'amenity_pharmacy': 'pharmacy',
        'amenity_post_office_building': 'post',
        'amenity_post_office': 'post',
        'amenity_post_box': 'post',
        'amenity_restaurant_building': 'restaurant',
        'amenity_restaurant': 'restaurant',
        'amenity_school_building': 'school',
        'amenity_school': 'school',
        'amenity_ferry_terminal': 'ferry',
        'amenity_toilets': 'toilet',
        'amenity_drinking_water': 'drinking-water',
        'amenity_shelter': 'shelter',

        'railway_station': 'rail',
        'railway_subway': 'rail',
        'railway_rail': 'rail',
        'highway_bus_stop': 'bus',
        'railway_tram_stop': 'rail-light',
        'railway_tram': 'rail-light',
        'aeroway_aerodrome': 'airport',
        'aeroway_terminal': 'airport',

        'leisure_building': 'building',
        'leisure_sports_centre': 'soccer',
        'leisure_stadium': 'stadium',
        'leisure_golf_course': 'golf',
        'leisure_water_park': 'swimming',
        'leisure_swimming_pool': 'swimming',
        'leisure_marina': 'harbor',
        'leisure_park': 'park',
        'leisure_garden': 'garden',
        'leisure_playground': 'playground',

        'tourism_aquarium': 'aquarium',
        'tourism_attraction_building': 'circle-stroked',
        'tourism_attraction': 'circle-stroked',
        'tourism_artwork': 'monument',
        'tourism_camp_site': 'campsite',
        'tourism_caravan_site': 'circle-stroked',
        'tourism_picnic_site': 'circle-stroked',
        'tourism_theme_park': 'circle-stroked',
        'tourism_zoo': 'zoo',
        'tourism_chalet_building': 'shelter',
        'tourism_chalet': 'shelter',
        'tourism_guest_house_building': 'circle-stroked',
        'tourism_guest_house': 'circle-stroked',
        'tourism_hostel_building': 'lodging',
        'tourism_hostel': 'lodging',
        'tourism_hotel_building': 'lodging',
        'tourism_hotel': 'lodging',
        'tourism_information_building': 'information',
        'tourism_information': 'information',
        'tourism_motel_building': 'lodging',
        'tourism_motel': 'lodging',
        'tourism_museum_building': 'museum',
        'tourism_museum': 'museum',
        'tourism_building': 'town-hall',
        'tourism': 'circle-stroked',

        'historic_castle_building': 'castle',
        'historic_castle': 'castle',
        'historic_manor_building': 'town-hall',
        'historic_manor': 'town-hall',
        'historic_monument_building': 'landmark',
        'historic_monument': 'landmark',
        'historic_memorial_building': 'landmark',
        'historic_memorial': 'landmark',
        'historic_ruins_building': 'castle',
        'historic_ruins': 'castle',
        'historic_archaeological_site': 'square-stroked',
        'historic_battlefield': 'square-stroked',
        'historic_wreck': 'square-stroked',
        'historic_building': 'town-hall',
        'historic': 'square-stroked',

        'shop': 'shop',
        'christian_church_building':'religious-christian',
        'christian_church': 'religious-christian',
        'landuse_cemetery': 'religious-christian',
        'railway_subway_entrance': 'entrance',

        'place_locality': 'circle-stroked',
        'boundary_administrative': 'square-stroked',
        'boundary_state': 'square-stroked',
        'boundary_country': 'square-stroked',
        'boundary_county': 'square-stroked',
        'place_island': 'circle-stroked',

        'landuse_farm':'farm',
        'landuse_farmyard':'farm',
        'place_hamlet': 'hamlet',
        'place_village': 'residential-community',
        'place_suburb': 'residential-community',
        'place_bigcity': 'city',
        'highway_residential': 'highway_residential',
        'highway_tertiary': 'highway_tertiary',
        'highway_unclassified': 'highway_tertiary',
        'highway_secondary': 'highway_tertiary',
        'highway_cycleway':'bicycle-share',
        'route_ferry': 'ferry',
        'building': 'building',
        'amenity_building': 'building',
        'landuse_industrial': 'industry',

        'natural_peak':'mountain',
        'waterway_stream': 'waterway_stream',
        'waterway_river': 'waterway_river',
        'waterway_canal': 'waterway_river',
        'waterway_riverbank': 'waterway_river',
        'landuse_reservoir': 'landuse_reservoir',
        'natural_water': 'natural_water',
        'natural_spring': 'water'
    }

    function iconUrl(icon) {
        return "qrc:/images/poi/" + icon + ".svg";
    }

    function typeIcon(type) {
      if (typeof iconMapping[type] === 'undefined') {
          if (type.length > 0)
              console.log("Can't find icon for type \"" + type + "\"");
          return iconUrl(unknownTypeIcon);
      }
      return iconUrl(iconMapping[type]);
    }

    source: typeIcon(poiType)
}
