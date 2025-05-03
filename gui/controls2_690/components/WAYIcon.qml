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

MapIcon {
    id: wayIcon

    property string stepType: "unknown"
    property string unknownTypeIcon: "information"
    property int roundaboutExit: -1
    property bool roundaboutClockwise: false

    /*
     * This is mapping libosmscout route step types and step icons.
     */
    property variant iconMapping: {
        'information': 'information',

        'start': 'start',
        'drive-along': 'drive-along',
        'target': 'target',

        'turn': 'turn',
        'turn-sharp-left': 'turn-sharp-left',
        'turn-left': 'turn-left',
        'turn-slightly-left': 'turn-slightly-left',
        'continue-straight-on': 'continue-straight-on',
        'turn-slightly-right': 'turn-slightly-right',
        'turn-right': 'turn-right',
        'turn-sharp-right': 'turn-sharp-right',

        'enter-roundabout': 'enter-roundabout',
        'leave-roundabout-1': 'leave-roundabout-1',
        'leave-roundabout-2': 'leave-roundabout-2',
        'leave-roundabout-3': 'leave-roundabout-3',
        'leave-roundabout-4': 'leave-roundabout-4',

        'enter-roundabout-lhd': 'enter-roundabout-lhd',
        'leave-roundabout-1-lhd': 'leave-roundabout-1-lhd',
        'leave-roundabout-2-lhd': 'leave-roundabout-2-lhd',
        'leave-roundabout-3-lhd': 'leave-roundabout-3-lhd',
        'leave-roundabout-4-lhd': 'leave-roundabout-4-lhd',

        'enter-motorway': 'enter-motorway',

        'change-motorway': 'leave-motorway',
        'change-motorway-left': 'leave-motorway-left',
        'change-motorway-right': 'leave-motorway-right',

        'leave-motorway': 'leave-motorway',
        'leave-motorway-left': 'leave-motorway-left',
        'leave-motorway-right': 'leave-motorway-right',

        'name-change': 'information'
    }

    function iconUrl(icon) {
        return "qrc:/images/way/" + icon + ".svg";
    }

    function typeIcon(type) {
      if (type === "leave-roundabout") {
          type += "-" + Math.max(1, Math.min(roundaboutExit, 4));
          if (roundaboutClockwise) {
            type += "-lhd"
          }
      }
      if (type === "enter-roundabout" && roundaboutClockwise) {
        type += "-lhd"
      }
      if (typeof iconMapping[type] === 'undefined') {
          if (type.length > 0)
              console.log("Can't find icon for type \"" + type + "\"");
          return iconUrl(unknownTypeIcon);
      }
      return iconUrl(iconMapping[type]);
    }

    source: typeIcon(stepType)
}
