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
import QtQml 2.2
import Osmin 1.0

Item {
    readonly property alias voiceValid: internal.voiceValid
    readonly property alias voiceName: internal.voiceName
    readonly property alias voiceId: internal.voiceId
    readonly property alias voiceLang: internal.voiceLang
    readonly property alias voiceGender: internal.voiceGender
    readonly property alias voiceLicense: internal.voiceLicense

    signal voiceSelected()

    function select(name) {
        for (var i = 0; i < voiceModel.rowCount(); ++i) {
            if (name === voiceModel.data(voiceModel.index(i, 0), InstalledVoicesModel.NameRole)) {
                internal.select(i);
                console.log("Selected voice: \"" + voiceName + "\"");
                return true;
            }
        }
        return false;
    }

    function testVoice() {
        if (voiceValid) {
            var indexObj = voiceModel.index(voiceId, 0);
            var samples = ["Arrive.ogg"];
            voiceModel.playSample(indexObj, samples);
        }
    }

    InstalledVoicesModel {
        id: voiceModel
    }

    QtObject {
        id: internal
        property bool voiceValid: false
        property string voiceName
        property int voiceId
        property string voiceLang
        property string voiceGender
        property string voiceLicense

        function select(row) {
            var index = voiceModel.index(row, 0);
            voiceModel.select(index);
            if (voiceModel.data(index, InstalledVoicesModel.SelectedRole)) {
                voiceId = row;
                voiceName = voiceModel.data(index, InstalledVoicesModel.NameRole);
                voiceLang = voiceModel.data(index, InstalledVoicesModel.LangRole);
                voiceGender = voiceModel.data(index, InstalledVoicesModel.GenderRole);
                voiceLicense = voiceModel.data(index, InstalledVoicesModel.LicenseRole);
                voiceValid = (row > 0 ? voiceModel.data(index, InstalledVoicesModel.ValidRole) : false);
                voiceSelected();
            }
        }
    }

    Component.onCompleted: {
        // get first available as default
        if (voiceModel.rowCount() > 1 && !select(settings.voiceName)) {
            internal.select(1);
            console.log("Selected voice: \"" + voiceName + "\"");
        }
    }
}
