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
import Osmin 1.0

Item {
    property Map map
    signal begin
    signal finished

    readonly property alias isRunning: worker.running

    function rotateTo(angle) {
        worker.stop();
        worker.angle = Math.round(angle * 1000.0);
        worker.start();
        begin();
    }

    function stop() {
        worker.stop();
        finished();
    }

    Timer {
        id: worker
        interval: 1000
        repeat: true
        triggeredOnStart: true
        property int angle: 0 // milli rad
        onTriggered: {
            if (Math.round(map.view.angle * 1000.0) != angle) {
                map.rotateTo(0.001 * angle);
            } else {
                stop();
                finished();
            }
        }
    }
}
