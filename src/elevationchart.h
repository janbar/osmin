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
#ifndef ELEVATIONCHART_H
#define ELEVATIONCHART_H

#include "converter.h"

#include <QQuickPaintedItem>
#include <QColor>

class QMutex;
class GPXObjectTrack;

class ElevationChart : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(Converter* converter READ getConverter WRITE setConverter NOTIFY converterChanged REQUIRED)
  Q_PROPERTY(int sampleCount READ getSampleCount NOTIFY loaded)
  Q_PROPERTY(bool succeeded READ getSucceeded NOTIFY finished)
  Q_PROPERTY(QColor lineColor READ getLineColor WRITE setLineColor NOTIFY lineColorChanged)
  Q_PROPERTY(QColor foregroundColor READ getForegroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
  Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
  Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor NOTIFY textColorChanged)
  Q_PROPERTY(int fontSizeSM READ getFontSizeSM WRITE setFontSizeSM NOTIFY fontSizeSMChanged)
  Q_PROPERTY(int fontSizeXS READ getFontSizeXS WRITE setFontSizeXS NOTIFY fontSizeXSChanged)

signals:
  void finished();

  void loaded();
  void lineColorChanged();
  void foregroundColorChanged();
  void backgroundColorChanged();
  void textColorChanged();
  void fontSizeSMChanged();
  void fontSizeXSChanged();
  void converterChanged();

public slots:
  void onLoaded();

public:
  explicit ElevationChart(QQuickItem* parent = nullptr);
  ~ElevationChart() override;

  Q_INVOKABLE void loadGPXObjectTrack(GPXObjectTrack * obj, int sampleCount);

  void paint(QPainter *painter) override;

  Converter * getConverter() const { return m_converter; }
  void setConverter(Converter * converter);

  int getSampleCount() const { return m_samples; }
  bool getSucceeded() const { return m_succeeded; }

  QColor getLineColor() const { return m_lineColor; }
  void setLineColor(const QColor &color);

  QColor getForegroundColor() const { return m_foregroundColor; }
  void setForegroundColor(const QColor &color);

  QColor getBackgroundColor() const { return m_backgroundColor; }
  void setBackgroundColor(const QColor &color);

  QColor getTextColor() const { return m_textColor; }
  void setTextColor(const QColor &color);

  int getFontSizeSM() const { return m_fontSizeSM; }
  void setFontSizeSM(int size);

  int getFontSizeXS() const { return m_fontSizeXS; }
  void setFontSizeXS(int size);

protected:
  void reset();

protected:
  QMutex * m_lock;
  Converter * m_converter = nullptr; // required
  bool m_succeeded = false;
  int m_samples = 0;
  double m_duration = 0;
  double m_distance = 0;
  double m_minElevation = 0;
  double m_maxElevation = 0;
  QList<double> m_distances;
  QList<double> m_minElevations;
  QList<double> m_maxElevations;

  QColor m_lineColor = Qt::red;
  QColor m_textColor = Qt::gray;
  QColor m_foregroundColor = Qt::black;
  QColor m_backgroundColor = Qt::white;
  int m_fontSizeSM = 15;
  int m_fontSizeXS = 13;

  // draw objects in the chart direction, i.e from bottom to top
  void traceLine(QPainter * painter, const QRect& v, qreal x0, qreal y0, qreal x1, qreal y1, const QColor& c, qreal w);
  void traceCurve(QPainter * painter, const QRect& v, qreal x0, qreal y0, const QColor& c, const QList<qreal>& x, const QList<qreal>& y);
  void traceText(QPainter * painter, const QRect& v, const QRectF& box, const QColor& c, const QFont& f, int flag, const QString& text);
};

#endif // ELEVATIONCHART_H
