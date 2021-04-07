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

QtObject {
    // the map to handle
    property Map map

    // constants
    readonly property int id_ROUTE: 0       // the route overlay
    readonly property int id_RECORDING: 1   // the recording overlay
    readonly property int id_DEPARTURE: 4   // the departure of route
    readonly property int id_ARRIVAL: 5     // the arrival of route
    readonly property int id_MARK_POINT: 8  // range start for marks
    readonly property int id_WAY_POINT: 128 // range start for way points
    readonly property int id_COURSE: 256    // range start for courses

    // add the route on the map
    function addRoute(routeWay) {
        map.addOverlayObject(id_ROUTE, routeWay);
    }

    // remove the route on the map
    function removeRoute() {
        map.removeOverlayObject(id_ROUTE);
    }

    // add the START mark on the map
    function addMarkStart(lat, lon) {
        map.addPositionMark(id_DEPARTURE, lat, lon);
    }

    // remove the START mark on the map
    function removeMarkStart() {
        map.removePositionMark(id_DEPARTURE);
    }

    // add the END mark on the map
    function addMarkEnd(lat, lon) {
        map.addPositionMark(id_ARRIVAL, lat, lon);
    }

    // remove the END mark on the map
    function removeMarkEnd() {
        map.removePositionMark(id_ARRIVAL);
    }

    // add a mark on the map
    function addMark(id, lat, lon) {
        map.addPositionMark(id_MARK_POINT + id, lat, lon);
    }

    // remove a mark on the map
    function removeMark(id) {
        map.removePositionMark(id_MARK_POINT + id);
    }

    // add a way point on the map
    function addWayPoint(id, lat, lon) {
        var wpt = map.createOverlayNode("_waypoint");
        wpt.addPoint(lat, lon);
        wpt.name = "Pos: " + lat.toFixed(4) + " " + lon.toFixed(4);
        map.addOverlayObject(id_WAY_POINT + id, wpt);
    }

    // remove a way point on the map
    function removeWayPoint(id) {
        map.removeOverlayObject(id_WAY_POINT + id);
    }

    // add the recording track on the map
    function addRecording(overlayObject) {
        map.addOverlayObject(id_RECORDING, overlayObject);
    }

    // remove the recording track on the map
    function removeRecording() {
        map.removeOverlayObject(id_RECORDING);
    }

    // keep track of courses added on the map
    property var courseObjects: []

    // add a course on the map
    function addCourse(bid, overlays) {
        if (overlays.length > 0) {
            for (var i = 0; i < overlays.length; ++i) {
                console.log("Add overlay " + (bid + i) + " : " + overlays[i].objectType + " , " + overlays[i].name);
                map.addOverlayObject((bid + i), overlays[i]);
            }
            courseObjects.push({ "bid": bid, "overlays": overlays});
            return true;
        }
        return false;
    }

    // remove a previously added course on the map
    function removeCourse(bid) {
        for (var c = 0; c < courseObjects.length; ++c) {
            if (courseObjects[c].bid === bid) {
                var len = courseObjects[c].overlays.length;
                for (var i = 0; i < len; ++i) {
                    console.log("Remove overlay " + (bid + i));
                    map.removeOverlayObject((bid + i));
                }
                courseObjects.splice(c, 1);
                return true;
            }
        }
        return false;
    }

    // remove all previously added courses on the map
    function removeAllCourses() {
        for (var c = 0; c < courseObjects.length; ++c) {
            var bid = courseObjects[c].bid;
            var len = courseObjects[c].overlays.length;
            for (var i = 0; i < len; ++i) {
                console.log("Remove overlay " + (bid + i));
                map.removeOverlayObject((bid + i));
            }
        }
        courseObjects = [];
    }
}
