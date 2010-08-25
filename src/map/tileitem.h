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

// Must be at least 3
#define  CELL_BITS   6
#define  CELL_MASK   ((1<<CELL_BITS)-1)
static const Point2i CELL_SIZE(1<<CELL_BITS, 1<<CELL_BITS);

class TileItem
{
public:
  TileItem() {};
  virtual ~TileItem() {};

  bool IsEmpty ();
  virtual bool IsTotallyEmpty() const = 0;
  virtual bool IsEmpty(const Point2i &pos) const = 0;
  virtual void ScalePreview(uint8_t* /*odata*/, int /*x*/,
                            uint /*opitch*/, uint /*shift*/) { };
  virtual void Draw(const Point2i &pos) = 0;
};

class TileItem_Empty : public TileItem
{
public:
  TileItem_Empty() { };
  ~TileItem_Empty() { };

  bool IsEmpty(const Point2i &/*pos*/) const { return true; };
  void Dig(const Point2i &/*position*/, const Surface& /*dig*/){};
  void Dig(const Point2i &/*center*/, const uint /*radius*/) {};
  void Draw(const Point2i&) { };
  bool IsTotallyEmpty() const { return true; };
};

class TileItem_NonEmpty : public TileItem
{
protected:
  friend class TileItem_BaseColorKey;
  friend class TileItem_ColorKey16;
  friend class TileItem_ColorKey24;
  Surface        m_surface;
  uint           m_offset;
  uint8_t       *m_empty_bitfield;
  bool           m_is_empty;
  bool           m_need_check_empty;
  uint8_t        m_alpha_threshold;

  TileItem_NonEmpty(uint8_t alpha_threshold);

public:
  ~TileItem_NonEmpty() { delete[] m_empty_bitfield; }

  virtual bool CheckEmpty() = 0;
  virtual void Empty(int start_x, int end_x, uint8_t* buf) = 0;
  virtual void Darken(int start_x, int end_x, uint8_t* buf) = 0;
  virtual void Dig(const Point2i &position, const Surface& dig) = 0;

  void MergeSprite(const Point2i &position, Surface& spr)
  {
    spr.SetAlpha(0, 0);
    m_surface.Blit(spr, position);
    spr.SetAlpha(SDL_SRCALPHA, 0);
    //m_surface.MergeSurface(spr, position);
  }

  bool NeedDelete()
  {
    if (m_need_check_empty)
      return CheckEmpty();

    return m_is_empty;
  }

  bool IsEmpty(const Point2i &pos) const
  {
    ASSERT(!m_need_check_empty);
    ASSERT(pos.x>-1);
    return m_empty_bitfield[(pos.y<<(CELL_BITS-3)) + (pos.x>>3)] & (1 << (pos.x&7));
  }


  void Dig(const Point2i &center, const uint radius);
  bool IsTotallyEmpty() const { return false; };
  Surface& GetSurface() { return m_surface; };
  void Draw(const Point2i &pos);
};

class TileItem_BaseColorKey : public TileItem_NonEmpty
{
protected:
  friend class TileItem_ColorKey16;
  friend class TileItem_ColorKey24;
  Uint32  color_key;
  TileItem_BaseColorKey(uint8_t bpp, uint8_t alpha_threshold);
  TileItem_BaseColorKey(uint8_t alpha_threshold)
    : TileItem_NonEmpty(alpha_threshold) { };

public:
  static const Uint32 ColorKey = 0xFF00FF;

  static TileItem_NonEmpty* NewEmpty(uint8_t bpp, uint8_t alpha_threshold);

  void Dig(const Point2i &position, const Surface& dig);
  bool CheckEmpty();
};

class TileItem_ColorKey16: public TileItem_BaseColorKey
{
public:
  TileItem_ColorKey16(uint8_t threshold)
    : TileItem_BaseColorKey(16, threshold) { };
  TileItem_ColorKey16(void *pixels, int stride, uint8_t threshold);

  void Empty(int start_x, int end_x, uint8_t* buf);
  void Darken(int start_x, int end_x, uint8_t* buf);
  void ScalePreview(uint8_t *odata, int x, uint opitch, uint shift);
};

class TileItem_ColorKey24: public TileItem_BaseColorKey
{
public:
  TileItem_ColorKey24(uint8_t threshold)
    : TileItem_BaseColorKey(24, threshold) { };
  TileItem_ColorKey24(void *pixels, int stride, uint8_t threshold);

  void Empty(int start_x, int end_x, uint8_t* buf);
  void Darken(int start_x, int end_x, uint8_t* buf);
  void ScalePreview(uint8_t *odata, int x, uint opitch, uint shift);
};

class TileItem_AlphaSoftware : public TileItem_NonEmpty
{
  void SetDefaults(void);

public:
  TileItem_AlphaSoftware(uint8_t threshold);
  TileItem_AlphaSoftware(void *pixels, int stride, uint8_t threshold);
  // Fill as empty

  bool CheckEmpty();
  void Empty(int start_x, int end_x, uint8_t* buf);
  void Darken(int start_x, int end_x, uint8_t* buf);
  void Dig(const Point2i &position, const Surface& dig);
  void ScalePreview(uint8_t *odata, int x, uint opitch, uint shift);
};

TileItem_NonEmpty* NewEmpty(uint8_t bpp, uint8_t alpha_threshold);

#endif
