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
 * Sprite:     Simple sprite management
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de)
 *             Initial version
 *****************************************************************************/

#include "graphic/sprite.h"
#include <SDL.h>
#include <iostream>
#include "graphic/surface.h"
#include "graphic/video.h"
#include "include/app.h"
#include "map/camera.h"
#include "map/map.h"
#include <WORMUX_rectangle.h>
#include "graphic/spriteframe.h"
#include "tool/string_tools.h"

Sprite::Sprite(bool _smooth) :
  smooth(_smooth),
  cache(*this),
  animation(*this)
{
  Constructor();
}

Sprite::Sprite(const Surface& surface, bool _smooth) :
  smooth(_smooth),
  cache(*this),
  animation(*this)
{
  Constructor();
  frame_width_pix = surface.GetWidth();
  frame_height_pix = surface.GetHeight();
  frames.push_back(SpriteFrame(surface));
}

Sprite::Sprite(const Sprite &other) :
  smooth(other.smooth),
  cache(*this),
  animation(other.animation, *this),
  frames()
{
  Constructor();
  current_surface = other.current_surface;
  show = other.show;
  current_frame = other.current_frame;
  frame_width_pix = other.frame_width_pix;
  frame_height_pix = other.frame_height_pix;
  alpha = other.alpha;
  scale_x = other.scale_x;
  scale_y = other.scale_y;
  rotation_rad = other.rotation_rad;
  rhs_pos = other.rhs_pos;
  rot_hotspot = other.rot_hotspot;
  rotation_point = other.rotation_point;

  for (uint f=0; f<other.frames.size(); f++)
    AddFrame(other.frames[f].surface,other.frames[f].delay);

  if (other.cache.have_lastframe_cache)
    EnableLastFrameCache();
  if (other.cache.have_rotation_cache)
    EnableRotationCache(other.cache.rotation_cache_size);
  if (other.cache.have_flipping_cache)
    EnableFlippingCache();
}

void Sprite::Constructor()
{
  show = true;
  current_frame = 0;
  frame_width_pix = frame_height_pix = 0;
  alpha = 1.0f;
  scale_x = scale_y = 1.0f;
  rotation_rad = 0.0f;
  SetRotation_HotSpot(center);
}

void Sprite::Init(Surface& surface, const Point2i &frameSize, int nb_frames_x, int nb_frames_y)
{
  Point2i f;

  this->frame_width_pix = frameSize.x;
  this->frame_height_pix = frameSize.y;

  surface.SetAlpha( 0, 0);

  for (f.y=0; f.y<nb_frames_y; f.y++)
    for (f.x=0; f.x<nb_frames_x; f.x++) {
      Surface new_surf(frameSize, SDL_SWSURFACE|SDL_SRCALPHA, true);
      Rectanglei sr(f * frameSize, frameSize);

      new_surf.Blit(surface, sr, Point2i(0, 0));
      frames.push_back(SpriteFrame(new_surf));
    }
}

void Sprite::AddFrame(const Surface &surf, uint delay){
  frames.push_back(SpriteFrame(surf, delay));
}

void Sprite::SetRotation_rad( Double angle_rad)
{
  while (angle_rad > TWO_PI)
    angle_rad -= TWO_PI;
  while (angle_rad <= -TWO_PI)
    angle_rad += TWO_PI;

  if (rotation_rad == angle_rad)
    return;

  rotation_rad = angle_rad;
  cache.InvalidLastFrame();
}

void Sprite::SetRotation_HotSpot(const Point2i& new_hotspot)
{
  rot_hotspot = user_defined;
  rhs_pos = new_hotspot;

  if (rhs_pos * 2 == GetSize())
    rot_hotspot = center; // avoid using Calculate_Rotation_Offset, thus avoiding a division by zero
}

