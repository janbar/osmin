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

#include "csvparser.h"

using namespace osmin;

CSVParser::CSVParser(const char separator, const char encapsulator)
: m_separator(separator)
, m_encapsulator(encapsulator)
, m_error(false)
, m_error_pos(0)
{ }

bool CSVParser::deserialize_chunk(bool next, container& out, const std::string& line)
{
  std::string::const_iterator pos = line.begin();
  if (pos == line.end())
  {
    // push blank value and avoid fault
    if (next && out.empty())
      out.push_back("");
    return next; // end of stream
  }
  field value;
  bool first =  true;
  bool error = false;
  bool encap = next;
  if (encap)
  {
    value.assign(out.back());
    out.pop_back();
  }
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
          value.push_back(*pos);
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
      out.push_back(std::move(value));
      value.clear();
      first = true;
      ++pos;
    }
    else
    {
      first = false;
      if (encap || (*pos != '\n' && *pos != '\r'))
        value.push_back(*pos);
      ++pos;
    }
  }
  // encap: Unexpected end of stream
  if (error)
  {
    out.clear();
    m_error = true;
    m_error_pos = (unsigned) std::distance(line.begin(), pos);
    return false;
  }
  out.push_back(std::move(value));
  return encap;
}

bool CSVParser::deserialize(container& out, const std::string& line)
{
  m_error = false;
  out.clear();
  return deserialize_chunk(false, out, line);
}

bool CSVParser::deserialize_next(container& out, const std::string& line)
{
  return deserialize_chunk(true, out, line);
}

void CSVParser::serialize(std::string& out, const container& row)
{
  bool first = true;
  out.clear();
  for (const field& data : row)
  {
    bool encap = false;
    field tmp;
    for (field::const_iterator it = data.begin(); it != data.end(); ++it)
    {
      if (*it == m_encapsulator)
      {
        encap = true;
        tmp.push_back(m_encapsulator);
        tmp.push_back(m_encapsulator);
      }
      else if (*it == m_separator || *it == '\r' || *it == '\n')
      {
        encap = true;
        tmp.push_back(*it);
      }
      else
        tmp.push_back(*it);
    }
    if (!first)
      out.push_back(m_separator);
    if (encap)
    {
      out.push_back(m_encapsulator);
      out.append(tmp);
      out.push_back(m_encapsulator);
    }
    else
      out.append(tmp);
    first = false;
  }
}
