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
  if (argc < 3)
  {
    qWarning("Usage: pack <packed> [<file1> <file2> ...]\n");
    return EXIT_FAILURE;
  }

  QString packfilepath(argv[1]);

  for (int i = 2; i < argc; ++i)
  {
    if (osmin::packFile(packfilepath, QString(argv[i])))
      continue;
    qWarning("Failed\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
