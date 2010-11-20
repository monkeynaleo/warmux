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
 ******************************************************************************
 * Sprite cache.
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de)
 *             Initial version
 *****************************************************************************/

#ifndef _SPRITE_CACHE_H
#define _SPRITE_CACHE_H

#include <vector>
#include <assert.h>
#include <WORMUX_base.h>
#include "graphic/spriteframe.h"
#include "graphic/surface.h"

class Sprite;

#if 0 //def ANDROID
#  define RotoZoomC(a, x, y, s) RotoZoom(a, x, y, s).DisplayFormatColorKey(128)
#else
#  define RotoZoomC(a, x, y, s) RotoZoom(a, x, y, s)
#endif

class SpriteFrameCache
{
  bool use_rotation;
  std::vector<Surface> rotated_surface;
  std::vector<Surface> rotated_flipped_surface;

public:
  Surface flipped_surface;
  Surface GetFlippedSurfaceForAngle(Double angle) const;
  Surface GetSurfaceForAngle(Double angle) const;

  SpriteFrameCache();
  void CreateRotationCache(Surface &surface, uint cache_size, bool smooth);
  void CreateFlippingCache(Surface &surface, bool smooth);
};

class SpriteCache
{
  Sprite &sprite;

public:
  bool have_rotation_cache;
  uint rotation_cache_size;
  bool have_flipping_cache;
  bool have_lastframe_cache;
  Surface last_frame;
  std::vector<SpriteFrameCache> frames;

public:
  explicit SpriteCache(Sprite &sprite);

  void EnableRotationCache(std::vector<SpriteFrame> &frames, uint cache_size);
  void EnableFlippingCache(std::vector<SpriteFrame> &frames);
  void EnableLastFrameCache()
  {
    //The result of the last call to SDLgfx is kept in memory
    //to display it again if rotation / scale / alpha didn't changed
    assert(!have_rotation_cache);
    assert(!have_flipping_cache);
    have_lastframe_cache = true;
  }
  void DisableLastFrameCache()
  {
    //The result of the last call to SDLgfx is kept in memory
    //to display it again if rotation / scale / alpha didn't changed
    assert(!have_rotation_cache);
    assert(!have_flipping_cache);
    have_lastframe_cache = false;
  }
  void InvalidLastFrame() { if (have_lastframe_cache) last_frame.Free(); }
};

#endif /* _SPRITE_CACHE_H */
