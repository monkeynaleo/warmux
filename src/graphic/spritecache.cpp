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

#include "graphic/spritecache.h"

void SpriteFrameCache::SetCaches(bool flipped, uint rotation_num, Double mini, Double maxi)
{
  min = mini;
  max = maxi;

  ASSERT(!normal_surface.IsNull());
  rotated_surface.clear();
  rotated_surface.resize(rotation_num);
  if (flipped) {
    flipped_surface = normal_surface.Mirror();
    rotated_flipped_surface.clear();
    rotated_flipped_surface.resize(rotation_num);
  }
}

Surface SpriteFrameCache::GetFlippedSurfaceForAngle(Double angle)
{
  ASSERT(max - min > ZERO);
  //Double fmin = PI-max;
  angle = RestrictAngle(angle, min, max);
  uint index = ((uint)rotated_flipped_surface.size()*angle - min)
             / Double(max-min);
  ASSERT(rotated_flipped_surface.size()>index);

  // On demand-cache
  if (rotated_flipped_surface[index].IsNull()) {
    angle = min + (max-min)*(1-index/(Double)rotated_flipped_surface.size());
    rotated_flipped_surface[index] = flipped_surface.RotoZoomC(angle, ONE, ONE, true);
  }
  return rotated_flipped_surface[index];
}

Surface SpriteFrameCache::GetSurfaceForAngle(Double angle)
{
  ASSERT(max - min > ZERO);
  angle = RestrictAngle(angle, min, max);
  uint index = ((uint)rotated_flipped_surface.size()*angle -min)
             / Double(max-min);
  ASSERT(rotated_surface.size()>index);

  // On demand-cache
  if (rotated_surface[index].IsNull()) {
    angle = min + (max-min)*(1-index/(Double)rotated_flipped_surface.size());
    rotated_surface[index] = normal_surface.RotoZoomC(angle, ONE, ONE, true);
  }
  return rotated_surface[index];
}

void SpriteCache::EnableCaches(bool flipped, uint rotation_num, const Double& min, const Double& max)
{
  //For each frame, we pre-render 'rotation_num' rotated surface
  //At runtime the prerender Surface with the nearest angle to what is asked is displayed
  have_flipping_cache = flipped;
  rotation_cache_size = rotation_num;

  assert(!empty());

  for (uint f=0; f<size(); f++) {
    operator[](f).SetCaches(flipped, rotation_num, min, max);
  }
}

void SpriteCache::FixParameters(const Double& rotation_rad, const Double& scale_x, const Double& scale_y)
{
  for (uint i=0; i<size(); i++) {
    SpriteFrameCache& frame = operator[](i);
#ifdef HAVE_HANDHELD
    frame.normal_surface = frame.normal_surface.RotoZoom(rotation_rad, scale_x, scale_y, true).DisplayFormatColorKey(128);
#else
    frame.normal_surface = frame.normal_surface.RotoZoom(rotation_rad, scale_x, scale_y, true);
#endif
  }
}
