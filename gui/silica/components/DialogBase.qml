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

import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    id: dialog

    /*!
      \qmlproperty list<Object> contents
      Content will be put inside a column in the foreround of the Dialog.
    */
    default property alias _contentChildren: contentsColumn.data

    property string title: ""
    property string acceptText: qsTr("Ok")
    property string cancelText: qsTr("Cancel")

    /*!
      The question to the user.
      \qmlproperty string text
     */
    property alias text: message.text
    property alias textHorizontalAlignment: message.horizontalAlignment
    property real contentSpacing: units.gu(1)

    Column {
        id: contentsColumn
        spacing: dialog.contentSpacing
        width: parent.width

        DialogHeader {
            id: header
            title: dialog.title
            acceptText: dialog.acceptText
            cancelText: dialog.cancelText
        }
        Text {
            id: message
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            horizontalAlignment: Text.AlignHCenter
            text: dialog.text
            font.pixelSize: units.fx("medium")
            color: styleMap.dialog.foregroundColor
            wrapMode: Text.Wrap
            visible: (text !== "")
        }
    }

}
