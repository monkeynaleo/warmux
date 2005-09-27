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

Sprite::Sprite()
{
   surface = NULL;
   frame_width_pix = 0;
   frame_height_pix = 0;
   nb_frames = 0;
   scale_x = 1.0f;
   scale_y = 1.0f;
   rotation_deg = 0.0f;
}

Sprite::~Sprite()
{
   if ( surface )
     SDL_FreeSurface( surface);
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
   return nb_frames;
}

void Sprite::SetCurrentFrame( unsigned int frame_no)
{
   current_frame = ( frame_no < nb_frames ) ? frame_no : nb_frames-1;
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
   current_frame = nb_frames-1;
}

void Sprite::Blit( SDL_Surface *dest, unsigned int pox_x, unsigned int pos_y)
{
   //current_frame;
   SDL_Rect sr = { current_frame*frame_width_pix, 0, frame_width_pix, frame_height_pix};
   SDL_Rect dr = { pox_x, pos_y, frame_width_pix, frame_height_pix};
   SDL_BlitSurface( surface, &sr, dest, &dr);
}

void Sprite::Update()
{
   current_frame = ( current_frame + 1 ) % nb_frames;
}

void Sprite::Draw(int pos_x, int pos_y)
{
  Blit(app.sdlwindow,pos_x - camera.GetX(),pos_y - camera.GetY());
}

