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
 * Ciel : ce qui est affiché en image de fond.
 *****************************************************************************/

#include "sky.h"
//-----------------------------------------------------------------------------
#include "camera.h"
#include "map.h"
#include "maps_list.h"
#ifndef CL
#include <SDL.h>
#include "../include/app.h"
#endif
//-----------------------------------------------------------------------------
namespace Wormux
{

// Vitesse (comprise entre 0 et 0.5)
const double VITESSE_CIEL_X = 0.3;
const double VITESSE_CIEL_Y = 1;

//-----------------------------------------------------------------------------

Ciel::Ciel()
{
#ifdef CL
  image = NULL;
#endif
}

//-----------------------------------------------------------------------------

void Ciel::Init()
{
#ifdef CL
  CL_Surface *m_image = new CL_Surface(lst_terrain.TerrainActif().LitImgCiel());
  LoadImage (*m_image);
  delete m_image;
#else
   // That is temporary -> image will be loaded directly without alpha chanel
   SDL_Surface *tmp_image = lst_terrain.TerrainActif().LitImgCiel();
   SDL_SetAlpha( tmp_image, 0, 0);
   image = SDL_DisplayFormat( tmp_image);
   SDL_FreeSurface( tmp_image);
#endif
}

//-----------------------------------------------------------------------------

void Ciel::Reset()
{
  Init();
}

//-----------------------------------------------------------------------------

void Ciel::Draw()
{
   int x = static_cast<int>(camera.GetX() * VITESSE_CIEL_X);
   int y = static_cast<int>(camera.GetY() * VITESSE_CIEL_Y);
#if 0
  static int lastx = 0;
  static int lasty = 0;
  if (lastx == x && lasty == y) return;
  lastx = x;
  lasty = y;
#endif

   if(!TerrainActif().infinite_bg)
   {
     SDL_Rect ds = {x, y,app.sdlwindow->w,app.sdlwindow->h};
     SDL_Rect dr = {0,0,app.sdlwindow->w,app.sdlwindow->h};
     SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);
   }
   else
   {
     int w,h;

     while(x<0)
       x += image->w;
     while(x>image->w)
       x -= image->w;
     while(y<0)
       y += image->h;
     while(y>image->h)
       y -= image->h;

     w = image->w - x;
     if(w >= static_cast<int>(camera.GetWidth()))
       w = camera.GetWidth();

     h = image->h - y;
     if(h >= static_cast<int>(camera.GetHeight()))
       h = camera.GetHeight();

     SDL_Rect ds = {x, y, w, h};
     SDL_Rect dr = {0,0, w, h};
     SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);

     if(w < static_cast<int>(camera.GetWidth()))
     {
       SDL_Rect ds = {x+w-image->w, y, (int)camera.GetWidth()-w, h};
       SDL_Rect dr = {w,0, (int)camera.GetWidth()-w, h};
       SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);
     }
     if(h < static_cast<int>(camera.GetHeight()))
     {
       SDL_Rect ds = {x, y+h-image->h, w, (int)camera.GetHeight()-h};
       SDL_Rect dr = {0,h, w, (int)camera.GetHeight()-h};
       SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);
     }
     if(w < static_cast<int>(camera.GetWidth()) && h < static_cast<int>(camera.GetHeight()))
     {
       SDL_Rect ds = {x+w-image->w, y+h-image->h, camera.GetWidth()-w, camera.GetHeight()-h};
       SDL_Rect dr = {w,h, camera.GetWidth()-w, camera.GetHeight()-h};
       SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);
     }

   }
}

//-----------------------------------------------------------------------------
} // namespace Wormux
