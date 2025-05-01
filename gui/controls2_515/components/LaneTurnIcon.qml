/*
 * Copyright (C) 2025
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
import QtGraphicalEffects 1.0

Item {
    id: laneTurnIcon

    property alias color: iconFill.color
    property string turnType: 'unknown'
    property bool suggested: false
    property string suggestedTurn: 'unknown'
    property string unknownTypeIcon: 'empty'
    property int roundaboutExit: -1

    Image {
        id: img
        source: typeIcon()
        sourceSize.height: parent.height
        sourceSize.width: parent.width
    }

    Rectangle {
        height: parent.height
        width: units.dp(1)
        color: parent.color
        opacity: 0.3
        anchors.left: img.left
    }

    Rectangle {
        height: parent.height
        width: units.dp(1)
        color: parent.color
        opacity: 0.3
        anchors.right: img.right
    }

    Rectangle {
        id: iconFill
        visible: false
        anchors.fill: laneTurnIcon
        color: "black"
    }

    OpacityMask {
        anchors.fill: iconFill
        source: iconFill
        maskSource: img
    }

    /*
     * This is mapping libosmscout route step types and step icons.
     */
    property variant iconMapping: {
        '': 'empty',
        'none': 'empty',
        'left': 'left',
        'slight_left': 'left',
        'merge_to_left': 'left',

        'through;left': 'through_left',
        'through;slight_left': 'through_left',
        'through;sharp_left': 'through_left',

        'through;left-through': 'through_left-through',
        'through;slight_left-through': 'through_left-through',
        'through;sharp_left-through': 'through_left-through',

        'through;left-left': 'through_left-left',
        'through;slight_left-slight_left': 'through_left-left',
        'through;sharp_left-sharp_left': 'through_left-left',

        'through': 'through',

        'through;right': 'through_right',
        'through;slight_right': 'through_right',
        'through;sharp_right': 'through_right',

        'through;right-through': 'through_right-through',
        'through;slight_right-through': 'through_right-through',
        'through;sharp_right-through': 'through_right-through',

        'through;right-right': 'through_right-right',
        'through;slight_right-slight_right': 'through_right-right',
        'through;sharp_right-sharp_right': 'through_right-right',

        'right': 'right',
        'slight_right': 'right',
        'merge_to_right': 'right'
    }

    function iconUrl(icon){
        return "qrc:/images/laneturn/" + icon + (suggested ? '' : "_outline") + ".svg";
    }

    function typeIcon(type){
      var icon = iconMapping[turnType];
      if (typeof icon === 'undefined'){
          console.log("Can't find icon for type " + turnType);
          return iconUrl(unknownTypeIcon);
      }
      if (suggested) {
          var suggestedTurnIcon = iconMapping[turnType + '-' + suggestedTurn];
          if (typeof suggestedTurnIcon !== 'undefined'){
              icon = suggestedTurnIcon;
          }
      }

      return iconUrl(icon);
    }
}
