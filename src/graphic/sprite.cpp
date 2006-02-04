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
#include <SDL.h>
#include <SDL_rotozoom.h>
#include <iostream>
#include "sdlsurface.h"
#include "surface.h"
#include "../game/game.h"
#include "../game/time.h"
#include "../include/app.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../tool/rectangle.h"

#ifdef DBG_SPRITE
#include <sstream>
#endif

Sprite::Sprite(){
   frame_width_pix = 0;
   frame_height_pix = 0;
   scale_x = 1.0f;
   scale_y = 1.0f;
   alpha = 1.0f;
   rotation_deg = 0.0f;
   speed_factor = 1.0f;
   current_frame = 0;
   frame_delta = 1;
   rot_hotspot = center;
   show = true;
   last_update = global_time.Read();
   show_on_finish = show_last_frame;
   loop = true;
   pingpong = false;
   finished = false;
   have_rotation_cache = false;
   have_flipping_cache = false;
   have_lastframe_cache = false;
#ifdef DBG_SPRITE
   info = new Text("");
#endif
}

Sprite::Sprite( Sprite& other){
   frame_width_pix = other.frame_width_pix;
   frame_height_pix = other.frame_height_pix;
   scale_x = other.scale_x;
   scale_y = other.scale_y;
   alpha = other.alpha;
   rotation_deg = other.rotation_deg;
   speed_factor = other.speed_factor;
   current_frame = other.current_frame;
   rot_hotspot = center;
   frame_delta = other.frame_delta;
   show = other.show;
   last_update = other.last_update;
   show_on_finish = other.show_on_finish;
   loop = other.loop;
   pingpong = other.pingpong;
   finished = other.finished;
   have_rotation_cache = false;
   have_flipping_cache = false;
   have_lastframe_cache = false;

   for ( unsigned int f = 0 ; f < other.frames.size() ; f++)
     {
       Surface new_surf = Surface(frame_width_pix, frame_height_pix, SDL_SWSURFACE|SDL_SRCALPHA, true);

	  // Disable per pixel alpha on the source surface
      // in order to properly copy the alpha chanel to the destination suface
	  // see the SDL_SetAlpha man page for more infos (RGBA->RGBA without SDL_SRCALPHA)
	  other.frames[f].surface.SetAlpha( 0, 0); 
	  new_surf.Blit( other.frames[f].surface, NULL, NULL);
	  // re-enable the per pixel alpha in the 
	  other.frames[f].surface.SetAlpha( SDL_SRCALPHA, 0); 
	  frames.push_back( SpriteFrame(new_surf,other.frames[f].delay));
     }
     if(other.have_rotation_cache)
       EnableRotationCache(other.rotation_cache_size);
     if(other.have_flipping_cache)
       EnableFlippingCache();
     if(other.have_lastframe_cache)
       EnableLastFrameCache();
#ifdef DBG_SPRITE
   info = new Text("");
#endif
}

Sprite::Sprite( Surface surface){
   frame_width_pix = surface.GetWidth();
   frame_height_pix = surface.GetHeight();
   frames.push_back( SpriteFrame(surface));
   
   scale_x = 1.0f;
   scale_y = 1.0f;
   alpha = 1.0f;
   rotation_deg = 0.0f;   
   speed_factor = 1.0f;
   current_frame = 0;
   rot_hotspot = center;
   frame_delta = 1;
   show = true;
   last_update = global_time.Read();
   show_on_finish = show_last_frame;
   loop = true;
   pingpong = false;
   finished = false;
   have_rotation_cache = false;
   have_flipping_cache = false;
   have_lastframe_cache = false;
#ifdef DBG_SPRITE
   info = new Text("");
#endif
}

Sprite::~Sprite(){
  if(have_rotation_cache)
  {
    for ( unsigned int f = 0 ; f < frames.size() ; f++)
    {
      delete []frames[f].rotated_surface;
      if(have_flipping_cache)
        delete []frames[f].rotated_surface;
    }
  }

#ifdef DBG_SPRITE
   delete info;
#endif
}