void Sprite::Calculate_Rotation_Offset(const Surface & tmp_surface)
{
  const SpriteFrame & frame = GetCurrentFrameObject();
  const Surface & surface = frame.surface;

  // Calculate offset of the surface depending on hotspot rotation position :
  int surfaceHeight = surface.GetHeight();
  int surfaceWidth = surface.GetWidth();
  int halfHeight = surfaceHeight / 2;
  int halfWidth = surfaceWidth / 2;

  //Do as if hotspot is center of picture:
  rotation_point.x = halfWidth  /*surfaceWidth  / 2*/ - tmp_surface.GetWidth()  / 2;
  rotation_point.y = halfHeight /*surfaceHeight / 2*/ - tmp_surface.GetHeight() / 2;

  if (rot_hotspot == center) {
    return;
  }

  if (rot_hotspot != user_defined) {
    switch(rot_hotspot) {
    case top_left:      rhs_pos = Point2i( 0,              0);               break;
    case top_center:    rhs_pos = Point2i( halfWidth /*surfaceWidth/2*/, 0);               break;
    case top_right:     rhs_pos = Point2i( surfaceWidth,   0);               break;
    case left_center:   rhs_pos = Point2i( 0,              surfaceHeight/2); break;
    case center:        rhs_pos = Point2i( halfWidth /*surfaceWidth/2*/, halfHeight /*surfaceHeight/2*/); break;
    case right_center:  rhs_pos = Point2i( surfaceWidth,   halfHeight /*surfaceHeight/2*/); break;
    case bottom_left:   rhs_pos = Point2i( 0,              surfaceHeight);   break;
    case bottom_center: rhs_pos = Point2i( halfWidth /*surfaceWidth/2*/, surfaceHeight);   break;
    case bottom_right:  rhs_pos = Point2i( surfaceWidth,   surfaceHeight);   break;
    default:
      ASSERT(false);
    }
  }

  Point2i rhs_pos_tmp;
  rhs_pos_tmp.x = static_cast<int>(rhs_pos.x * scale_x);
  rhs_pos_tmp.y = static_cast<int>(rhs_pos.y * scale_y);
  surfaceWidth  = static_cast<int>(surfaceWidth  * scale_x);
  surfaceHeight = static_cast<int>(surfaceHeight * scale_y);
  //Calculate the position of the hotspot after a rotation around the center of the surface:

  Point2i center(surfaceWidth / 2, surfaceHeight / 2);
  // Don't let the compiler any choice with which types the resulting program will calculate with.
  Point2i old_hotspot_delta_i = center - rhs_pos_tmp;
  Point2d old_hotspot_delta = old_hotspot_delta_i;
  Double rhs_dst = old_hotspot_delta.Norm();
  Double rhs_angle = 0.0;
  if (rhs_dst != ZERO)
    rhs_angle = -acos(-old_hotspot_delta.x / rhs_dst);
  if (halfHeight /*surfaceHeight/2*/ - rhs_pos.y < 0)
    rhs_angle = -rhs_angle;

  rhs_angle += rotation_rad;

  Point2d new_hotspot_delta = Point2d::FromPolarCoordinates(rhs_dst, rhs_angle);
  Point2i new_hotspot_delta_i = new_hotspot_delta;
  Point2i new_hotspot = center + new_hotspot_delta_i;

  rotation_point -= new_hotspot;
  rotation_point += rhs_pos_tmp;
}

void Sprite::Start()
{
  show = true;
  animation.Start();
  cache.InvalidLastFrame();
}

void Sprite::Blit(Surface &dest, int pos_x, int pos_y, int src_x, int src_y, uint w, uint h)
{
  if (!show)
    return;

  RefreshSurface();

  Rectanglei srcRect (src_x, src_y, w, h);
  Rectanglei dstRect (pos_x + rotation_point.x, pos_y + rotation_point.y, w, h);

  if (alpha == ONE) {
    dest.Blit(current_surface, srcRect, dstRect.GetPosition());
  } else {
    Surface surf_alpha;
    surf_alpha.NewSurface(srcRect.GetSize(),SDL_SWSURFACE,false);
    surf_alpha.Blit(dest,dstRect,Point2i(0,0));
    Double max_unsigned_char = 255.0;
    surf_alpha.SetAlpha(SDL_SRCALPHA, (int)(alpha * max_unsigned_char));
    surf_alpha.Blit(current_surface,srcRect,Point2i(0,0));
    dest.Blit(surf_alpha, srcRect, dstRect.GetPosition());
  }

  // For the cache mechanism
  GetWorld().ToRedrawOnScreen(dstRect);
}

