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

#ifndef PACKED_H
#define PACKED_H

#include <QString>

namespace osmin
{

/**
 * @brief Append new file stream to a PACK archive
 * @param pack The path to PACK archive
 * @param filepath The path of the file to append
 * @return true on success elese false
 */
bool packFile(const QString& pack, const QString& filepath);

/**
 * @brief Extract a PACK archive into the given dir
 * @param pack The path to PACK archive
 * @param dirpath The path of the destination directory
 * @return true on success elese false
 */
bool unpack(const QString& pack, const QString& dirpath);

}
#endif // PACKED_H
