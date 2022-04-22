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

#include "csvparser.h"
#include <QList>
#include <QByteArray>

using namespace osmin;

CSVParser::CSVParser(const char separator, const char encapsulator)
: m_separator(separator)
, m_encapsulator(encapsulator)
{ }

QList<QByteArray*> CSVParser::deserialize(const QByteArray& line)
{
  QList<QByteArray*> data;
  QByteArray::const_iterator pos = line.begin();
  if (pos == line.end())
    return data; // no data
  QByteArray* value = new QByteArray();
  bool first =  true;
  bool encap = false;
  bool error = false;
  while (pos != line.end())
  {
    if (*pos == m_encapsulator)
    {
      ++pos;
      if (encap)
      {
        if (pos == line.end() || *pos != m_encapsulator)
        {
          encap = false;
          while (pos != line.end() && *pos != m_separator) ++pos;
        }
        else
        {
          value->append(*pos);
          ++pos;
        }
      }
      else if (!first)
      {
        // error: Invalid character in stream
        error = true;
        break;
      }
      else
      {
        encap = true;
      }
    }
    else if (*pos == m_separator && !encap)
    {
      data.push_back(value);
      value = new QByteArray();
      first = true;
      ++pos;
    }
    else if (*pos == '\r' || *pos == '\n')
    {
      ++pos;
    }
    else
    {
      first = false;
      value->append(*pos);
      ++pos;
    }
  }
  // encap: Unexpected end of stream
  if (error || encap)
  {
    delete value;
    qDeleteAll(data);
    data.clear();
  }
  else
    data.push_back(value);
  return data;
}

QByteArray CSVParser::serialize(const QList<QByteArray*> row)
{
  QByteArray line;
  for (const QByteArray* data : row)
  {
    bool encap = false;
    QByteArray field;
    for (QByteArray::const_iterator it = data->begin(); it != data->end(); ++it)
    {
      if (*it == m_encapsulator)
      {
        encap = true;
        field.append(m_encapsulator).append(m_encapsulator);
      }
      else if (*it == m_separator)
      {
        encap = true;
        field.append(*it);
      }
      else if (*it == '\r')
        field.append("");
      else if (*it == '\n')
        field.append(" ");
      else if (*it == '\t')
        field.append(" ");
      else
        field.append(*it);
    }
    if (line.end() != line.begin())
      line.append(m_separator);
    if (encap)
      line.append(m_encapsulator).append(field).append(m_encapsulator);
    else
      line.append(field);
  }
  return line;
}