void Sprite::Finish()
{
  animation.Finish();
  switch(animation.GetShowOnFinish())
  {
  case SpriteAnimation::show_first_frame:
    current_frame = 0;
    break;
  case SpriteAnimation::show_blank:
    show = false;
    break;
  default:
  case SpriteAnimation::show_last_frame:
    current_frame = frames.size()-1;
    break;
  }
  cache.InvalidLastFrame();
}

void Sprite::Draw(const Point2i &pos)
{
  DrawXY(pos - Camera::GetInstance()->GetPosition());
}

void Sprite::DrawXY(const Point2i &pos)
{
  if (!show)
    return;

  Blit(GetMainWindow(), pos);
}

// Help tracking whether rotozooms are performed, depending on the
// caching mechanisms
//#define DEBUG_ROTOZOOM

void Sprite::RefreshSurface()
{
  current_surface.Free();

#ifdef DEBUG_ROTOZOOM
  int rotozoom = 0;
#endif

  if (!cache.have_rotation_cache && !cache.have_flipping_cache) {
    if (!cache.have_lastframe_cache) {
      current_surface = frames[current_frame].surface.RotoZoom(-rotation_rad, scale_x, scale_y, smooth);
#ifdef DEBUG_ROTOZOOM
      rotozoom = 1;
#endif
    } else if (cache.last_frame.IsNull()) {
      current_surface = frames[current_frame].surface.RotoZoomC(-rotation_rad, scale_x, scale_y, smooth);
      cache.last_frame = current_surface;
#ifdef DEBUG_ROTOZOOM
      rotozoom = 2;
#endif
    }
    else {
      current_surface = cache.last_frame;
    }
  } else if (cache.have_flipping_cache && !cache.have_rotation_cache) {
    if (rotation_rad != ZERO || scale_y != ONE || (scale_x != ZERO && scale_x != -ONE)) {
      current_surface = frames[current_frame].surface.RotoZoom(rotation_rad, scale_x, scale_y, smooth);
#ifdef DEBUG_ROTOZOOM
      rotozoom = 3;
#endif
    } else if (scale_x == ONE)
      current_surface = frames[current_frame].surface;
    else
      current_surface = cache.frames[current_frame].flipped_surface;
  } else if (!cache.have_flipping_cache && cache.have_rotation_cache) {
    if (scale_x != ONE || scale_y != ONE) {
      current_surface = frames[current_frame].surface.RotoZoom(rotation_rad, scale_x, scale_y, smooth);
#ifdef DEBUG_ROTOZOOM
      rotozoom = 4;
#endif
    } else
      current_surface = cache.frames[current_frame].GetSurfaceForAngle(rotation_rad);
  }
  //cache.have_flipping_cache==true && cache.have_rotation_cache==true
  else if ((scale_x != ONE && scale_x != -ONE)  || scale_y != ONE) {
    current_surface = frames[current_frame].surface.RotoZoom(rotation_rad, scale_x, scale_y, smooth);
#ifdef DEBUG_ROTOZOOM
    rotozoom = 5;
#endif
  } else if (scale_x == ONE) //Scale_y == 1.0
    current_surface = cache.frames[current_frame].GetSurfaceForAngle(rotation_rad);
  else
    current_surface = cache.frames[current_frame].GetFlippedSurfaceForAngle(rotation_rad);

  ASSERT(!current_surface.IsNull());
#ifdef DEBUG_ROTOZOOM
   if (rotozoom && scale_x!=ONE && scale_y!=ONE && scale_x==scale_y)
     MSG_DEBUG("sprite", "rotation: %.3f, scale_x: %.3f, scale_y: %.3f",
               rotation_rad.tofloat(), scale_x.tofloat(), scale_y.tofloat());
#endif

  // Calculate offset of the sprite depending on hotspot rotation position :
  rotation_point.x=0;
  rotation_point.y=0;
  if (rot_hotspot != center || rotation_rad!=ZERO)
    Calculate_Rotation_Offset(current_surface);
}

Surface Sprite::GetSurface() const
{
  ASSERT(!current_surface.IsNull());
  return current_surface;
}