void Sprite::Init( Surface surface, int frame_width, int frame_height, int nb_frames_x, int nb_frames_y){
   this->frame_width_pix = frame_width;
   this->frame_height_pix = frame_height;

   surface.SetAlpha( 0, 0);
   
   for( unsigned int fy = 0 ; fy < (unsigned int)nb_frames_y ; fy++)
     for( unsigned int fx = 0 ; fx < (unsigned int)nb_frames_x ; fx++)
       {
		   Surface new_surf = Surface(frame_width, frame_height, SDL_SWSURFACE|SDL_SRCALPHA, true);
	  SDL_Rect sr = {fx*frame_width, fy*frame_width, frame_width, frame_height};
	  SDL_Rect dr = {0,0,frame_width,frame_height};
	  
	    new_surf.Blit( surface, &sr, &dr);
	  frames.push_back( SpriteFrame(new_surf));
       }
}

void Sprite::AddFrame(Surface surf, unsigned int delay){
	  frames.push_back( SpriteFrame(surf, delay) );
}

void Sprite::EnableRotationCache(unsigned int cache_size){
  //For each frame, we pre-render 'cache_size' rotated surface
  //At runtime the prerender SDL_Surface with the nearest angle to what is asked is displayed
  assert(!have_lastframe_cache);
  assert(!have_rotation_cache);
  assert(!have_flipping_cache); //Always compute rotation cache before flipping cache!
  assert(cache_size > 1);
  assert(cache_size <= 360);

  rotation_cache_size = cache_size;
  have_rotation_cache = true;

  for ( unsigned int f = 0 ; f < frames.size() ; f++)
  {
    frames[f].rotated_surface=new Surface[cache_size];
    frames[f].rotated_surface[0]=frames[f].surface;
    for(unsigned int i=1 ; i< cache_size ; i++){
		Surface rotatedSurface = frames[f].surface.RotoZoomXY( -360.0 * (float) i / (float) cache_size, 1.0, 1.0, SMOOTHING_ON);
		frames[f].rotated_surface[i] = rotatedSurface;
	}
  }
}

void Sprite::EnableFlippingCache(){
  //For each frame, we pre-render the flipped frame
  assert(!have_flipping_cache);
  assert(!have_lastframe_cache);

  have_flipping_cache = true;

  for ( unsigned int f = 0 ; f < frames.size() ; f++)
  {
	  Surface flippedSurface = frames[f].surface.RotoZoomXY( 0.0, -1.0, 1.0, SMOOTHING_OFF);	  
	  frames[f].flipped_surface = flippedSurface;
    if(have_rotation_cache)
    {
      frames[f].rotated_flipped_surface=new Surface[rotation_cache_size];
      frames[f].rotated_flipped_surface[0]=frames[f].flipped_surface;
      for(unsigned int i=1 ; i< rotation_cache_size ; i++)
      {
		  Surface rotatedFlippedSurface = frames[f].surface.RotoZoomXY(-360.0 * (float) i / (float) rotation_cache_size, -1.0, 1.0, SMOOTHING_ON);
		  frames[f].rotated_flipped_surface[i] = rotatedFlippedSurface;
      }
    }
  }
}

void Sprite::EnableLastFrameCache(){
  //The result of the last call to SDLgfx is kept in memory
  //to display it again if rotation / scale / alpha didn't changed
  assert(!have_rotation_cache);
  assert(!have_flipping_cache);
  have_lastframe_cache = true;
}

void Sprite::DisableLastFrameCache(){
  //The result of the last call to SDLgfx is kept in memory
  //to display it again if rotation / scale / alpha didn't changed
  assert(!have_rotation_cache);
  assert(!have_flipping_cache);
  have_lastframe_cache = false;
}

void Sprite::LastFrameModified(){
  //Free lastframe_cache if the next frame to be displayed
  //is not the same as the last one.
  if(!have_lastframe_cache) 
    return;
  last_frame.Free();
}

void Sprite::SetSize(unsigned int w, unsigned int h){
  assert(frame_width_pix == 0 && frame_height_pix == 0)
	frame_width_pix = w;
	frame_height_pix = h;
}

unsigned int Sprite::GetWidth(){
   return (uint)((float)frame_width_pix * (scale_x>0?scale_x:-scale_x));
}

unsigned int Sprite::GetHeight(){
   return (uint)((float)frame_height_pix * (scale_y>0?scale_y:-scale_y));
}

unsigned int Sprite::GetFrameCount(){
   return frames.size();
}

void Sprite::SetCurrentFrame( unsigned int frame_no){
  assert (frame_no < frames.size());
  current_frame = frame_no;
  LastFrameModified();
}

unsigned int Sprite::GetCurrentFrame() const{
   return current_frame;
}

SpriteFrame& Sprite::operator[] (unsigned int index){
  return frames.at(index);
}

const SpriteFrame& Sprite::operator[] (unsigned int index) const{
  return frames.at(index);
}

