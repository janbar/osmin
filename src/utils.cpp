
#include "utils.h"
#include <cmath>

using namespace osmin;

Utils::Utils(QObject* parent)
: QObject(parent)
{
}

QString Utils::normalizedInputString(const QString& str)
{
  QString ret;
  QString tmp = str.normalized(QString::NormalizationForm_D);
  ret.reserve(tmp.size());
  int pcat = QChar::Separator_Space;
  for (QString::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
  {
    int cat = it->category();
    if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining)
    {
      if (cat != QChar::Separator_Space || pcat != QChar::Separator_Space)
        ret.append(*it);
      pcat = cat;
    }
  }
  if (!ret.isEmpty() && pcat == QChar::Separator_Space)
    ret.truncate(ret.length() - 1);
  return ret;
}

#define DEGTORAD(a) ((a)*M_PI/180.0)

/**
 * Calculating basic cost for the A* algorithm based on the
 * spherical distance of two points on earth.
 */
double Utils::sphericalDistance(double aLat, double aLon, double bLat, double bLon)
{
  double r = 6371010.0; // Average radius of earth
  double aLatRad=DEGTORAD(aLat);
  double bLatRad=DEGTORAD(bLat);
  double dLat=DEGTORAD(bLat-aLat);
  double dLon=DEGTORAD(bLon-aLon);
  double sindLonDiv2=sin(dLon/2);
  double aa = sin(dLat/2)*sin(dLat/2)+
      cos(aLatRad)*cos(bLatRad)*
      sindLonDiv2*sindLonDiv2;
  double c = 2*atan2(sqrt(aa),sqrt(1-aa));
  return r*c;
}
