/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
  virtual unsigned char GetAlpha(const Point2i &pos) = 0;
  virtual void Dig(const Point2i &position, const Surface& dig) = 0;
  virtual void Dig(const Point2i &center, const uint radius) = 0;
  virtual Surface GetSurface() = 0;
  virtual void Draw(const Point2i &pos) = 0;
  virtual bool IsTotallyEmpty() const = 0;
#ifdef DBG_TILE
  virtual void FillWithRGB(Uint8 r, Uint8 g, Uint8 b) {};
#endif
};

class TileItem_Empty : public TileItem
{
public:
  TileItem_Empty () {};
  ~TileItem_Empty () {};

  unsigned char GetAlpha (const Point2i &pos){return 0;};
  void Dig(const Point2i &position, const Surface& dig){};
  Surface GetSurface(){return *new Surface();};
  void Dig(const Point2i &center, const uint radius) {};
  void Draw(const Point2i &pos);
  bool IsTotallyEmpty() const {return true;};
};

class TileItem_AlphaSoftware : public TileItem
{
  unsigned char* last_filled_pixel;

public:
  bool need_check_empty;
  bool need_delete;

TileItem_AlphaSoftware(const Point2i &size);
  ~TileItem_AlphaSoftware();

  unsigned char GetAlpha(const Point2i &pos);
  void Dig(const Point2i &position, const Surface& dig);
  void Dig(const Point2i &center, const uint radius);
  void Draw(const Point2i &pos);


  bool NeedDelete() const {return need_delete; };
  void CheckEmpty();
  void ResetEmptyCheck();

  bool IsTotallyEmpty() const {return false;};

private:
  TileItem_AlphaSoftware(const TileItem_AlphaSoftware &copy);
  unsigned char (TileItem_AlphaSoftware::*_GetAlpha)(const Point2i &pos);
  unsigned char GetAlpha_Index0(const Point2i &pos);
  inline unsigned char GetAlpha_Index3(const Point2i &pos);
  inline unsigned char GetAlpha_Generic(const Point2i &pos);
  Surface GetSurface();

  void Empty(const int start_x, const int end_x, unsigned char* buf, const int bpp);
  void Darken(const int start_x, const int end_x, unsigned char* buf, const int bpp);

  Point2i m_size;
  Surface m_surface;

#ifdef DBG_TILE
  void FillWithRGB(Uint8 r, Uint8 g, Uint8 b);
#endif
};

#endif
