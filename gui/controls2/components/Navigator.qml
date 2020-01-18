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
    id: navigator

    property MapPosition position
    readonly property bool ready: !suspended && destination !== null && destination.type !== "none"

    property LocationEntry destination
    property string vehicle: "car"
    property bool suspended: true
    property bool connected: false
    property bool routeRunning: !routing.ready

    property alias vehiclePosition: navigationModel.vehiclePosition
    property alias nextRouteStep: navigationModel.nextRouteStep
    property alias currentSpeed: navigationModel.currentSpeed
    property alias maximumSpeed: navigationModel.maxAllowedSpeed
    property alias remainingDistance: navigationModel.remainingDistance
    property string arrivalEstimate: ""

    property alias model: navigationModel

    signal rerouteRequested
    signal targetReached(double targetDistance, string targetBearing)
    signal started
    signal stopped

    function setup(vehicle, route, routeWay, destination) {
        mapView.addRoute(0, routeWay);
        mapView.addMarkEnd(destination.lat, destination.lon);
        navigator.vehicle = vehicle;
        navigator.destination = destination;
        navigationModel.locationChanged(position._posValid, position._lat, position._lon, position._accValid, position._acc);
        navigationModel.route = route;
        suspended = false;
        started();
    }

    function stop() {
        suspended = true;
        destination = null;
        navigationModel.route = null;
        routing.cancel();
        mapView.removeRoute(0);
        mapView.removeMarkEnd();
        stopped();
    }

    function reroute() {
        if (destination !== null && routing.ready) {
            var location = routing.locationEntryFromPosition(position._lat, position._lon);
            routing.setStartAndTarget(location, navigator.destination, navigator.vehicle);
        }
    }

    onSuspendedChanged: {
        if (!suspended) {
            if (!connected) {
                console.log("Navigator: Connect on positionChanged");
                position.dataUpdated.connect(navigationModel.locationChanged);
                connected = true;
            }
        } else {
            if (connected) {
                console.log("Navigator: Disconnect from positionChanged");
                position.dataUpdated.disconnect(navigationModel.locationChanged);
                connected = false;
            }
        }
    }

    Timer {
        id: delayReroute
        interval: 5000
        onTriggered: { }
    }

    RoutingListModel {
        id: routing
        onComputingChanged: {
            console.log("Navigator: route.computingChanged");
            if (routing.ready && navigator.destination) {
                delayReroute.start(); // wait 5 sec before next reroute
                if (routing.count > 0) {
                    console.log("Navigator: route.count = " + routing.count);
                    mapView.addRoute(0, routing.routeWay);
                    navigationModel.locationChanged(position._posValid, position._lat, position._lon, position._accValid, position._acc);
                    navigationModel.route = routing.route;
                    navigator.suspended = false;
                } else {
                    console.log("Navigator: Request reroute again");
                    navigator.rerouteRequested();
                }
            }
        }
        onRouteFailed: {
            console.log("Navigator: route.routeFailed: " + reason);
            navigator.rerouteRequested();
        }
    }

    NavigationModel {
        id: navigationModel

        onArrivalEstimateChanged: {
            if (!isNaN(arrivalEstimate.getTime())) {
                navigator.arrivalEstimate = Qt.formatTime(arrivalEstimate);
            } else {
                navigator.arrivalEstimate = "";
            }
        }

        onBreakRequest: {
            console.log("Navigator: Requesting break");
            navigator.suspended = true; // will be reset after rerouting
            navigator.rerouteRequested();
        }

        onRerouteRequest: {
            if (!delayReroute.running) {
                console.log("Navigator: Requesting reroute");
                navigator.reroute();
            }
        }

        onTargetReached: {
            console.log("Navigator: Target reached");
            navigator.stop();
            navigator.targetReached(targetDistance, targetBearing);
        }
    }
}
