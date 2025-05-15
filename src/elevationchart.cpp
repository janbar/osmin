/*
 *      Copyright (C) 2025 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "elevationchart.h"
#include "gpxfilemodel.h"

#include <QMutex>
#include <QPainter>
#include <QPainterPath>

//#include <osmscoutclientqt/OSMScoutQt.h>

ElevationChart::ElevationChart(QQuickItem* parent):
    QQuickPaintedItem(parent), m_lock(new QMutex())
{
  reset();
  connect(this, &ElevationChart::loaded, this, &ElevationChart::onLoaded, Qt::QueuedConnection);
}

ElevationChart::~ElevationChart()
{
  disconnect(this, &ElevationChart::loaded, this, &ElevationChart::onLoaded);
  delete m_lock;
}

void ElevationChart::onLoaded()
{
  update();
}

void ElevationChart::loadGPXObjectTrack(GPXObjectTrack * obj, int sampleCount)
{
  QMutexLocker g(m_lock);
  reset();
  if (obj == nullptr)
  {
    emit loaded();
    return;
  }

  double length = obj->length();
  // check zero length
  if (length < 1.0)
  {
    emit loaded();
    return;
  }

  // define the number of sample to compute
  int cx = 0;
  for (auto const& segment : obj->data().segments)
    cx += segment.points.size();
  if (sampleCount < cx)
    cx = sampleCount;
  // requires at least 2 samples
  if (cx < 2)
  {
    emit loaded();
    return;
  }

  // fill elevation data from the track
  QList<double> wlistMax;
  QList<double> wlistMin;
  wlistMax.assign(cx, std::nan(""));
  wlistMin.assign(cx, std::nan(""));
  const osmscout::gpx::TrackPoint * from = nullptr;
  double minele = 0.0;
  double maxele = 0.0;
  double d = std::nan(""); // progress from start
  std::optional<osmscout::Timestamp> startTime;
  std::optional<osmscout::Timestamp> endTime;
  for (auto const& segment : obj->data().segments)
  {
    // for all segments
    for (auto const& p : segment.points)
    {
      if (std::isnan(d))
      {
        // begining
        d = 0.0;
        startTime = p.timestamp;
      }
      if (from)
      {
        endTime = p.timestamp;
        d += from->coord.GetDistance(p.coord).AsMeter();
        int idx = std::round(double(cx - 1) * d / length);
        if (idx >= cx)
        {
          d = length;
          qWarning("Truncate data out of range !!!");
          break;
        }
        if (p.elevation.has_value())
        {
          double e = p.elevation.value();
          double omax = wlistMax.at(idx);
          double omin = wlistMin.at(idx);
          if (!std::isnan(e))
          {
            if (std::isnan(omax) || std::fabs(e) > std::fabs(omax))
              wlistMax[idx] = e;
            if (std::isnan(omin) || std::fabs(e) < std::fabs(omin))
              wlistMin[idx] = e;
          }
          if (std::isnan(minele) || e < minele)
            minele = e;
          if (std::isnan(maxele) || e > maxele)
            maxele = e;
        }
        from = &p;
      }
      else
      {
        wlistMax[0] = p.elevation.value_or(std::nan(""));
        wlistMin[0] = p.elevation.value_or(std::nan(""));
        from = &p;
        minele = maxele = wlistMax[0];
      }
    }
  }

  // cleanup data
  m_distances.reserve(cx);
  m_minElevations.reserve(cx);
  m_maxElevations.reserve(cx);
  for (int idx = 0; idx < cx; ++idx)
  {
    if (!std::isnan(wlistMax[idx]))
    {
      // add a fake start index when the first sample has been discarded
      if (m_distances.empty() && idx > 0)
      {
        m_distances.push_back(0.0);
        m_minElevations.push_back(wlistMin[idx]);
        m_maxElevations.push_back(wlistMax[idx]);
        //qDebug("x = %f , y = [ %f , %f ]", m_distances.back(), m_minElevations.back(), m_maxElevations.back());
      }
      // add the valid sample
      m_distances.push_back(length * double(idx) / double(cx - 1));
      m_minElevations.push_back(wlistMin[idx]);
      m_maxElevations.push_back(wlistMax[idx]);
      //qDebug("x = %f , y = [ %f , %f ]", m_distances.back(), m_minElevations.back(), m_maxElevations.back());
    }
  }
  // add a fake end index when the last sample has been discarded
  if (!m_distances.empty() && m_distances.back() < length)
  {
    m_distances.push_back(length);
    m_minElevations.push_back(m_minElevations.back());
    m_maxElevations.push_back(m_maxElevations.back());
    //qDebug("x = %f , y = [ %f , %f ]", m_distances.back(), m_minElevations.back(), m_maxElevations.back());
  }
  wlistMax.clear();
  wlistMin.clear();

  if (startTime.has_value() && endTime.has_value())
    m_duration = std::chrono::duration<double>(endTime.value() - startTime.value()).count();
  else
    m_duration = 0;
  m_distance = length;
  m_minElevation = minele;
  m_maxElevation = maxele;
  m_samples = m_distances.size();
  //qDebug("samples = %d , distance = %f , duration = %f , minEle = %f , maxEle = %f",
  //       m_samples, m_distance, m_duration , minele, maxele);
  emit loaded();
}

void ElevationChart::paint(QPainter * painter)
{
  QMutexLocker g(m_lock);
  if (m_converter == nullptr)
  {
    m_succeeded = false;
    emit finished();
    qWarning("Unit Converter is not defined !!!");
    return;
  }

  painter->setViewport(0, 0, this->width(), this->height());
  painter->setViewTransformEnabled(false);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);
  QRect viewport = painter->viewport();
  QFont fontSM = painter->font();
  fontSM.setPixelSize(m_fontSizeSM);
  QFont fontXS = painter->font();
  fontXS.setPixelSize(m_fontSizeXS);

  if (m_samples < 2)
  {
    m_succeeded = false;
    emit finished();
    return;
  }

  QRectF bbox(0, 0, viewport.width(), 1.2*m_fontSizeSM);
  QRectF tbox(0, viewport.height() - 1.2*m_fontSizeSM, viewport.width(), 1.2*m_fontSizeSM);

  // show at least 100 meters and not less
  double delta = m_maxElevation - m_minElevation;
  if (delta < 100.0)
    delta = 100.0;
  double ratio_h = (viewport.height() - tbox.height() - bbox.height()) / delta;
  double ratio_w = viewport.width() / m_distance;

  // trace elevation curves YMax Ymin
  QList<qreal> x;
  x.reserve(m_distances.size());
  QList<qreal> ymin;
  ymin.reserve(m_distances.size());
  QList<qreal> ymax;
  ymax.reserve(m_distances.size());
  for (int i = 0; i < m_distances.size(); ++i)
  {
    x.push_back(ratio_w * m_distances[i]);
    ymin.push_back(bbox.height() + ratio_h * (m_minElevations[i] - m_minElevation));
    ymax.push_back(bbox.height() + ratio_h * (m_maxElevations[i] - m_minElevation));
    //qDebug("x=%f, y=%f", x.back(), ymax.back());
  }
  traceCurve(painter, viewport, 0, 0,  m_lineColor, x, ymax);
  traceCurve(painter, viewport, 0, 0,  m_foregroundColor, x, ymin);

  //qDebug("minEle=%f, maxEle=%f", m_minElevation, m_maxElevation);

  // trace intermediates
  double sub = 5000;
  if (delta < 50)
      sub = 10;
  else if (delta < 100)
      sub = 20;
  else if (delta < 200)
      sub = 50;
  else if (delta < 500)
      sub = 100;
  else if (delta < 1000)
      sub = 200;
  else if (delta < 2000)
      sub = 500;
  else if (delta < 5000)
      sub = 1000;
  else if (delta < 10000)
      sub = 2000;

  double ele = sub + sub * std::floor(m_minElevation / sub);
  qreal h = bbox.height() + ratio_h * (ele - m_minElevation);
  if (h > (2 * bbox.height()))
  {
      traceLine(painter, viewport, 0, h, viewport.width(), h, m_textColor, 1);
      traceText(painter, viewport, QRect(0, h, viewport.width(), m_fontSizeXS),
                m_textColor, fontXS, Qt::AlignBottom | Qt::AlignHCenter,
                m_converter->panelElevation(ele));
  }
  h += ratio_h * sub;
  ele += sub;
  if (h > (2 * bbox.height()) && h < (viewport.height() - 2 * tbox.height()))
  {
      traceLine(painter, viewport, 0, h, viewport.width(), h, m_textColor, 1);
      traceText(painter, viewport, QRect(0, h, viewport.width(), m_fontSizeXS),
                m_textColor, fontXS, Qt::AlignBottom | Qt::AlignHCenter,
                m_converter->panelElevation(ele));
  }
  h += ratio_h * sub;
  ele += sub;
  if (h < (viewport.height() - 2 * tbox.height()))
  {
      traceLine(painter, viewport, 0, h, viewport.width(), h, m_textColor, 1);
      traceText(painter, viewport, QRect(0, h, viewport.width(), m_fontSizeXS),
                m_textColor, fontXS, Qt::AlignBottom | Qt::AlignHCenter,
                m_converter->panelElevation(ele));
  }

  // trace min and max
  h = bbox.height();
  traceLine(painter, viewport, 0, h, viewport.width(), h, m_backgroundColor, 1);
  traceText(painter, viewport, bbox, m_backgroundColor,
            fontXS, Qt::AlignVCenter| Qt::AlignLeft,
            "0");
  traceText(painter, viewport, bbox, m_backgroundColor,
            fontSM, Qt::AlignTop | Qt::AlignHCenter,
            m_converter->panelElevation(m_minElevation));
  traceText(painter, viewport, bbox, m_backgroundColor,
            fontXS, Qt::AlignVCenter| Qt::AlignRight,
            m_converter->panelDistance(m_distance));

  h = bbox.height() + ratio_h * (m_maxElevation - m_minElevation);
  QRectF mbox(0, h, viewport.width(), 1.2*m_fontSizeSM);
  traceLine(painter, viewport, 0, h, viewport.width(), h, m_lineColor, 1);
  traceText(painter, viewport, mbox, m_foregroundColor,
            fontSM, Qt::AlignBottom | Qt::AlignHCenter,
            m_converter->panelElevation(m_maxElevation));
  if (m_duration > 0)
  {
    traceText(painter, viewport, tbox, m_foregroundColor,
              fontXS, Qt::AlignVCenter | Qt::AlignRight,
              m_converter->panelDurationHMS(m_duration));
  }

  m_succeeded = true;
  emit finished();
}

void ElevationChart::traceLine(QPainter * painter, const QRect& v, qreal x0, qreal y0, qreal x1, qreal y1, const QColor& c, qreal w)
{
  QPainterPath path;
  path.moveTo(x0, v.height() - y0);
  path.lineTo(x1, v.height() - y1);
  QPen pen;
  pen.setColor(c);
  pen.setWidthF(w);
  pen.setStyle(Qt::SolidLine);
  painter->setPen(pen);
  painter->drawPath(path);
}


void ElevationChart::traceCurve(QPainter * painter, const QRect& v, qreal x0, qreal y0, const QColor& c, const QList<qreal>& x, const QList<qreal>& y)
{
  QPainterPath path;
  qreal posiX = x0;
  qreal posiY = y0;
  path.moveTo(posiX, v.height() - posiY);
  for (int i = 0; i < x.size(); ++i)
  {
    posiX = x0 + x[i];
    posiY = y0 + y[i];
    path.lineTo(posiX, v.height() - posiY);
  }
  path.lineTo(posiX, v.height() - y0);
  path.lineTo(x0, v.height() - y0);
  QPen pen;
  pen.setColor(c);
  pen.setWidthF(1);
  pen.setStyle(Qt::SolidLine);
  painter->setPen(pen);
  painter->drawPath(path);
  QBrush brush;
  brush.setColor(c);
  brush.setStyle(Qt::SolidPattern);
  painter->fillPath(path, brush);
}

void ElevationChart::traceText(QPainter *painter, const QRect& v, const QRectF& box, const QColor& c, const QFont& f, int flag, const QString& text)
{
  painter->setPen(c);
  painter->setFont(f);
  painter->drawText(box.x(), v.height() - box.y() - box.height(), box.width(), box.height(), flag, text);
}

void ElevationChart::reset()
{
  m_samples = 0;
  m_distances.clear();
  m_minElevations.clear();
  m_maxElevations.clear();
  m_distance = std::nan("");
  m_duration = std::nan("");
  m_minElevation = std::nan("");
  m_maxElevation = std::nan("");
}

void ElevationChart::setConverter(Converter * converter)
{
  if (m_converter == converter)
    return;
  m_converter = converter;
  emit converterChanged();
  update();
}

void ElevationChart::setLineColor(const QColor &color)
{
  if (m_lineColor == color)
    return;
  m_lineColor = color;
  emit lineColorChanged();
  update();
}

void ElevationChart::setForegroundColor(const QColor &color)
{
  if (m_foregroundColor == color)
    return;
  m_foregroundColor = color;
  emit foregroundColorChanged();
  update();
}

void ElevationChart::setBackgroundColor(const QColor &color)
{
  if (m_backgroundColor == color)
    return;
  m_backgroundColor = color;
  emit backgroundColorChanged();
  update();
}

void ElevationChart::setTextColor(const QColor &color)
{
  if (m_textColor == color)
    return;
  m_textColor = color;
  emit textColorChanged();
  update();
}

void ElevationChart::setFontSizeSM(int size)
{
  if (m_fontSizeSM == size)
    return;
  m_fontSizeSM = size;
  emit fontSizeSMChanged();
  update();
}

void ElevationChart::setFontSizeXS(int size)
{
  if (m_fontSizeXS == size)
    return;
  m_fontSizeXS = size;
  emit fontSizeXSChanged();
  update();
}
