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

#include "packed.h"
#include "compressor.h"

#include <QFile>
#include <QDir>
#include <QDebug>

/*
 * The PACK archive structure:
 *
 * The PACK archive contains a sequence of stream (chunk), with a
 * name and a data section. Each stream should start with a magic
 * header, followed by the chunk header and data. The data section
 * is encoded with the GZIP compression algorythm (ZLIB).
 *
 * [PACK] + name_size[1] + name[] + 0x0 + data_size[4] + data[]
 *
 * PACK         : The magic chunk header is 4 bytes: 50 41 43 4B
 * name_size    : The size of the chunk name is 1 bytes [0..FF]
 * name         : The chunk name [name_size]
 * separator    : The byte 0x0
 * data_size    : The size of the data is 4 bytes encoded BE
 * data         : The data stream (GZIP) [data_size]
 *
 * The name section is limited to 255 bytes
 * The data section is limited to 4GB
 * The size section is stored with network byte order (BE)
 */

namespace
{

int fileStreamReader(void *handle, void *buf, int sz)
{
  QFile * file = reinterpret_cast<QFile*>(handle);
  return file->read((char*)buf, sz);
}

struct Packed
{
  QFile& stream;
  unsigned bytes;
  Packed(QFile& _stream, unsigned _bytes) : stream(_stream), bytes(_bytes) { }
};

int packedStreamReader(void *handle, void *buf, int sz)
{
  Packed * packed = reinterpret_cast<Packed*>(handle);
  int r = packed->stream.read((char*)buf, ((unsigned)sz < packed->bytes ? sz : packed->bytes));
  if (r <= 0)
    return 0;
  packed->bytes -= r;
  return r;
}

}

bool osmin::packFile(const QString& pack, const QString& filepath)
{
  QFile filein(filepath);
  QFile fileout(pack);
  if (!filein.open(QIODevice::ReadOnly))
  {
    qWarning("%s: open file failed (%s)", __FUNCTION__, filepath.toUtf8().constData());
    return false;
  }
  if (!fileout.open(QIODevice::ReadWrite))
  {
    qWarning("%s: open file failed (%s)", __FUNCTION__, pack.toUtf8().constData());
    return false;
  }

  fileout.seek(fileout.size());
  if (!fileout.atEnd())
    return false;

  // write the magic header
  fileout.write("PACK", 4);

  char tmp[4];
  unsigned sz;
  QString chunk_name;

  int p = filepath.lastIndexOf('/');
  if (p >= 0)
    chunk_name = filepath.mid(p + 1);
  else
    chunk_name = filepath;

  // write name section
  tmp[0] = chunk_name.size() & 0xff;
  tmp[1] = 0;
  fileout.write(tmp, 1);
  fileout.write(chunk_name.toUtf8().constData());
  fileout.write(tmp + 1, 1);

  // write size section
  sz = 0;
  tmp[0] = 0;
  tmp[1] = 0;
  tmp[2] = 0;
  tmp[3] = 0;
  fileout.write(tmp, 4);

  // write data section
  Compressor encoder(fileStreamReader, &filein, true);
  for (;;)
  {
    const char * data;
    if (encoder.IsCompleted())
      break;
    else if (encoder.HasOutputData())
    {
      int r = (int)encoder.FetchOutput(&data);
      if (r > 0)
      {
        if (fileout.write(data, r) != r)
        {
          qWarning("%s: write file failed (%s)", __FUNCTION__, pack.toUtf8().constData());
          fileout.remove();
          return false;
        }
        sz += r;
      }
      else if (!encoder.IsCompleted())
      {
        if (encoder.HasStreamError())
          qWarning("%s: encoding failed: stream error", __FUNCTION__);
        else if (encoder.HasBufferError())
          qWarning("%s: encoding failed: buffer error", __FUNCTION__);
        else
          qWarning("%s: encoding failed", __FUNCTION__);
        fileout.remove();
        return false;
      }
    }
    else
      break;
  }
  fileout.flush();

  // update size section
  if (!fileout.seek(fileout.pos() - sz - 4))
    return false;
  tmp[0] = (sz >> 24) & 0xff;
  tmp[1] = (sz >> 16) & 0xff;
  tmp[2] = (sz >> 8) & 0xff;
  tmp[3] = sz & 0xff;
  if (!fileout.write(tmp, 4))
    return false;
  fileout.flush();
  return true;
}

bool osmin::unpack(const QString& pack, const QString& dirpath)
{
  QDir root(dirpath);
  if (!root.exists())
  {
    qWarning("%s: destination dir doesn't exist (%s)", __FUNCTION__, dirpath.toUtf8().constData());
    return false;
  }
  QFile filein(pack);
  if (!filein.open(QIODevice::ReadOnly))
  {
    qWarning("%s: open file failed (%s)", __FUNCTION__, pack.toUtf8().constData());
    return false;
  }

  for (;;)
  {
    char tmp[4];
    unsigned sz;
    // read the magic header
    if (filein.read(tmp, 4) != 4 || ::memcmp(tmp, "PACK", 4) != 0)
      break;

    // read name section
    if (filein.read(tmp, 1) != 1)
      return false;
    sz = (unsigned)tmp[0];
    char chunk_name[256];
    if (filein.read(chunk_name, sz) != sz || filein.read(&chunk_name[sz], 1) != 1 ||
        chunk_name[sz] != 0)
      return false;

    // read size section
    if (filein.read(tmp, 4) != 4)
      return false;
    sz = ((tmp[0] << 24) & 0xff000000) |
         ((tmp[1] << 16) & 0xff0000) |
         ((tmp[2] << 8) & 0xff00) |
         (tmp[3] & 0xff);

    qInfo("%s: extract stream %s (%u)", __FUNCTION__, chunk_name, sz);

    QFile fileout(root.absoluteFilePath(QString(chunk_name)));

    // FIXME: WriteOnly will fail on Android 35 when permissions are not
    // set correctly, so workaround by removing the old file
    if (fileout.exists() && !fileout.remove())
    {
      qWarning("%s: file overwrite failed (%s)", __FUNCTION__, fileout.fileName().toUtf8().constData());
      return false;
    }
    if (!fileout.open(QIODevice::WriteOnly))
    {
      qWarning("%s: create file failed (%s)", __FUNCTION__, fileout.fileName().toUtf8().constData());
      return false;
    }

    Packed packed(filein, sz);
    Decompressor decoder(packedStreamReader, &packed, true);
    for (;;)
    {
      const char * data;
      if (decoder.IsCompleted())
        break;
      else if (decoder.HasOutputData())
      {
        int sz = (int)decoder.FetchOutput(&data);
        if (sz > 0)
        {
          if (fileout.write(data, sz) != sz)
          {
            qWarning("%s: write file failed (%s)", __FUNCTION__, fileout.fileName().toUtf8().constData());
            fileout.remove();
            return false;
          }
        }
        else if (!decoder.IsCompleted())
        {
          if (decoder.HasStreamError())
            qWarning("%s: decoding failed: stream error", __FUNCTION__);
          else if (decoder.HasBufferError())
            qWarning("%s: decoding failed: buffer error", __FUNCTION__);
          else
            qWarning("%s: decoding failed", __FUNCTION__);
          fileout.remove();
          return false;
        }
      }
      else
        break;
    }
    fileout.flush();
  }

  return true;
}
