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
#include "graphic/surface.h"

class Sprite;

#if 0 //def ANDROID
#  define RotoZoomC(a, x, y, s) RotoZoom(a, x, y, s).DisplayFormatColorKey(128)
#else
#  define RotoZoomC(a, x, y, s) RotoZoom(a, x, y, s)
#endif

class SpriteFrameCache
{
  uint delay;
  Double min, max;

  static Double RestrictAngle(Double angle, Double mini, Double maxi)
  {
    while (angle < mini)
      angle += TWO_PI;
    while (angle >= maxi)
    angle -= TWO_PI;
    ASSERT(angle>=mini && angle<maxi);
    return angle;
  }

  Surface normal_surface;
  Surface flipped_surface;
  std::vector<Surface> rotated_surface;
  std::vector<Surface> rotated_flipped_surface;

public:
  Surface GetFlippedSurfaceForAngle(Double angle);
  Surface GetSurfaceForAngle(Double angle);

  SpriteFrameCache(uint d = 100) { delay = d; }
  SpriteFrameCache(const Surface& surf, uint d = 100)
    : normal_surface(surf)
    , delay(d)
  { }
  SpriteFrameCache(const SpriteFrameCache& other)
    : delay(other.delay)
    , min(other.min), max(other.max)
    , normal_surface(other.normal_surface)
    , flipped_surface(other.flipped_surface)
    , rotated_surface(other.rotated_surface)
    , rotated_flipped_surface(other.rotated_flipped_surface)
  { }

  ~SpriteFrameCache()
  {
    rotated_surface.clear();
    rotated_flipped_surface.clear();
    normal_surface.Free();
    flipped_surface.Free();
  }

  void SetCaches(bool flipped, uint rotation_num, Double min, Double max);
  uint GetDelay() const { return delay; }
  void SetDelay(uint d) { delay = d; }
};

class SpriteCache : public std::vector<SpriteFrameCache>
{
  Sprite &sprite;

  uint rotation_cache_size;
  bool have_flipping_cache;
  Surface last_frame;

public:
  explicit SpriteCache(Sprite &spr)
    : sprite(spr)
    , rotation_cache_size(0)
    , have_flipping_cache(false)
  { }

  void SetFrames(SpriteCache &other)
  {
    clear();
    rotation_cache_size = other.rotation_cache_size;
    have_flipping_cache = other.have_flipping_cache;
    for (uint i=0; i<other.size(); i++)
      push_back(SpriteFrameCache(other[i]));
  }
  void AddFrame(const Surface& surf, uint delay) { push_back(SpriteFrameCache(surf, delay)); }
  void EnableCaches(bool flipped, uint rotation_num, Double min, Double max);
  void InvalidLastFrame() { last_frame.Free(); }

  //operator SpriteFrameCache& [](uint index) { return frames.at(index); }
  void SetDelay(uint delay)
  {
    for (uint i=0; i<size(); i++)
      at(i).SetDelay(delay);
  }
};

#endif /* _SPRITE_CACHE_H */
