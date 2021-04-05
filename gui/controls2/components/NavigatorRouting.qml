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
    signal routeComputed()
    signal routeFailed(var reason)
    signal routeNoRoute()

    function setup(navigator) {
        var location = routing.locationEntryFromPosition(navigator.position._lat, navigator.position._lon);
        routing.setStartAndTarget(location, navigator.destination, navigator.vehicle);
    }

    property alias routing: routing

    RoutingListModel {
        id: routing
        onComputingChanged: {
            if (routing.ready) {
                console.log("Route computed");
                if (routing.count > 0)
                    routeComputed();
                else
                    routeNoRoute();
            } else {
                console.log("Route computing");
            }
        }
        onRouteFailed: {
            console.log("Route failed: " + reason);
            routeFailed(reason);
        }
    }
}
