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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQml 2.2
import Osmin 1.0

Page {
    id: bannerPage
    objectName: "bannerPage"
    property string pageTitle: "Banner"
    property bool isRoot: true

    InstalledMapsModel {
        id: installedMaps
    }

    Component.onCompleted: {
        if (installedMaps.rowCount() === 0)
            mainView.launcherMode = 1;  // launch the welcome page
        else
            mainView.launcherMode = 2;  // run quick startup
    }
}
