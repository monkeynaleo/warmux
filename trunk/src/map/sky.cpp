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
#include <iostream>
#include <limits.h>
#include <SDL.h>
#include "camera.h"
#include "map.h"
#include "maps_list.h"
#include "../graphic/surface.h"
#include "../include/app.h"

// Vitesse (comprise entre 0 et 0.5)
const double VITESSE_CIEL_X = 0.3;
const double VITESSE_CIEL_Y = 1;

Sky::Sky(){
}

void Sky::Init(){
   // That is temporary -> image will be loaded directly without alpha chanel
   Surface tmp_image = lst_terrain.TerrainActif().LitImgCiel();
   tmp_image.SetAlpha( 0, 0);
   image = tmp_image.DisplayFormat();
}

void Sky::Reset(){
  Init();
  lastx = lasty = INT_MAX;
}

void Sky::CompleteDraw(){
   int x = static_cast<int>(camera.GetX() * VITESSE_CIEL_X);
   int y = static_cast<int>(camera.GetY() * VITESSE_CIEL_Y);

   if(!TerrainActif().infinite_bg)
   {    
     uint margin_x = 0, margin_y = 0;
     
     if (image.GetWidth() < int(camera.GetWidth())) {
       x = 0;
       margin_x = (camera.GetWidth()-image.GetWidth())/2;
     }
     if (image.GetHeight() < int(camera.GetHeight())) {
       y = 0;
       margin_y = (camera.GetHeight()-image.GetHeight())/2;
     }
     SDL_Rect ds = {x, y,camera.GetWidth(),camera.GetHeight()};
     SDL_Rect dr = {margin_x,margin_y,camera.GetWidth(),camera.GetHeight()};
     app.video.window.Blit( image, &ds, &dr);
   }
   else
   {
     int w,h;

     while(x<0)
       x += image.GetWidth();
     while(x>image.GetWidth())
       x -= image.GetWidth();
     while(y<0)
       y += image.GetHeight();
     while(y>image.GetHeight())
       y -= image.GetHeight();

     w = image.GetWidth() - x;
     if(w >= static_cast<int>(camera.GetWidth()))
       w = camera.GetWidth();

     h = image.GetHeight() - y;
     if(h >= static_cast<int>(camera.GetHeight()))
       h = camera.GetHeight();

     SDL_Rect ds = {x, y, w, h};
     SDL_Rect dr = {0,0, w, h};
     app.video.window.Blit( image, &ds, &dr);

     if(w < static_cast<int>(camera.GetWidth()))
     {
       SDL_Rect ds = {x+w-image.GetWidth(), y, (int)camera.GetWidth()-w, h};
       SDL_Rect dr = {w,0, (int)camera.GetWidth()-w, h};
       app.video.window.Blit( image, &ds, &dr);
     }
     if(h < static_cast<int>(camera.GetHeight()))
     {
       SDL_Rect ds = {x, y+h-image.GetHeight(), w, (int)camera.GetHeight()-h};
       SDL_Rect dr = {0,h, w, (int)camera.GetHeight()-h};
       app.video.window.Blit( image, &ds, &dr);
     }
     if(w < static_cast<int>(camera.GetWidth()) && h < static_cast<int>(camera.GetHeight()))
     {
       SDL_Rect ds = {x+w-image.GetWidth(), y+h-image.GetHeight(), camera.GetWidth()-w, camera.GetHeight()-h};
       SDL_Rect dr = {w,h, camera.GetWidth()-w, camera.GetHeight()-h};
       app.video.window.Blit( image, &ds, &dr);
     }
   }
}

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
     
  if (image.GetWidth() < int(camera.GetWidth())) {
    sky_cx = 0;
    margin_x = (camera.GetWidth()-image.GetWidth())/2;
  }
  if (image.GetHeight() < int(camera.GetHeight())) {
    sky_cy = 0;
    margin_y = (camera.GetHeight()-image.GetHeight())/2;
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
    app.video.window.Blit( image, &ds, &dr);
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
    app.video.window.Blit( image, &ds, &dr);
  }
#endif
}
