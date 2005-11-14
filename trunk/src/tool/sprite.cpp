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
 ******************************************************************************
 * Sprite:     Simple sprite management
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de) 
 *             Initial version
 * TODO:       Scale,Rotation...
 *****************************************************************************/

#include "sprite.h"
#include "../map/camera.h"
#include "../include/app.h"
#include <SDL.h>
#include <SDL_rotozoom.h>
#include <iostream>
#include "../game/time.h"
// *****************************************************************************/

SpriteFrame::SpriteFrame(SDL_Surface *p_surface, unsigned int p_speed)
{
  this->surface = p_surface;
  this->delay = p_speed;
}

// *****************************************************************************/

SDL_Surface *newFlippedSurface(SDL_Surface *src, int fliph, int flipv)
{
	int		x, y;
	unsigned int	*srcbuff, *dstbuff; /* assuming unsigned int is 32bit */
	SDL_Surface	*flipped = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w,
		src->h, 32, src->format->Rmask, src->format->Gmask,
		src->format->Bmask, src->format->Amask);

	SDL_LockSurface(src);
	SDL_LockSurface(flipped);
	srcbuff = (unsigned int*)src->pixels;
	dstbuff = (unsigned int*)flipped->pixels;
	for (y=0;y<src->h;y++)
		for (x=0;x<src->w;x++)
			dstbuff[y*src->w + x] =
				srcbuff[(flipv?(src->h-y-1):y)*src->w +
				(fliph?(src->w-x-1):x)];
	SDL_UnlockSurface(flipped);
	SDL_UnlockSurface(src);
	
	return flipped;
}

// *****************************************************************************/


Sprite::Sprite()
{
   frame_width_pix = 0;
   frame_height_pix = 0;
   scale_x = 1.0f;
   scale_y = 1.0f;
   rotation_deg = 0.0f;
   current_frame = 0;
   frame_delta = 1;
   rot_hotspot = center;
   show = true;
   backward = false; 
   last_update = Wormux::temps.Lit();
   show_on_finish = show_last_frame;
   loop = false;
   finished = false;
}

Sprite::Sprite( const Sprite& other)
{
   frame_width_pix = other.frame_width_pix;
   frame_height_pix = other.frame_height_pix;
   scale_x = other.scale_x;
   scale_y = other.scale_y;
   rotation_deg = other.rotation_deg;
   current_frame = other.current_frame;
   rot_hotspot = center;
   frame_delta = other.frame_delta;
   show = other.show;
   backward = other.backward;
   last_update = other.last_update;
   show_on_finish = other.show_on_finish;
   loop = other.loop;
   finished = other.finished;
   
   for ( unsigned int f = 0 ; f < other.frames.size() ; f++)
     {
	  SDL_Surface *new_surf = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, 
							frame_width_pix, frame_height_pix, 
							32, // force to 32 bits per pixel
							0x000000ff,  // red mask
							0x0000ff00,  // green mask
							0x00ff0000,  // blue mask
							0xff000000); // alpha mask
	  // Disable per pixel alpha on the source surface
          // in order to properly copy the alpha chanel to the destination suface
	  // see the SDL_SetAlpha man page for more infos (RGBA->RGBA without SDL_SRCALPHA)
	  SDL_SetAlpha( other.frames[f].surface, 0, 0); 
	  SDL_BlitSurface( other.frames[f].surface, NULL, new_surf, NULL);
	  // re-enable the per pixel alpha in the 
	  SDL_SetAlpha( other.frames[f].surface, SDL_SRCALPHA, 0); 
	  frames.push_back( SpriteFrame(new_surf));
     }
}

Sprite::Sprite( SDL_Surface *surface)
{   
   frame_width_pix = surface->w;
   frame_height_pix = surface->h;
   frames.push_back( SpriteFrame(surface));
   
   scale_x = 1.0f;
   scale_y = 1.0f;
   rotation_deg = 0.0f;   
   current_frame = 0;
   rot_hotspot = center;
   frame_delta = 1;
   show = true;
   backward = false; 
   last_update = Wormux::temps.Lit();
   show_on_finish = show_last_frame;
   loop = false;
   finished = false;
}

Sprite::~Sprite()
{
   for ( unsigned int f = 0 ; f < frames.size() ; f++)
     SDL_FreeSurface( frames[f].surface);
}