const SpriteFrame& Sprite::GetCurrentFrameObject() const{
   return frames[current_frame];
}

void Sprite::Scale( float scale_x, float scale_y){
   if(this->scale_x == scale_x && this->scale_y == scale_y) return;
   this->scale_x = scale_x;
   this->scale_y = scale_y;
   LastFrameModified();
}

void Sprite::ScaleSize(int width, int height){
  Scale(float(width)/float(frame_width_pix),
        float(height)/float(frame_height_pix));
}

void Sprite::GetScaleFactors( float &scale_x, float &scale_y){
   scale_x = this->scale_x;
   scale_y = this->scale_y;
}

void Sprite::SetFrameSpeed(unsigned int nv_fs){
   for ( unsigned int f = 0 ; f < frames.size() ; f++)
     frames[f].delay = nv_fs;
}

void Sprite::SetSpeedFactor( float nv_speed){
  speed_factor = nv_speed;
}

void Sprite::SetAlpha( float alpha){
  if(this->alpha == alpha)
    return;
  this->alpha = alpha;
  LastFrameModified();
}

float Sprite::GetAlpha(){
  return alpha;
}

void Sprite::SetRotation_deg( float angle_deg){
   while(angle_deg >= 360.0)
     angle_deg -= 360.0;
   while(angle_deg < 0.0)
     angle_deg += 360.0;

   if(rotation_deg == angle_deg) return;

   rotation_deg = angle_deg;
   LastFrameModified();
}

void Sprite::Calculate_Rotation_Offset(int & rot_x, int & rot_y, Surface tmp_surface){
    const SpriteFrame& frame = GetCurrentFrameObject();
    const Surface &surface = frame.surface;
    // Calculate offset of the depending on hotspot rotation position :
  
    int surfaceHeight = surface.GetHeight();
	int surfaceWidth = surface.GetWidth();

   //Do as if hotspot is center of picture:
   rot_x = surfaceWidth / 2 - tmp_surface.GetWidth() / 2;
   rot_y = surfaceHeight / 2 - tmp_surface.GetHeight() / 2;

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
      d = sqrt(surfaceWidth * surfaceWidth +
               surfaceHeight * surfaceHeight) / 2;
      break;
   case top_center:
   case bottom_center:
      d = surfaceHeight / 2;
      break;
   case left_center:
   case right_center:
      d = surfaceWidth / 2;
      break;
   default: break;
   }

   switch(rot_hotspot)
   {
   case top_center:
      d_angle = 0.0;
      break;
   case top_left:
      if(surfaceWidth<surfaceHeight)
        d_angle = atan(surfaceWidth / surfaceHeight);
      else
        d_angle = atan(surfaceHeight / surfaceWidth);
      break;
   case left_center:
      d_angle = M_PI_2;
      break;
   case bottom_left:
      if(surfaceWidth<surfaceHeight)
        d_angle = M_PI - atan(surfaceWidth / surfaceHeight);
      else
        d_angle = M_PI - atan(surfaceHeight / surfaceWidth);
      break;
   case bottom_center:
      d_angle = M_PI;
      break;
   case bottom_right:
      if(surfaceWidth<surfaceHeight)
        d_angle = M_PI + atan(surfaceWidth / surfaceHeight);
      else
        d_angle = M_PI + atan(surfaceHeight / surfaceWidth);
      break;
   case right_center:
      d_angle =  - M_PI_2;
      break;
   case top_right:
      if(surfaceWidth<surfaceHeight)
        d_angle = - atan(surfaceWidth / surfaceHeight);
      else
        d_angle = - atan(surfaceHeight / surfaceWidth);
      break;
   default: break;
   }

   if(this->scale_y > 0.0)
      d_angle += M_PI_2;
   else
      d_angle = - d_angle - M_PI_2;

   rot_x -= static_cast<int>(cos(angle - d_angle) * d);
   rot_y -= static_cast<int>(sin(angle - d_angle) * d);

   switch(rot_hotspot)
   {
   case top_left:
   case left_center:
   case bottom_left:
      rot_x -= static_cast<int>(scale_y * surfaceWidth / 2);
      break;
   case top_right:
   case right_center:
   case bottom_right:
      rot_x += static_cast<int>(scale_y * surfaceWidth / 2);
      break;
   default: break;
   }

   switch(rot_hotspot)
   {
   case top_left:
   case top_center:
   case top_right:
      rot_y -= surfaceHeight / 2;
      break;
   case bottom_left:
   case bottom_center:
   case bottom_right:
      rot_y += surfaceHeight / 2;
      break;
   default: break;
   }
}

