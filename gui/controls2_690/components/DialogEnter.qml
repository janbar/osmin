/*
 * Copyright (C) 2022
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
    property alias userEntry: textField.text

    // note: connect the signal reply(bool, entry) to process the response
    signal reply(bool accepted, string entry)

    onClosed: {
        reply((result === Dialog.Accepted), textField.text);
    }
    onOpened: {
        result = Dialog.Rejected;
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
            focus: true
        }
        Button {
            flat: true
            text: qsTr("Cancel")
            onClicked: {
                dialog.reject();
            }
        }
    }

    TextField {
        id: textField
        anchors {
            left: parent.left
            right: parent.right
        }
        inputMethodHints: Qt.ImhNoPredictiveText
        font.pixelSize: units.fs("medium")
        Keys.onReturnPressed: dialog.accept()
    }
}
