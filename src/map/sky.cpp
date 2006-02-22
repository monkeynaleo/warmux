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
const Point2d SKY_SPEED( 0.3, 1);

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
  lastPos.SetValues(INT_MAX, INT_MAX);
}

void Sky::CompleteDraw(){
    Point2i pos = camera.GetPosition() * SKY_SPEED;

	if(!TerrainActif().infinite_bg){
		Point2i tstVect = image.GetSize().inf( camera.GetSize() );
		Point2i margin = tstVect * (camera.GetSize() - image.GetSize())/2;
		pos =  pos * (Point2i(1, 1) - tstVect);

		Rectanglei ds(pos, camera.GetSize());
		app.video.window.Blit( image, ds, margin);
   }else{
     Point2i size;

     while( pos.x<0 )
       pos.x += image.GetWidth();
     while( pos.x>image.GetWidth() )
       pos.x -= image.GetWidth();
     while( pos.y<0 )
       pos.y += image.GetHeight();
     while(pos.y>image.GetHeight())
       pos.y -= image.GetHeight();

	 size = image.GetSize() - pos;
     if(size.x >= camera.GetSizeX())
       size.x = camera.GetSizeX();

     if(size.y >= camera.GetSizeY())
       size.y = camera.GetSizeY();

     Rectanglei ds(pos, size);
     app.video.window.Blit( image, ds, Point2i(0, 0));

     if(size.x < camera.GetSizeX())
     {
       Rectanglei ds(pos.x + size.x - image.GetWidth(), pos.y, camera.GetSizeX() - size.x, size.y);
       Point2i dp(size.x, 0);
       app.video.window.Blit( image, ds, dp);
     }
     if(size.y < camera.GetSizeY())
     {
       Rectanglei ds(pos.x, pos.y + size.y - image.GetHeight(), size.x, camera.GetSizeY() - size.y);
       Point2i dp(0, size.y);
       app.video.window.Blit( image, ds, dp);
     }
     if(size.x < camera.GetSizeX() && size.y < camera.GetSizeY())
     {
       Rectanglei ds(pos + size - image.GetSize(), camera.GetSize() - size);
       app.video.window.Blit( image, ds, size);
     }
   }
}

void Sky::Draw()
{
#if defined(WIN32)
  // TODO: Why the cache doesn't work on Windows!?
  CompleteDraw();
#else  
  Point2i cPos = camera.GetPosition();
  
  if( lastPos != cPos){
    CompleteDraw();
    lastPos = cPos;
    return;
  }

  Point2i tstVect = image.GetSize().inf( camera.GetSize() );
  Point2i margin = tstVect * (camera.GetSize() - image.GetSize())/2;
  Point2i skyPos = camera.GetPosition() * SKY_SPEED * (Point2i(1, 1) - tstVect);

  std::list<Rectanglei>::iterator it;
  for (it = world.to_redraw_now->begin(); 
       it != world.to_redraw_now->end(); 
       ++it){
    Rectanglei ds(skyPos + it->GetPosition() - cPos - margin, 
		    it->GetSize() + 1);
    Point2i dp(it->GetPosition() - cPos);
    app.video.window.Blit( image, ds, dp);
  }

  for (it = world.to_redraw_particles_now->begin(); 
       it != world.to_redraw_particles_now->end(); 
       ++it){
    Rectanglei ds(skyPos + it->GetPosition() - cPos - margin, 
		    it->GetSize() + 1);
    Point2i dp(it->GetPosition() - cPos);
    app.video.window.Blit( image, ds, dp);
  }
#endif
}
