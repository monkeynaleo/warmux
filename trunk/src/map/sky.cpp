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
 * Sky: background of the map
 *****************************************************************************/

#include "sky.h"
//-----------------------------------------------------------------------------
#include "camera.h"
#include "map.h"
#include "maps_list.h"
#include <limits.h>
#include <SDL.h>
#include "../include/app.h"
#include <iostream>
//-----------------------------------------------------------------------------
namespace Wormux
{

// Vitesse (comprise entre 0 et 0.5)
const double VITESSE_CIEL_X = 0.3;
const double VITESSE_CIEL_Y = 1;

//-----------------------------------------------------------------------------

Sky::Sky()
{
  image = NULL;
}

//-----------------------------------------------------------------------------

void Sky::Init()
{
   // That is temporary -> image will be loaded directly without alpha chanel
   SDL_Surface *tmp_image = lst_terrain.TerrainActif().LitImgCiel();
   SDL_SetAlpha(tmp_image, 0, 0);
   image = SDL_DisplayFormat(tmp_image);
}

//-----------------------------------------------------------------------------

void Sky::Reset()
{
  Init();
  lastx = lasty = INT_MAX;
}

//-----------------------------------------------------------------------------

void Sky::CompleteDraw()
{
   int x = static_cast<int>(camera.GetX() * VITESSE_CIEL_X);
   int y = static_cast<int>(camera.GetY() * VITESSE_CIEL_Y);

   if(!TerrainActif().infinite_bg)
   {    
     uint margin_x = 0, margin_y = 0;
     
     if (image->w < int(camera.GetWidth())) {
       x = 0;
       margin_x = (camera.GetWidth()-image->w)/2;
     }
     if (image->h < int(camera.GetHeight())) {
       y = 0;
       margin_y = (camera.GetHeight()-image->h)/2;
     }
     SDL_Rect ds = {x, y,camera.GetWidth(),camera.GetHeight()};
     SDL_Rect dr = {margin_x,margin_y,camera.GetWidth(),camera.GetHeight()};
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

void Sky::Draw()
{
#if defined(WIN32)
  // TODO: Why the cache doesn't work on Windows!?
  CompleteDraw();
#else  
  int cx = camera.GetX();
  int cy = camera.GetY();

  if (lastx != cx || lasty != cy) {
    CompleteDraw();
    lastx = cx;
    lasty = cy;
    return;
  }

  lastx = cx;
  lasty = cy;

  int sky_cx = static_cast<int>(camera.GetX() * VITESSE_CIEL_X);
  int sky_cy = static_cast<int>(camera.GetY() * VITESSE_CIEL_Y);
  
  uint margin_x = 0, margin_y = 0;
     
  if (image->w < int(camera.GetWidth())) {
    sky_cx = 0;
    margin_x = (camera.GetWidth()-image->w)/2;
  }
  if (image->h < int(camera.GetHeight())) {
    sky_cy = 0;
    margin_y = (camera.GetHeight()-image->h)/2;
  }

  std::list<Rectanglei>::iterator it;
  for (it = world.to_redraw_now->begin(); 
       it != world.to_redraw_now->end(); 
       ++it)
  {
    SDL_Rect ds = { sky_cx + it->x - cx -margin_x, 
		    sky_cy + it->y - cy -margin_y, 
		    it->w+1, 
		    it->h+1};
    SDL_Rect dr = {it->x-cx,
		   it->y-cy,
		   it->w+1, 
		   it->h+1};
    SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);
  }

  for (it = world.to_redraw_particles_now->begin(); 
       it != world.to_redraw_particles_now->end(); 
       ++it)
  {
    SDL_Rect ds = { sky_cx + it->x - cx -margin_x, 
		    sky_cy + it->y - cy -margin_y, 
		    it->w+1, 
		    it->h+1};
    SDL_Rect dr = {it->x-cx,
		   it->y-cy,
		   it->w+1, 
		   it->h+1};
    SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);
  }
#endif
}

//-----------------------------------------------------------------------------
} // namespace Wormux
