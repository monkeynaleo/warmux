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
}

Sprite::Sprite( const Sprite& other)
{
   frame_width_pix = other.frame_width_pix;
   frame_height_pix = other.frame_height_pix;
   scale_x = other.scale_x;
   scale_y = other.scale_y;
   rotation_deg = other.rotation_deg;
   current_frame = other.current_frame;
   
   for ( unsigned int f = 0 ; f < other.surfaces.size() ; f++)
     {
	  SDL_Surface *new_surf = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, 
							frame_width_pix, frame_height_pix, 
							32, // force to 32 bits per pixel
							0x000000ff,  // red mask
							0x0000ff00,  // green mask
							0x00ff0000,  // blue mask
							0xff000000); // alpha mask
	  SDL_BlitSurface( other.surfaces[f], NULL, new_surf, NULL);
	  surfaces.push_back( new_surf);
     }
}

Sprite::Sprite( SDL_Surface *surface)
{   
   frame_width_pix = surface->w;
   frame_height_pix = surface->h;
   surfaces.push_back( surface);
   
   scale_x = 1.0f;
   scale_y = 1.0f;
   rotation_deg = 0.0f;   
   current_frame = 0;
}

Sprite::~Sprite()
{
   for ( unsigned int f = 0 ; f < surfaces.size() ; f++)
     SDL_FreeSurface( surfaces[f]);
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
	  surfaces.push_back( new_surf);
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
   return surfaces.size();
}

void Sprite::SetCurrentFrame( unsigned int frame_no)
{
   current_frame = ( frame_no < surfaces.size() ) ? frame_no : surfaces.size()-1;
}

unsigned int Sprite::GetCurrentFrame()
{
   return current_frame;
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

void Sprite::Start()
{
   current_frame = 0;
}

void Sprite::StartLoop()
{
   current_frame = 0;
}
void Sprite::Finish()
{
   current_frame = surfaces.size()-1;
}

void Sprite::Blit( SDL_Surface *dest, unsigned int pos_x, unsigned int pos_y)
{
/*
 SDL_Rect dr = { pos_x, pos_y, frame_width_pix, frame_height_pix};

 SDL_Surface *current;
 
 // do we need to scale ?
 bool scale = false;
 if ( fabs(scale_x)!=1 || fabs(scale_y)!=1 ) {
   scale = true;
   current = zoomSurface (surfaces[current_frame], fabs(scale_x), fabs(scale_y), 1);
 } else {
   current = surfaces[current_frame]; 
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
   SDL_Surface *tmp_surface = rotozoomSurfaceXY (surfaces[current_frame], rotation_deg, scale_x, scale_y, SMOOTHING_OFF);

   SDL_Rect dr = {pos_x, pos_y, frame_width_pix, frame_height_pix};

   SDL_BlitSurface (tmp_surface, NULL, dest, &dr);

   SDL_FreeSurface (tmp_surface);
}

void Sprite::Update()
{
   current_frame = ( current_frame + 1 ) % surfaces.size();
}

void Sprite::Draw(int pos_x, int pos_y)
{
  Blit(app.sdlwindow,pos_x - camera.GetX(),pos_y - camera.GetY());
}


//-----------------------------------------------------------------------------

