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
 * Monde ou plateau de jeu.
 *****************************************************************************/

#include "map.h"
//-----------------------------------------------------------------------------
#include <iostream>
#include "../game/config.h"
#include "../include/constant.h"
#include "../object/bonus_box.h"
#include "../tool/i18n.h"
#include "../game/time.h"
#include "wind.h"
#include "camera.h"
#include "maps_list.h"
#include "../tool/stats.h"

using namespace std;
using namespace Wormux;
//-----------------------------------------------------------------------------
const double DST_MIN_ENTRE_VERS = 50.0;

const uint AUTHOR_INFO_TIME = 5000; // ms
const uint AUTHOR_INFO_X = 30;
const uint AUTHOR_INFO_Y = 50;
//-----------------------------------------------------------------------------
Monde monde;
//-----------------------------------------------------------------------------

Monde::Monde()
{
  dst_min_entre_vers = DST_MIN_ENTRE_VERS;
}

//NOT USED ANYMORE
// void Monde::Init()
// {

//   ciel.Init();
//   water.Init();

//   cout.flush();
//   terrain.Init();

//   // Distance minimale entre les vers
//   dst_min_entre_vers = DST_MIN_ENTRE_VERS;
// }

//-----------------------------------------------------------------------------

void Monde::Reset()
{
  ciel.Reset();
  terrain.Reset();
  water.Reset();
  wind.Reset();

  // Configure le jeu selon que le terrain soit ouvert ou non
  bool ouvert = terrain.EstOuvert();
  caisse.Active (ouvert);
  config.exterieur_monde_vide = ouvert;

  delete author_info1; author_info1 = NULL;
  delete author_info2; author_info2 = NULL;
}

//-----------------------------------------------------------------------------

void Monde::Refresh()
{
  water.Refresh();
  wind.Refresh();
}

//-----------------------------------------------------------------------------

void Monde::FreeMem() 
{ 
  terrain.Free(); 
  ciel.Free(); 
  water.Free();
}

//-----------------------------------------------------------------------------


void Monde::Creuse (uint x, uint y, SDL_Surface *surface)
{
   terrain.Dig (x, y, surface);
}

//-----------------------------------------------------------------------------

void Monde::DrawSky()
{ ciel.Draw(); }

//-----------------------------------------------------------------------------

void Monde::DrawWater()
{ water.Draw(); }

//-----------------------------------------------------------------------------

void Monde::Draw()
{ 
  //StatStart("GameDraw: wind_particles");
  wind.DrawParticles();
  //StatStop("GameDraw: wind_particles");

  //StatStart("GameDraw: ground");
  terrain.Draw(); 
  //StatStop("GameDraw: ground");
}

//-----------------------------------------------------------------------------

bool Monde::EstHorsMondeX(int x) const
{ return ((x < 0) || ((int)GetWidth() <= x)) && !TerrainActif().infinite_bg; }

bool Monde::EstHorsMondeY(int y) const
{ return (((y < 0) && !TerrainActif().infinite_bg) || ((int)GetHeight() <= y)); }

bool Monde::EstHorsMondeXlarg(int x, uint larg) const
{ return ((x+(int)larg-1 < 0) || ((int)GetWidth() <= x)) && !TerrainActif().infinite_bg; }

bool Monde::EstHorsMondeYhaut(int y, uint haut) const
{ return ((y+(int)haut-1 < 0  && !TerrainActif().infinite_bg) || ((int)GetHeight() <= y)); }

bool Monde::EstHorsMondeXY(int x, int y) const
{ return EstHorsMondeX(x) || EstHorsMondeY(y) && !TerrainActif().infinite_bg; }

bool Monde::EstHorsMonde (const Point2i &pos) const
{ return EstHorsMondeXY (pos.x, pos.y); }

//-----------------------------------------------------------------------------

bool Monde::EstDansVide(int x, int y)
{ return terrain.EstDansVide (x,y); }

//-----------------------------------------------------------------------------

bool Monde::LigneH_EstDansVide (int ox, int y, int width)
{ 
  // Traite une ligne
  for (int i=0; i<width; i++) 
     {
	if (!EstDansVide(ox+i, (uint)y)) 
	  {
	     return false;
	  }
     }
   
   return true;
}

//-----------------------------------------------------------------------------

bool Monde::LigneV_EstDansVide (int x, int top, int bottom)
{ 
  assert (top <= bottom);

  // Vérifie qu'on reste dans le monde
  if (EstHorsMondeX(x) || EstHorsMondeYhaut(top, bottom-top+1))
    return config.exterieur_monde_vide;
  if (top < 0) top = 0;
  if ((int)GetHeight() <= bottom) bottom = GetHeight()-1;

  // Traite une ligne
  for (uint iy=(uint)top; iy<=(uint)bottom; iy++) 
  {
    if (!EstDansVide((uint)x, iy)) return false;
  }
  return true;
}

//-----------------------------------------------------------------------------

bool Monde::RectEstDansVide (const Rectanglei &prect)
{
   Rectanglei rect(prect);

   // Clip rectangle in the the world area
   rect.Clip( Rectanglei(0,0,GetWidth(),GetHeight())); 
   
   // Check line by line
   for (int i=rect.y; i < rect.y+rect.h; i++)
     {
	if (!LigneH_EstDansVide (rect.x, i, rect.w))
	  {
	     return false;
	  }
     }
   
   return true;
}

//-----------------------------------------------------------------------------


bool Monde::EstDansVide_haut (const PhysicalObj &obj, int dx, int dy)
{
  return LigneH_EstDansVide (obj.GetTestRect().x+dx,
			     obj.GetTestRect().y+obj.GetTestRect().h+dy,
			     obj.GetTestRect().w);
}

//-----------------------------------------------------------------------------


bool Monde::EstDansVide_bas (const PhysicalObj &obj, int dx, int dy)
{
  return LigneH_EstDansVide (obj.GetTestRect().x+dx,
			     obj.GetTestRect().y+dy,
			     obj.GetTestRect().w);
}

//-----------------------------------------------------------------------------

void Monde::DrawAuthorName()
{
  if (AUTHOR_INFO_TIME < global_time.Read()) {
    if (author_info1 != NULL) {
      delete author_info1;
      delete author_info2;
      author_info1 = author_info2 = NULL;
    }
    return;
  }

  if (author_info1 == NULL) {
    std::string txt;
    txt  = Format(_("Map %s, a creation of :"),
		  lst_terrain.TerrainActif().name.c_str());
    author_info1 = new Text(txt, white_color, &small_font);
    txt = lst_terrain.TerrainActif().author_info;
    author_info2 = new Text(txt, white_color, &small_font);
  }
  
  author_info1->DrawTopLeft(AUTHOR_INFO_X,AUTHOR_INFO_Y);
  author_info2->DrawTopLeft(AUTHOR_INFO_X,AUTHOR_INFO_Y+small_font.GetHeight());
}

//-----------------------------------------------------------------------------