void Sprite::Init( SDL_Surface *surface, int frame_width, int frame_height, int nb_frames_x, int nb_frames_y)
{
   this->frame_width_pix = frame_width;
   this->frame_height_pix = frame_height;

   SDL_SetAlpha( surface, 0, 0);
   
   for( unsigned int fy = 0 ; fy < (unsigned int)nb_frames_y ; fy++)
     for( unsigned int fx = 0 ; fx < (unsigned int)nb_frames_x ; fx++)
       {
	  SDL_Surface *new_surf = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, 
							frame_width, frame_height, 
							32, // force to 32 bits per pixel
							0x000000ff,  // red mask
							0x0000ff00,  // green mask
							0x00ff0000,  // blue mask
							0xff000000); // alpha mask
	  SDL_Rect sr = {fx*frame_width, fy*frame_width, frame_width, frame_height};
	  SDL_Rect dr = {0,0,frame_width,frame_height};
	  
	  SDL_BlitSurface( surface, &sr, new_surf, &dr);
	  frames.push_back( SpriteFrame(new_surf));
       }
}

unsigned int Sprite::GetWidth()
{
   return frame_width_pix;
}

unsigned int Sprite::GetHeight()
{
   return frame_height_pix;
}

unsigned int Sprite::GetFrameCount()
{
   return frames.size();
}

void Sprite::SetCurrentFrame( unsigned int frame_no)
{
  assert (frame_no < frames.size());
  current_frame = frame_no;
}

unsigned int Sprite::GetCurrentFrame() const
{
   return current_frame;
}

SpriteFrame& Sprite::operator[] (unsigned int index)
{ return frames.at(index); }

const SpriteFrame& Sprite::operator[] (unsigned int index) const
{ return frames.at(index); }

const SpriteFrame& Sprite::GetCurrentFrameObject() const
{
   return frames[current_frame];
}

void Sprite::Scale( float scale_x, float scale_y)
{
   this->scale_x = scale_x;
   this->scale_y = scale_y;
}

void Sprite::GetScaleFactors( float &scale_x, float &scale_y)
{
   scale_x = this->scale_x;
   scale_y = this->scale_y;
}

void Sprite::ScaleSize(int width, int height)
{
  scale_x = float(width)/float(frame_width_pix);
  scale_y = float(height)/float(frame_height_pix);
}

void Sprite::SetAlpha( float alpha)
{
   this->alpha = alpha;
}

float Sprite::GetAlpha()
{
   return alpha;
}

void Sprite::SetRotation_deg( float angle_deg)
{
   rotation_deg = angle_deg;
}

void Sprite::Calculate_Rotation_Offset(int & rot_x, int & rot_y, SDL_Surface* tmp_surface)
{
  const SpriteFrame& frame = GetCurrentFrameObject();
  const SDL_Surface& surface = *frame.surface;
   // Calculate offset of the depending on hotspot rotation position :

   //Do as if hotspot is center of picture:
   rot_x = (surface.w/2)-(tmp_surface->w/2);
   rot_y = (surface.h/2)-(tmp_surface->h/2);

   if(rot_hotspot == center)
      return;

   //TODO:Rewrite this function... It's quite dirty, i know :p
   //Distance between center of picture and hotspot:
   // Works with scale == 1 or -1

   float d = 0.0 ;
   //Angle between center of picture and hotspot:
   float d_angle = 0.0;

   //Rotoation angle in radians:
   float angle = rotation_deg * M_PI / 180.0;

   switch(rot_hotspot)
   {
   case top_left:
   case top_right:
   case bottom_left:
   case bottom_right:
      d = sqrt(surface.w * surface.w +
               surface.h * surface.h) / 2;
      break;
   case top_center:
   case bottom_center:
      d = surface.h / 2;
      break;
   case left_center:
   case right_center:
      d = surface.w / 2;
      break;
   default: break;
   }

   switch(rot_hotspot)
   {
   case top_center:
      d_angle = 0.0;
      break;
   case top_left:
      if(surface.w<surface.h)
        d_angle = atan(surface.w / surface.h);
      else
        d_angle = atan(surface.h / surface.w);
      break;
   case left_center:
      d_angle = M_PI_2;
      break;
   case bottom_left:
      if(surface.w<surface.h)
        d_angle = M_PI - atan(surface.w / surface.h);
      else
        d_angle = M_PI - atan(surface.h / surface.w);
      break;
   case bottom_center:
      d_angle = M_PI;
      break;
   case bottom_right:
      if(surface.w<surface.h)
        d_angle = M_PI + atan(surface.w / surface.h);
      else
        d_angle = M_PI + atan(surface.h / surface.w);
      break;
   case right_center:
      d_angle =  - M_PI_2;
      break;
   case top_right:
      if(surface.w<surface.h)
        d_angle = - atan(surface.w / surface.h);
      else
        d_angle = - atan(surface.h / surface.w);
      break;
   default: break;
   }

   if(this->scale_y > 0.0)
   {
      d_angle += M_PI_2;
   }
   else
   {
      d_angle = - d_angle - M_PI_2;
   }

   rot_x -= static_cast<int>(cos(angle - d_angle) * d);
   rot_y -= static_cast<int>(sin(angle - d_angle) * d);

   switch(rot_hotspot)
   {
   case top_left:
   case left_center:
   case bottom_left:
      rot_x -= static_cast<int>(scale_y * surface.w / 2);
      break;
   case top_right:
   case right_center:
   case bottom_right:
      rot_x += static_cast<int>(scale_y * surface.w / 2);
      break;
   default: break;
   }

   switch(rot_hotspot)
   {
   case top_left:
   case top_center:
   case top_right:
      rot_y -= surface.h / 2;
      break;
   case bottom_left:
   case bottom_center:
   case bottom_right:
      rot_y += surface.h / 2;
      break;
   default: break;
   }
}


