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

#include "graphic/sprite.h"
#include "tool/math_tools.h"

SpriteFrameCache::SpriteFrameCache()
{
  use_rotation = false;
}

void SpriteFrameCache::CreateRotationCache(Surface &surface, uint cache_size, bool smooth)
{
  ASSERT(use_rotation == false);
  use_rotation = true;

  rotated_surface.push_back(surface);
  for (uint i=1; i<cache_size; i++){
    Double angle = TWO_PI * (1 /* to inverte rotation angle */ - i / Double(cache_size));
    rotated_surface.push_back(surface.RotoZoomC(angle, 1.0, 1.0, smooth));
  }
}

static const Double ANGLE_DIV = 1 / TWO_PI;

Surface SpriteFrameCache::GetFlippedSurfaceForAngle(Double angle) const
{
  uint index = (uint)rotated_flipped_surface.size() * RestrictAngle(angle) * ANGLE_DIV;
  ASSERT(rotated_flipped_surface.size()>index);
  return rotated_flipped_surface[index];
}

Surface SpriteFrameCache::GetSurfaceForAngle(Double angle) const
{
  uint index = (uint)rotated_surface.size() * RestrictAngle(angle) * ANGLE_DIV;
  ASSERT(rotated_surface.size()>index);
  return rotated_surface[index];
}

void SpriteFrameCache::CreateFlippingCache(Surface &surface, bool smooth)
{
  ASSERT(flipped_surface.IsNull());
  flipped_surface = surface.RotoZoomC(0.0, -1.0, 1.0, smooth);
  if (use_rotation) {
    ASSERT(rotated_surface.size() != 0);
    ASSERT(rotated_flipped_surface.size() == 0);
    rotated_flipped_surface.push_back(flipped_surface);
    const uint n = rotated_surface.size();
    for (uint i=1 ; i<n; i++) {
      Double angle = TWO_PI * (1 - Double(i) / n);
      rotated_flipped_surface.push_back(surface.RotoZoomC(angle, -1.0, 1.0, smooth));
    }
  }
}




SpriteCache::SpriteCache(Sprite &p_sprite)
  : sprite(p_sprite)
{
  have_rotation_cache = false;
  have_flipping_cache = false;
  have_lastframe_cache = false;
  rotation_cache_size = 0;
}

void SpriteCache::EnableRotationCache(std::vector<SpriteFrame> &sprite_frames, uint cache_size)
{
  //For each frame, we pre-render 'cache_size' rotated surface
  //At runtime the prerender Surface with the nearest angle to what is asked is displayed
  ASSERT(1 < cache_size && cache_size <= 360);
  ASSERT(!have_lastframe_cache);
  ASSERT(!have_flipping_cache); //Always compute rotation cache before flipping cache!
  ASSERT(!have_rotation_cache);
  have_rotation_cache = true;

  if (frames.empty())
    frames.resize(sprite_frames.size());
  ASSERT(frames.size() == sprite_frames.size());
  rotation_cache_size = cache_size;

  for (uint f=0; f<frames.size(); f++) {
    frames[f].CreateRotationCache(sprite_frames[f].surface, cache_size, sprite.IsAntialiased());
  }
}

void SpriteCache::EnableFlippingCache(std::vector<SpriteFrame> &sprite_frames)
{
  //For each frame, we pre-render the flipped frame
  ASSERT(!have_flipping_cache);
  ASSERT(!have_lastframe_cache);

  if (frames.empty())
    frames.resize(sprite_frames.size());
  ASSERT(frames.size() == sprite_frames.size());

  have_flipping_cache = true;

  for (uint f=0; f<frames.size(); f++)
    frames[f].CreateFlippingCache(sprite_frames[f].surface, sprite.IsAntialiased());
}
