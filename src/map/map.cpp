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
#include "../graphic/graphism.h"
//#include "../weapon/air_attack.h"
#include "../game/config.h"
#include "../include/constant.h"
#include "../object/bonus_box.h"
#include "../tool/i18n.h"
#include "../game/time.h"
#include "wind.h"
#include "camera.h"
#include "maps_list.h"
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

//NOT USED ANYMORE
/*void Monde::Init()
{

  ciel.Init();
  water.Init();

  cout.flush();
  terrain.Init();

  // Distance minimale entre les vers
  dst_min_entre_vers = DST_MIN_ENTRE_VERS;
}*/

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

#ifdef CL
void Monde::Creuse (uint x, uint y, CL_Surface &surface)
{
  terrain.Dig (x, y, 
	       surface.get_width(), surface.get_height(),
               surface.get_pixeldata());
}
#else
void Monde::Creuse (uint x, uint y, SDL_Surface *surface)
{
   terrain.Dig (x, y, surface);
}
#endif

//-----------------------------------------------------------------------------

void Monde::DrawSky()
{ ciel.Draw(); }

//-----------------------------------------------------------------------------

void Monde::DrawWater()
{ water.Draw(); }

//-----------------------------------------------------------------------------

void Monde::Draw()
{ 
  wind.DrawParticles();
  terrain.Draw(); 
}

//-----------------------------------------------------------------------------

bool Monde::EstHorsMondeX(int x) const
{ return ((x < 0) || ((int)GetWidth() <= x)); }

bool Monde::EstHorsMondeY(int y) const
{ return ((y < 0) || ((int)GetHeight() <= y)); }

bool Monde::EstHorsMondeXlarg(int x, uint larg) const
{ return ((x+(int)larg-1 < 0) || ((int)GetWidth() <= x)); }

bool Monde::EstHorsMondeYhaut(int y, uint haut) const
{ return ((y+(int)haut-1 < 0) || ((int)GetHeight() <= y)); }

bool Monde::EstHorsMondeXY(int x, int y) const
{ return EstHorsMondeX(x) || EstHorsMondeY(y); }

#ifdef CL
bool Monde::EstHorsMonde (const CL_Point &pos) const
{ return EstHorsMondeXY (pos.x, pos.y); }
#else
bool Monde::EstHorsMonde (const Point2i &pos) const
{ return EstHorsMondeXY (pos.x, pos.y); }
#endif
//-----------------------------------------------------------------------------

bool Monde::EstDansVide(int x, int y)
{ return terrain.EstDansVide (x,y); }

//-----------------------------------------------------------------------------

#ifdef CL
bool Monde::LigneH_EstDansVide (int left, int y, int right)
{ 
  assert (left <= right);

  // Vérifie qu'on reste dans le monde
  if (EstHorsMondeXlarg(left, right-left+1) || EstHorsMondeY(y)) 
    return config.exterieur_monde_vide;
  if (left < 0) left = 0;
  if ((int)GetWidth() <= right) right = GetWidth()-1;

  // Traite une ligne
  for (uint i=(uint)left; i<=(uint)right; i++) 
  {
    if (!EstDansVide(i, (uint)y)) return false;
  }
  return true;
}
#else
bool Monde::LigneH_EstDansVide (int ox, int y, int width)
{ 
  // Traite une ligne
  for (unsigned int i=0; i<width; i++) 
     {
	if (!EstDansVide(ox+i, (uint)y)) 
	  {
	     return false;
	  }
     }
   
   return true;
}
#endif

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

#ifdef CL
bool Monde::RectEstDansVide (const CL_Rect &prect)
{
  CL_Rect rect(prect);

  // Vérifie qu'on reste dans le monde
  if (EstHorsMondeYhaut(rect.top, rect.bottom-rect.top+1))
    return config.exterieur_monde_vide;
  if (rect.top < 0) rect.top = 0;
  if ((int)GetHeight() <= rect.bottom) rect.bottom = GetHeight()-1;

  // Teste un pixel après l'autre
  for (int i=rect.top; i <= rect.bottom; ++i)
  {
    if (!LigneH_EstDansVide (rect.left, i, rect.right)) return false;
  }
  return true;
}
#else
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
#endif

//-----------------------------------------------------------------------------

#ifdef CL
bool Monde::EstDansVide_haut (const PhysicalObj &obj, int dx, int dy)
{
  return LigneH_EstDansVide (obj.GetTestRect().left+dx,
			     obj.GetTestRect().top+dy,
			     obj.GetTestRect().right+dx);
}
#else
bool Monde::EstDansVide_haut (const PhysicalObj &obj, int dx, int dy)
{
  return LigneH_EstDansVide (obj.GetTestRect().x+dx,
			     obj.GetTestRect().y+obj.GetTestRect().h+dy,
			     obj.GetTestRect().w);
}
#endif
//-----------------------------------------------------------------------------

#ifdef CL
bool Monde::EstDansVide_bas (const PhysicalObj &obj, int dx, int dy)
{
  return LigneH_EstDansVide (obj.GetTestRect().left+dx,
			     obj.GetTestRect().bottom+dy,
			     obj.GetTestRect().right+dx);
}
#else
bool Monde::EstDansVide_bas (const PhysicalObj &obj, int dx, int dy)
{
  return LigneH_EstDansVide (obj.GetTestRect().x+dx,
			     obj.GetTestRect().y+dy,
			     obj.GetTestRect().w);
}
#endif
//-----------------------------------------------------------------------------

void Monde::DrawAuthorName()
{
  uint time = temps.Lit();

  if (AUTHOR_INFO_TIME < time) return;

  std::string txt;
  txt  = Format(_("Map %s, a creation of :"),
		lst_terrain.TerrainActif().name.c_str());
  txt += '\n';
  txt += lst_terrain.TerrainActif().author_info;

#ifdef CL
   police_mix.WriteLeft(AUTHOR_INFO_X,AUTHOR_INFO_Y, txt);
#else
//TODO
#endif
}

//-----------------------------------------------------------------------------
