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

#define  CELL_BITS   6
#define  CELL_MASK   ((1<<CELL_BITS)-1)
static const Point2i CELL_SIZE(1<<CELL_BITS, 1<<CELL_BITS);

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
  void Draw(const Point2i&) { };
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

  void MergeSprite(const Point2i &position, Surface& spr)
  {
    spr.SetAlpha(0, 0);
    m_surface.Blit(spr, position);
    spr.SetAlpha(SDL_SRCALPHA, 0);
    //m_surface.MergeSurface(spr, position);
  }

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
    int a;
    m_surface.Lock();
    a = (m_surface.GetPixel(pos.x, pos.y) == color_key) ? 0 : 255;
    m_surface.Unlock();
    return a;
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

public:
  TileItem_AlphaSoftware();
  TileItem_AlphaSoftware(void *pixels, int stride);
  // Fill as empty
  static TileItem_NonEmpty* NewEmpty(void);

  unsigned char GetAlpha(const Point2i &pos)
  {
    int a;
    m_surface.Lock();
    a = m_surface.GetPixels()[pos.y*m_surface.GetPitch() + pos.x*4 + m_offset];
    m_surface.Unlock();
    return a;
  }

  void Empty(int start_x, int end_x, unsigned char* buf);
  void Darken(int start_x, int end_x, unsigned char* buf);
  void Dig(const Point2i &position, const Surface& dig);
  void ScalePreview(uint8_t *odata, int x, uint opitch, uint shift);
  bool NeedDelete();
};

#endif
