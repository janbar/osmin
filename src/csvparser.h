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

#include <vector>
#include <string>

namespace osmin
{

class CSVParser
{
public:
  typedef std::string field;
  typedef std::vector<field> container;

  CSVParser(const char separator, const char encapsulator);
  virtual ~CSVParser() { }

  bool deserialize(container& out, const std::string& line);
  bool deserialize_next(container& out, const std::string& line);
  bool in_error() const { return m_error; }
  unsigned error_position() const { return m_error_pos; }

  void serialize(std::string& out, const container& row);

private:
  char m_separator;
  char m_encapsulator;
  bool m_error;
  unsigned m_error_pos;

  bool deserialize_chunk(bool next, container& out, const std::string& line);
};

}

#endif /* CSVPARSER_H */