void Sprite::Start(){
   show = true;
   finished = false;
   last_update = global_time.Read();
   LastFrameModified();
}

void Sprite::SetPlayBackward(bool enable){
  if (enable)
    frame_delta = -1;
  else 
    frame_delta = 1;
}

#ifdef DEBUG
#define CACHE_WARNING std::cout << "Warning : Sprite uses SDL_gfx while flipping or rotation cache is enabled!" << std::endl;
#else
#define CACHE_WARNING
#endif

void Sprite::RefreshSurface()
{
  tmp_surface.Free();

  if(!have_rotation_cache && !have_flipping_cache)
  {
    if(!have_lastframe_cache)
    {
		Surface newTmp = frames[current_frame].surface.RotoZoomXY( -rotation_deg, scale_x, scale_y, SMOOTHING_OFF);
      tmp_surface = newTmp;
    }
    else
    {
      if(last_frame.IsNull() )
      {
#define BUGGY_SDLGFX
#ifdef BUGGY_SDLGFX
        if(rotation_deg != 0.0 || (scale_x != 1.0 && scale_y == 1.0))
        {
		  Surface newTmp = frames[current_frame].surface.RotoZoomXY( -rotation_deg, scale_x, scale_y, SMOOTHING_OFF);
		  tmp_surface = newTmp;
          last_frame = tmp_surface;
        }
        else
        if(scale_x != 1.0 || scale_y != 1.0)
        {
#endif
		  Surface newTmp = frames[current_frame].surface.RotoZoomXY( -rotation_deg, scale_x, scale_y, SMOOTHING_ON);
		  tmp_surface = newTmp;
          last_frame = tmp_surface;
#ifdef BUGGY_SDLGFX
        }
        else
        {
          tmp_surface = frames[current_frame].surface;
          last_frame.Free();
        }
#endif
      }
      else
      {
        tmp_surface = last_frame;
      }
    }
  }
  else
  {
    if(have_flipping_cache && !have_rotation_cache)
    {
      if(rotation_deg != 0.0 || scale_y != 1.0 || (scale_x != 1.0 && scale_x != -1.0))
      {
        CACHE_WARNING;
		Surface newTmp = frames[current_frame].surface.RotoZoomXY( -rotation_deg, scale_x, scale_y, SMOOTHING_OFF );
        tmp_surface = newTmp;
      }
      else
      if(scale_x == 1.0)
        tmp_surface = frames[current_frame].surface;
      else
        tmp_surface = frames[current_frame].flipped_surface;
    }
    else
    if(!have_flipping_cache && have_rotation_cache)
    {
      if(scale_x != 1.0 || scale_y != 1.0)
      {
        CACHE_WARNING;
		Surface newTmp = frames[current_frame].surface.RotoZoomXY( -rotation_deg, scale_x, scale_y, SMOOTHING_OFF);
        tmp_surface = newTmp;
      }
      else
        tmp_surface = frames[current_frame].rotated_surface[(unsigned int)rotation_deg*rotation_cache_size/360];
    }
    else
    {
      //have_flipping_cache==true && have_rotation_cache==true
      if((scale_x != 1.0 && scale_x != -1.0)  || scale_y != 1.0)
      {
        CACHE_WARNING;
		Surface newTmp = frames[current_frame].surface.RotoZoomXY( -rotation_deg, scale_x, scale_y, SMOOTHING_OFF);
        tmp_surface = newTmp;
      }
      else
      {
        //Scale_y == 1.0
        if(scale_x == 1.0)
          tmp_surface = frames[current_frame].rotated_surface[(unsigned int)rotation_deg*rotation_cache_size/360];
        else
          tmp_surface = frames[current_frame].rotated_flipped_surface[(unsigned int)rotation_deg*rotation_cache_size/360];
      }
    }
  }
  assert( !tmp_surface.IsNull() );
}
void Sprite::Blit( Surface dest, uint pos_x, uint pos_y)
{
  if (!show) return;

  RefreshSurface();
   // Calculate offset of the depending on hotspot rotation position :
  int rot_x=0;
  int rot_y=0;
  if( rotation_deg!=0.0 )
    Calculate_Rotation_Offset(rot_x, rot_y, tmp_surface);

  int x = pos_x + rot_x;
  int y = pos_y + rot_y;

  SDL_Rect dr = {x, y, tmp_surface.GetWidth(), tmp_surface.GetHeight() };

  dest.Blit( tmp_surface, NULL, &dr);

  // For the cache mechanism
  if( game.IsGameLaunched() )
    world.ToRedrawOnScreen(Rectanglei(x, y, tmp_surface.GetWidth(), tmp_surface.GetHeight() ));

#ifdef DBG_SPRITE
   std::ostringstream ss;
   ss << pos_x << "," << pos_y << " " << current_frame << "/" << frames.size() << " L" << loop << " P" << pingpong << " R" << rotation_deg;
   std::string s = ss.str();   
   info->Set(s);
   info->DrawTopLeft(pos_x + frame_width_pix, pos_y);
#endif
}

