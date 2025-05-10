/*
 * Copyright (C) 2025
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
#ifndef TRACKPROFILECHART_H
#define TRACKPROFILECHART_H

#include <osmscoutclientqt/ElevationChartWidget.h>

class TrackProfileChart : public osmscout::ElevationChartWidget
{
  Q_OBJECT

signals:
  void redraw();

public:
  TrackProfileChart(QQuickItem* parent = nullptr);
  ~TrackProfileChart() override = default;

  Q_INVOKABLE void draw(QObject * way);
};

#endif // TRACKPROFILECHART_H