void Sprite::Start()
{
   if (backward) {
     current_frame = frames.size()-1;
     frame_delta = -1;
   } else {
     current_frame = 0;
     frame_delta = 1;
   }
   show = true;
   loop = false;
   finished = false;
   last_update = Wormux::temps.Lit();
}

void Sprite::StartLoop()
{
  Start();
  loop = true;
}
void Sprite::SetPlayBackward(bool enable)
{
  backward = enable;
  if (backward)
    frame_delta = -1;
  else
    frame_delta = 1;
}

void Sprite::Blit( SDL_Surface *dest, unsigned int pos_x, unsigned int pos_y)
{
  if (!show) return;
/*
 SDL_Rect dr = { pos_x, pos_y, frame_width_pix, frame_height_pix};

 SDL_Surface *current;
 
 // do we need to scale ?
 bool scale = false;
 if ( fabs(scale_x)!=1 || fabs(scale_y)!=1 ) {
   scale = true;
   current = zoomSurface (frames[current_frame].surface, fabs(scale_x), fabs(scale_y), 1);
 } else {
   current = frames[current_frame].surface; 
 }

 // do we need to mirror ?
 int flip_h=0, flip_v=0;
 if (scale_x < 0) flip_h = 1;
 if (scale_y < 0) flip_v = 1;

 if (flip_h || flip_v) {
   SDL_Surface *tmp = newFlippedSurface(current,flip_h,flip_v);

//   SDL_Rect sr = { (nb_frames-current_frame-1)*frame_width_pix*fabs(scale_x), 0, 
//		   frame_width_pix, frame_height_pix}; // this is buggy, do not work in case of flip_v
   
   SDL_BlitSurface (tmp, NULL, dest, &dr);
   SDL_FreeSurface(tmp);

 } else {
//   SDL_Rect sr = { current_frame*frame_width_pix*fabs(scale_x), 0, frame_width_pix, frame_height_pix};
   SDL_BlitSurface (current, NULL, dest, &dr);
 }

 if (scale)
   SDL_FreeSurface(current);

*/
#ifndef __MINGW32__
   SDL_Surface *tmp_surface = rotozoomSurfaceXY (frames[current_frame].surface, -rotation_deg, scale_x, scale_y, SMOOTHING_OFF);

   // Calculate offset of the depending on hotspot rotation position :
   int rot_x=0;
   int rot_y=0;
   if(rotation_deg!=0.0)
     Calculate_Rotation_Offset(rot_x, rot_y, tmp_surface);

   SDL_Rect dr = {pos_x + rot_x, pos_y + rot_y, frame_width_pix, frame_height_pix};

   SDL_BlitSurface (tmp_surface, NULL, dest, &dr);

   SDL_FreeSurface (tmp_surface);
#else
   SDL_Rect dr = {pos_x , pos_y , frame_width_pix, frame_height_pix};
   SDL_BlitSurface (frames[current_frame].surface, NULL, dest, &dr);
#endif


}

void Sprite::Finish()
{
  finished = true;
  loop = false;
  switch(show_on_finish)
  {
  case show_blank:
    current_frame = 0;
    show = false;
    break;      
  default:
  case show_last_frame:
    current_frame = frames.size()-1;
    break;      
  }
  frame_delta = 0;
}

void Sprite::Update()
{
  if (finished) return;
  if (Wormux::temps.Lit() < (last_update + GetCurrentFrameObject().delay))
     return;
   last_update = Wormux::temps.Lit();
   bool finish;
   if (frame_delta < 0)
     finish = (current_frame + frame_delta) <= 0;
   else
     finish = (frames.size()-1) <= (current_frame + frame_delta);
   if (finish && !loop)
      Finish();
   else
     current_frame = ( current_frame + frame_delta ) % frames.size();
}

void Sprite::Draw(int pos_x, int pos_y)
{
  if (!show) return;
  Blit(app.sdlwindow,pos_x - camera.GetX(),pos_y - camera.GetY());
}

void Sprite::Show() { show = true; }
void Sprite::Hide() { show = false; }
void Sprite::SetShowOnFinish(SpriteShowOnFinish show) { show_on_finish = show; }
bool Sprite::IsFinished() const { return finished; }

//-----------------------------------------------------------------------------

