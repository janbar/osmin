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
    property bool routeRunning: runningRouting !== null

    property alias vehiclePosition: navigationModel.vehiclePosition
    property alias nextRouteStep: navigationModel.nextRouteStep
    property alias laneTurns: navigationModel.laneTurns
    property alias laneTurn: navigationModel.laneTurn
    property alias laneSuggested: navigationModel.laneSuggested
    property alias suggestedLaneFrom: navigationModel.suggestedLaneFrom
    property alias suggestedLaneTo: navigationModel.suggestedLaneTo
    property alias currentSpeed: navigationModel.currentSpeed
    property alias maximumSpeed: navigationModel.maxAllowedSpeed
    property alias remainingDistance: navigationModel.remainingDistance
    property string arrivalEstimate: ""

    property alias model: navigationModel

    // the routing model to navigate
    property RoutingListModel routingModel: null

    signal rerouteRequested
    signal targetReached(double targetDistance, string targetBearing)
    signal started
    signal stopped

    function setup(vehicle, routingModel, destination) {
        // change routing model
        navigator.vehicle = vehicle;
        navigator.routingModel = routingModel;
        navigator.destination = destination;
        // show route overlays
        overlayManager.addRoute(routingModel.routeWay);
        overlayManager.addMarkEnd(destination.lat, destination.lon);
        // setup navigation module
        navigationModel.locationChanged(position._posValid, position._lat, position._lon, position._accValid, position._acc);
        navigationModel.route = routingModel.route;
        // start navigation
        suspended = false;
        started();
    }

    Timer {
        id: delayReroute
        interval: 5000
        onTriggered: {
        }
    }

    function stop() {
        cancelRouting();
        delayReroute.stop();
        suspended = true;
        destination = null;
        navigationModel.route = null;
        overlayManager.removeRoute();
        overlayManager.removeMarkEnd();
        stopped();
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
                delayReroute.start(); // wait 5 sec before next reroute
                navigator.reroute();
            }
        }

        onTargetReached: function(targetBearing, targetDistance) {
            console.log("Navigator: Target reached");
            navigator.targetReached(targetDistance, targetBearing);
        }
    }

    property Component routingComponent
    property NavigatorRouting runningRouting: null
    property NavigatorRouting currentRouting: null

    Component.onCompleted: {
        // prepare the routing component
        routingComponent = Qt.createComponent("NavigatorRouting.qml");
    }
    Component.onDestruction: {
        cancelRouting();
        navigator.routingModel = null;
        if (currentRouting)
            currentRouting.destroy();
        if (connected)
            position.dataUpdated.disconnect(locationChanged);
    }

    // cancel running routing
    function cancelRouting() {
        if (runningRouting) {
            runningRouting.routing.cancel();
            runningRouting.destroy();
            runningRouting = null;
        }
    }

    // create routing instance and reroute
    function reroute() {
        if (destination && !runningRouting) {
            runningRouting = routingComponent.createObject(navigator);
            runningRouting.routeComputed.connect(routeComputed);
            runningRouting.routeNoRoute.connect(routeNoRoute);
            runningRouting.routeFailed.connect(routeFailed);
            runningRouting.setup(navigator);
        }
    }

    // on routing completion
    function routeComputed() {
        if (navigator.destination) {
            delayReroute.start(); // wait 5 sec before next reroute
            navigator.routingModel = runningRouting.routing; // update routing model
            if (currentRouting)
                currentRouting.destroy();
            currentRouting = runningRouting;
            runningRouting = null;
            console.log("Navigator: route has " + navigator.routingModel.count + " steps");
            overlayManager.addRoute(navigator.routingModel.routeWay);
            navigationModel.locationChanged(position._posValid, position._lat, position._lon, position._accValid, position._acc);
            navigationModel.route = navigator.routingModel.route;
            navigator.suspended = false;
        } else {
            runningRouting.destroy(); // destroy unusable obj
            runningRouting = null;
        }
    }

    function routeNoRoute() {
        navigator.routingModel = runningRouting.routing; // update routing model
        if (currentRouting)
            currentRouting.destroy();
        currentRouting = runningRouting;
        runningRouting = null;
        console.log("Navigator: Request to reroute again");
        delayReroute.stop();
        navigator.rerouteRequested();
    }

    function routeFailed(reason) {
        runningRouting.destroy(); // destroy unusable obj
        runningRouting = null;
        delayReroute.stop();
        navigator.rerouteRequested();
    }
}
