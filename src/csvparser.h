/*
 *      Copyright (C) 2020 Jean-Luc Barriere
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
#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QList>
#include <QByteArray>

namespace osmin
{
  class CSVParser
  {
  public:
    CSVParser(const char separator, const char encapsulator);
    virtual ~CSVParser() { }

    QList<QByteArray*> deserialize(const QByteArray& line);
    QByteArray serialize(const QList<QByteArray*>& row);

  private:
    char m_separator;
    char m_encapsulator;
  };
}

#endif /* CSVPARSER_H */