void Sprite::Blit( Surface dest, int pos_x, int pos_y, int src_x, int src_y, uint w, uint h)
{
  if (!show) return;

  RefreshSurface();
   // Calculate offset of the depending on hotspot rotation position :
  int rot_x=0;
  int rot_y=0;
  if( rotation_deg!=0.0 )
    Calculate_Rotation_Offset(rot_x, rot_y, tmp_surface);

  int x = pos_x + rot_x;
  int y = pos_y + rot_y;

  SDL_Rect src = {src_x, src_y, w, h};
  SDL_Rect dst = {x, y, w, h};
  dest.Blit(tmp_surface, &src, &dst);

  // For the cache mechanism
  if( game.IsGameLaunched() )
    world.ToRedrawOnScreen(Rectanglei(x, y, tmp_surface.GetWidth(), tmp_surface.GetHeight() ));

#ifdef DBG_SPRITE
   std::ostringstream ss;
   ss << pos_x << "," << pos_y << " " << current_frame << "/" << frames.size() << " L" << loop << " P" << pingpong << " R" << rotation_deg;
   std::string s = ss.str();   
   info->Set(s);
   info->DrawTopLeft(pos_x + frame_width_pix, pos_y);
#endif
}

void Sprite::Finish(){
  finished = true;
  switch(show_on_finish)
  {
  case show_first_frame:
    current_frame = 0;
    break;      
  case show_blank:
    show = false;
    break;      
  default:
  case show_last_frame:
    current_frame = frames.size()-1;
    break;      
  }
  LastFrameModified();
}

void Sprite::Update(){
  if (finished) return;
  if (global_time.Read() < (last_update + GetCurrentFrameObject().delay))
     return;

  //Delta to next frame used to enable frameskip
  //if delay between 2 frame is < fps
  int delta_to_next_f = (int)((float)((global_time.Read() - last_update) / GetCurrentFrameObject().delay) * speed_factor);
  last_update += (int)((float)(delta_to_next_f * GetCurrentFrameObject().delay) / speed_factor);

  //Animation is finished, when last frame have been fully played
  bool finish;
  if (frame_delta < 0)
    finish = ((int)current_frame + frame_delta * delta_to_next_f) <= -1;
  else
    finish = frames.size() <= (current_frame + frame_delta * delta_to_next_f);

  if (finish && !loop && (!pingpong || frame_delta < 0))
     Finish();
  else
  {
    unsigned int next_frame = ( current_frame + frame_delta * delta_to_next_f ) % frames.size();

    if(pingpong)
    {
      if( frame_delta>0 && ( current_frame + frame_delta * delta_to_next_f )>=frames.size())
      {
        next_frame = frames.size() - next_frame -2;
        frame_delta = - frame_delta;
      }
      else
      if( frame_delta<0 && ( (int)current_frame + frame_delta * delta_to_next_f ) <= -1)
      {
        next_frame = (-((int)current_frame + frame_delta * delta_to_next_f )) % frames.size();
        frame_delta = - frame_delta;
      }
    }

    if(next_frame != current_frame)
    {
      if(!(next_frame >= 0 && next_frame < frames.size()))
      {
        next_frame = 0;
      }
      LastFrameModified();
      current_frame = next_frame;
    }
  }
}

void Sprite::Draw(int pos_x, int pos_y){
  if( !show )
    return;
  Blit( app.video.window.GetSurface(), pos_x - camera.GetX(), pos_y - camera.GetY() );
}

void Sprite::Show() { show = true; }
void Sprite::Hide() { show = false; }
void Sprite::SetShowOnFinish(SpriteShowOnFinish show) { show_on_finish = show; loop = false;}
bool Sprite::IsFinished() const { return finished; }

