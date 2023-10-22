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

QtObject {
    // the map to handle
    property Map map

    // add the route on the map
    function addRoute(routeWay) {
        var ovs = MapExtras.findOverlays("ROUTE", 0);
        if (ovs.length > 0)
            map.addOverlayObject(ovs[0], routeWay);
        else
            map.addOverlayObject(MapExtras.addOverlay("ROUTE", 0), routeWay);
        return true;
    }

    // remove the route on the map
    function removeRoute() {
        var ovs = MapExtras.clearOverlays("ROUTE", 0);
        ovs.forEach(function(e){ map.removeOverlayObject(e); });
        MapExtras.releaseOverlayIds(ovs);
        return true;
    }

    // add the START mark on the map
    function addMarkStart(lat, lon) {
        var ovs = MapExtras.findOverlays("ROUTE", 1);
        if (ovs.length > 0)
            map.addPositionMark(ovs[0], lat, lon);
        else
            map.addPositionMark(MapExtras.addOverlay("ROUTE", 1), lat, lon);
        return true;
    }

    // remove the START mark on the map
    function removeMarkStart() {
        var ovs = MapExtras.clearOverlays("ROUTE", 1);
        ovs.forEach(function(e){ map.removePositionMark(e); });
        MapExtras.releaseOverlayIds(ovs);
        return true;
    }

    // add the END mark on the map
    function addMarkEnd(lat, lon) {
        var ovs = MapExtras.findOverlays("ROUTE", 2);
        if (ovs.length > 0)
            map.addPositionMark(ovs[0], lat, lon);
        else
            map.addPositionMark(MapExtras.addOverlay("ROUTE", 2), lat, lon);
        return true;
    }

    // remove the END mark on the map
    function removeMarkEnd() {
        var ovs = MapExtras.clearOverlays("ROUTE", 2);
        ovs.forEach(function(e){ map.removePositionMark(e); });
        MapExtras.releaseOverlayIds(ovs);
        return true;
    }

    // add a mark on the map
    function addMark(id, lat, lon) {
        var ovs = MapExtras.findOverlays("MARK", id);
        if (ovs.length > 0)
            map.addPositionMark(ovs[0], lat, lon);
        else
            map.addPositionMark(MapExtras.addOverlay("MARK", id), lat, lon);
        return true;
    }

    // remove a mark on the map
    function removeMark(id) {
        var ovs = MapExtras.clearOverlays("MARK", id);
        ovs.forEach(function(e){ map.removePositionMark(e); });
        MapExtras.releaseOverlayIds(ovs);
        return true;
    }

    // add a way point on the map
    function addWayPoint(id, lat, lon) {
        var wpt = map.createOverlayNode("_waypoint");
        wpt.addPoint(lat, lon);
        wpt.name = "Pos: " + lat.toFixed(4) + " " + lon.toFixed(4);
        var ovs = MapExtras.findOverlays("WAYPOINT", id);
        if (ovs.length > 0)
             map.addOverlayObject(ovs[0], wpt);
        else
            map.addOverlayObject(MapExtras.addOverlay("WAYPOINT", id), wpt);
        return true;
    }

    // remove a way point on the map
    function removeWayPoint(id) {
        var ovs = MapExtras.clearOverlays("WAYPOINT", id);
        ovs.forEach(function(e){ map.removeOverlayObject(e); });
        MapExtras.releaseOverlayIds(ovs);
        return true;
    }

    // add the recording track on the map
    function addRecording(overlayObject) {
        var ovs = MapExtras.findOverlays("RECORDING", 0);
        if (ovs.length > 0)
             map.addOverlayObject(ovs[0], overlayObject);
        else
            map.addOverlayObject(MapExtras.addOverlay("RECORDING", 0), overlayObject);
        return true;
    }

    // remove the recording track on the map
    function removeRecording() {
        var ovs = MapExtras.clearOverlays("WAYPOINT", id);
        ovs.forEach(function(e){ map.removeOverlayObject(e); });
        MapExtras.releaseOverlayIds(ovs);
        return true;
    }

    // add a course on the map
    function addCourse(bid, overlays) {
        if (overlays.length > 0) {
            for (var i = 0; i < overlays.length; ++i) {
                console.log("Add overlay " + (bid) + " : " + overlays[i].objectType + " , " + overlays[i].name);
                map.addOverlayObject(MapExtras.addOverlay("COURSE", bid), overlays[i]);
            }
            return true;
        }
        return false;
    }

    // remove a previously added course on the map
    function removeCourse(bid) {
        var ovs = MapExtras.clearOverlays("COURSE", bid);
        ovs.forEach(function(e){ map.removeOverlayObject(e); });
        MapExtras.releaseOverlayIds(ovs);
        return true;
    }

    // remove all previously added courses on the map
    function removeAllCourses() {
        var keys = MapExtras.findOverlayKeys("COURSE");
        keys.forEach(function(k){
            var ovs = MapExtras.clearOverlays("COURSE", k);
            console.log("Remove overlay " + (k));
            ovs.forEach(function(e){ map.removeOverlayObject(e); });
            MapExtras.releaseOverlayIds(ovs);
        });
        return true;
    }
}
