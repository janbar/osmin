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
import QtQml 2.2
import Osmin 1.0

Item {
    id: navigator

    property MapPosition position
    property OverlayManager overlayManager
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
        overlayManager.addRoute(routeWay);
        overlayManager.addMarkEnd(destination.lat, destination.lon);
        navigator.vehicle = vehicle;
        navigator.destination = destination;
        navigationModel.locationChanged(position._posValid, position._lat, position._lon, position._accValid, position._acc);
        navigationModel.route = route;
        suspended = false;
        started();
    }

    function stop() {
        delayReroute.resetAndStop();
        suspended = true;
        destination = null;
        navigationModel.route = null;
        routing.cancel();
        overlayManager.removeRoute();
        overlayManager.removeMarkEnd();
        stopped();
    }

    function reroute() {
        if (destination !== null && routing.ready) {
            var location = routing.locationEntryFromPosition(position._lat, position._lon);
            routing.setStartAndTarget(location, navigator.destination, navigator.vehicle);
            delayReroute.start();
        }
    }

    // passing signal args to the slot of NavigationModel
    function locationChanged(valid, lat, lon, accValid, acc, alt) {
        navigationModel.locationChanged(valid, lat, lon, accValid, acc);
    }

    onSuspendedChanged: {
        if (!suspended) {
            if (!connected) {
                console.log("Navigator: Connect on positionChanged");
                position.dataUpdated.connect(locationChanged);
                connected = true;
            }
        } else {
            if (connected) {
                console.log("Navigator: Disconnect from positionChanged");
                position.dataUpdated.disconnect(locationChanged);
                connected = false;
            }
        }
    }

    Timer {
        id: delayReroute
        interval: 5000
        property bool routeRequested: false
        onTriggered: {
            if (routeRequested) {
                routeRequested = false;
                navigator.reroute();
            }
        }
        function resetAndStop() {
            routeRequested = false;
            stop();
        }
    }

    RoutingListModel {
        id: routing
        onComputingChanged: {
            console.log("Navigator: route.computingChanged");
            if (routing.ready && navigator.destination) {
                delayReroute.start(); // wait 5 sec before next reroute
                if (routing.count > 0) {
                    console.log("Navigator: route.count = " + routing.count);
                    overlayManager.addRoute(routing.routeWay);
                    navigationModel.locationChanged(position._posValid, position._lat, position._lon, position._accValid, position._acc);
                    navigationModel.route = routing.route;
                    navigator.suspended = false;
                } else {
                    console.log("Navigator: Request to reroute again");
                    delayReroute.resetAndStop();
                    navigator.rerouteRequested();
                }
            }
        }
        onRouteFailed: {
            console.log("Navigator: route.routeFailed: " + reason);
            delayReroute.resetAndStop();
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

        onRerouteRequest: {
            if (!delayReroute.running) {
                console.log("Navigator: Requesting reroute");
                navigator.reroute();
            } else {
                delayReroute.routeRequested = true;
                console.log("Navigator: Requesting reroute after timeout");
            }
        }

        onTargetReached: {
            console.log("Navigator: Target reached");
            navigator.stop();
            navigator.targetReached(targetDistance, targetBearing);
        }
    }
}
