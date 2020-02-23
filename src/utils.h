
#ifndef OSMINUTILS_H
#define OSMINUTILS_H

#include <QObject>
#include <QString>


namespace osmin
{

  class Utils : public QObject
  {
    Q_OBJECT

  public:
    explicit Utils(QObject *parent = nullptr);
    ~Utils() { }

    Q_INVOKABLE static QString normalizedInputString(const QString& str);

    Q_INVOKABLE static double sphericalDistance(double aLat, double aLon, double bLat, double bLon);

    Q_INVOKABLE static double sphericalBearingFinal(double aLat, double aLon, double bLat, double bLon);
  };

}
#endif /* OSMINUTILS_H */
