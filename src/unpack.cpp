/*
 * Copyright (C) 2026
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

#include <cstdlib>
#include "packed.h"

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    qWarning("Usage: unpack <packed> [<dir>]\n");
    return EXIT_FAILURE;
  }

  QString packfilepath(argv[1]);
  QString dir(".");
  if (argc > 2)
    dir.assign(argv[2]);

  if (osmin::unpack(packfilepath, dir))
    return EXIT_SUCCESS;
  return EXIT_FAILURE;
}
