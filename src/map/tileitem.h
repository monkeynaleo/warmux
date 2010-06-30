/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *****************************************************************************/

#ifndef TILEITEM_H
#define TILEITEM_H

#include <WORMUX_point.h>
#include "graphic/surface.h"

const Point2i CELL_SIZE(64, 64);

#ifdef DEBUG
//#define DBG_TILE
#endif

class TileItem
{
public:
  TileItem () {};
  virtual ~TileItem () {};

  bool IsEmpty ();
  virtual bool IsTotallyEmpty() const = 0;
  virtual unsigned char GetAlpha(const Point2i &pos) = 0;
  virtual void ScalePreview(uint8_t* /*odata*/, int /*x*/,
                            uint /*opitch*/, uint /*shift*/) { };
  virtual void Draw(const Point2i &pos) = 0;
  //~ virtual void Dig(const Point2i &position, const Surface& dig) = 0;
  //~ virtual void Dig(const Point2i &center, const uint radius) = 0;
  //~ virtual void MergeSprite(const Point2i &/*position*/, Surface& /*spr*/) {};
#ifdef DBG_TILE
  virtual void FillWithRGB(Uint8 /*r*/, Uint8 /*g*/, Uint8 /*b*/) {};
#endif
};

class TileItem_Empty : public TileItem
{
public:
  TileItem_Empty () { empty = NULL; };
  ~TileItem_Empty () { if (empty) delete empty; };

  Surface *empty;
  unsigned char GetAlpha (const Point2i &/*pos*/){return 0;};
  void Dig(const Point2i &/*position*/, const Surface& /*dig*/){};
  void Dig(const Point2i &/*center*/, const uint /*radius*/) {};
  void Draw(const Point2i &pos);
  bool IsTotallyEmpty() const {return true;};
};

class TileItem_NonEmpty : public TileItem
{
protected:
  friend class TileItem_BaseColorKey;
  friend class TileItem_ColorKey16;
  friend class TileItem_ColorKey24;
  Surface        m_surface;
  uint           m_offset;

  TileItem_NonEmpty() { };

public:
  bool           need_check_empty;

  virtual bool NeedDelete() = 0;
  virtual void Empty(int start_x, int end_x, unsigned char* buf) = 0;
  virtual void Darken(int start_x, int end_x, unsigned char* buf) = 0;
  virtual void Dig(const Point2i &position, const Surface& dig) = 0;

  void MergeSprite(const Point2i &position, Surface& spr);
  void Dig(const Point2i &center, const uint radius);
  bool IsTotallyEmpty() const { return false; };
  Surface& GetSurface() { return m_surface; };
  void ResetEmptyCheck() { need_check_empty = false; }
  bool NeedCheckEmpty() const { return need_check_empty; }
  void Draw(const Point2i &pos);
};

class TileItem_BaseColorKey : public TileItem_NonEmpty
{
protected:
  friend class TileItem_ColorKey16;
  friend class TileItem_ColorKey24;
  Uint32  color_key;
  TileItem_BaseColorKey(uint bpp);
  TileItem_BaseColorKey() { };

public:
  static const Uint32 ColorKey = 0xFF00FF;

  unsigned char GetAlpha(const Point2i &pos)
  {
    return (m_surface.GetPixel(pos.x, pos.y) == color_key) ? 0 : 255;
  }

  void Dig(const Point2i &position, const Surface& dig);
};

class TileItem_ColorKey16: public TileItem_BaseColorKey
{
public:
  TileItem_ColorKey16() : TileItem_BaseColorKey(16) { };
  TileItem_ColorKey16(void *pixels, int stride, uint threshold);

  // Fill as empty
  static TileItem_NonEmpty* NewEmpty();
  bool NeedDelete();
  void Empty(int start_x, int end_x, unsigned char* buf);
  void Darken(int start_x, int end_x, unsigned char* buf);
  void ScalePreview(uint8_t *odata, int x, uint opitch, uint shift);
};

class TileItem_ColorKey24: public TileItem_BaseColorKey
{
public:
  TileItem_ColorKey24() : TileItem_BaseColorKey(24) { };
  TileItem_ColorKey24(void *pixels, int stride, uint threshold);

  // Fill as empty
  static TileItem_NonEmpty* NewEmpty();
  bool NeedDelete();
  void Empty(int start_x, int end_x, unsigned char* buf);
  void Darken(int start_x, int end_x, unsigned char* buf);
  void ScalePreview(uint8_t *odata, int x, uint opitch, uint shift);
};

class TileItem_AlphaSoftware : public TileItem_NonEmpty
{
  void SetDefaults(void);

#ifdef DBG_TILE
  void FillWithRGB(Uint8 r, Uint8 g, Uint8 b);
#endif

public:
  TileItem_AlphaSoftware();
  TileItem_AlphaSoftware(void *pixels, int stride);
  // Fill as empty
  static TileItem_NonEmpty* NewEmpty(void);

  unsigned char GetAlpha(const Point2i &pos)
  {
    return m_surface.GetPixels()[pos.y*m_surface.GetPitch() + pos.x*4 + m_offset];
  }

  void Empty(int start_x, int end_x, unsigned char* buf);
  void Darken(int start_x, int end_x, unsigned char* buf);
  void Dig(const Point2i &position, const Surface& dig);
  void ScalePreview(uint8_t *odata, int x, uint opitch, uint shift);
  bool NeedDelete();
};

#endif
