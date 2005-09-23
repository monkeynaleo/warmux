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
#include "../graphic/graphism.h"
#include "maps_list.h"
#ifndef CL
#include <SDL.h>
#include "../include/app.h"
#endif
//-----------------------------------------------------------------------------
namespace Wormux
{

// Vitesse (comprise entre -0.5 et 0)
const double VITESSE_CIEL_X = -0.3;
const double VITESSE_CIEL_Y = -1;

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

#ifdef CL
void Ciel::Draw()
{
  int x=camera.GetX();
  if (0 <= x)
    x = (int)(x*VITESSE_CIEL_X);
  else
    x = 0;
  int y=camera.GetY();
  if (0 <= y)
    y = (int)(y*VITESSE_CIEL_Y);
  else
    y = 0;
  do
  {
    CL_Display::push_translate(x, y);
    DrawTile();
    CL_Display::pop_modelview();
    x += GetWidth();
  } while (x <= (int)monde.GetWidth());
}
#else
void Ciel::Draw()
{   	
   SDL_Rect ds = {-camera.GetX()*VITESSE_CIEL_X, -camera.GetY()*VITESSE_CIEL_Y,app.sdlwindow->w,app.sdlwindow->h};
   SDL_Rect dr = {0,0,app.sdlwindow->w,app.sdlwindow->h};
   
   SDL_BlitSurface( image, &ds, app.sdlwindow, &dr);
}
   
#endif
   
   
//-----------------------------------------------------------------------------
} // namespace Wormux
