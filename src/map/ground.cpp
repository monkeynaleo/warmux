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
 * Terrain de jeu.
 *****************************************************************************/

#include "ground.h"
//-----------------------------------------------------------------------------
#include <iostream>
#include "map.h"
#include "maps_list.h"
#include "../map/camera.h"

#include "../include/constant.h"
#include "../graphic/graphism.h"
#ifdef CL
#include "../weapon/mine.h"
#else
#include "../tool/resource_manager.h"
#include <SDL/SDL_video.h>
#endif

#ifdef DEBUG
//#  define DESSINE_BORDURE_CANVAS
#endif

//-----------------------------------------------------------------------------
namespace Wormux {

//-----------------------------------------------------------------------------

Terrain::Terrain()
{ //FIXME (a effacer) 
}

//-----------------------------------------------------------------------------

void Terrain::Init()
{
  std::cout << "Ground initialization...";
  std::cout.flush();
  
  // Charge les données du terrain
#ifdef CL
  CL_Surface *m_image = new CL_Surface(lst_terrain.TerrainActif().LitImgTerrain());
  LoadImage (*m_image);
  delete m_image;
#else
  SDL_Surface *m_image = lst_terrain.TerrainActif().LitImgTerrain();
  LoadImage ( m_image);
//delete m_image;
#endif

  // Vérifie la taille du terrain
  assert (LARG_MIN_TERRAIN <= GetWidth());
  assert (HAUT_MIN_TERRAIN <= GetHeight());
  assert ((ulong)GetWidth()*GetHeight() <= TAILLE_MAX_TERRAIN);
  
  // Vérifie si c'est un terrain ouvert ou fermé
  ouvert = lst_terrain.TerrainActif().is_opened;

  std::cout << "done" << std::endl;
}

//-----------------------------------------------------------------------------

void Terrain::Reset()
{
  Init();
}

//-----------------------------------------------------------------------------

// Lit la valeur alpha du pixel (x,y)
bool Terrain::EstDansVide (int x, int y)
{ 
  // En dehors du monde : c'est vide :-p
  //  if (monde.EstHorsMondeXY(x,y)) return config.exterieur_monde_vide;
  assert (!monde.EstHorsMondeXY(x,y));
  if(TerrainActif().infinite_bg)
  {
    if(x < 0 || y<0 || x>static_cast<int>(GetWidth()) || y>static_cast<int>(GetHeight()))
      return true;
  }

  // Lit le monde
#ifdef CL
   return EstTransparent( GetAlpha(x,y) );
#else
   return ( GetAlpha(x,y) != 255);
//   return ( GetAlpha(x,y) == SDL_ALPHA_TRANSPARENT);
#endif
}

//-----------------------------------------------------------------------------
//Renvoie l'angle entre la tangeante au terrain en (x,y) et l'horizontale.
//l'angle est toujours > 0.
//Renvoie -1.0 s'il n'y a pas de tangeante (si le pixel(x,y) ne touche
//aucun autre morceau de terrain)
double Terrain::Tangeante(int x,int y)
{
  //Approxiamtion:on renvoie la corde de la courbe formée
  //par le terrain...

  //On cherche deux points du terrain autour de (x,y), à la limite entre le terrain
  //et le vide:
  //(p1 = 1er point à gauche
  // p2 = 1er point à droite
  // p3 = 2em point à gauche
  // p4 = 2em point à droite)
#ifdef CL
  CL_Point p1,p2,p3,p4;
#else
  Point2i p1,p2,p3,p4;
#endif
  if(!PointContigu(x,y, p1.x,p1.y, -1,-1))
    return -1.0;
  
  if(!PointContigu(x,y, p2.x,p2.y, p1.x,p1.y))
  {
    p2.x = x;
    p2.y = y;
  }

  if(!PointContigu(p1.x,p1.y, p3.x,p3.y, x,y))
  {
    p3.x = p1.x;
    p3.y = p1.y;
  }
  if(!PointContigu(p2.x,p2.y, p4.x,p4.y, x,y))
  {
    p4.x = p2.x;
    p4.y = p2.y;
  }

  if(p3.x == p4.x)
    return M_PI / 2.0;
  if(p3.y == p4.y)
    return M_PI;

  assert (p3.x != p4.x);
  
  double tangeante = atan((double)(p4.y-p3.y)/(double)(p4.x-p3.x));

  while(tangeante <= 0.0)
    tangeante += M_PI;
  while(tangeante > M_PI)
    tangeante -= M_PI;

  return tangeante;
}

//-----------------------------------------------------------------------------
bool Terrain::PointContigu(int x,int y,  int & p_x,int & p_y,
                           int pas_bon_x,int pas_bon_y)
{
  //Cherche un pixel autour du pixel(x,y) qui est à la limite entre
  //le terrin et le vide.
  //renvoie true (+ p_x et p_y) si on a trouvé qqch, sinon false
#ifdef CL
  if(monde.EstHorsMonde(CL_Point(x-1,y))
  || monde.EstHorsMonde(CL_Point(x+1,y))
  || monde.EstHorsMonde(CL_Point(x,y-1))
  || monde.EstHorsMonde(CL_Point(x,y+1)) )
    return false;
#else
  if(monde.EstHorsMonde(Point2i(x-1,y))
  || monde.EstHorsMonde(Point2i(x+1,y))
  || monde.EstHorsMonde(Point2i(x,y-1))
  || monde.EstHorsMonde(Point2i(x,y+1)) )
    return false;
#endif
   
  //regarde en haut à gauche
  if(x-1 != pas_bon_x
  || y-1 != pas_bon_y)
  if(!EstDansVide(x-1,y-1)
  &&(EstDansVide(x-1,y)
  || EstDansVide(x,y-1)))
  {
    p_x=x-1;
    p_y=y-1;
    return true;
  }
  //regarde en haut
  if(x != pas_bon_x
  || y-1 != pas_bon_y)
  if(!EstDansVide(x,y-1)
  &&(EstDansVide(x-1,y-1)
  || EstDansVide(x+1,y-1)))
  {
    p_x=x;
    p_y=y-1;
    return true;
  }
  //regarde en haut à droite
  if(x+1 != pas_bon_x
  || y-1 != pas_bon_y)
  if(!EstDansVide(x+1,y-1)
  &&(EstDansVide(x,y-1)
  || EstDansVide(x+1,y)))
  {
    p_x=x+1;
    p_y=y-1;
    return true;
  }
  //regarde à droite
  if(x+1 != pas_bon_x
  || y != pas_bon_y)
  if(!EstDansVide(x+1,y)
  &&(EstDansVide(x+1,y-1)
  || EstDansVide(x,y+1)))
  {
    p_x=x+1;
    p_y=y;
    return true;
  }
  //regarde en bas à droite
  if(x+1 != pas_bon_x
  || y+1 != pas_bon_y)
  if(!EstDansVide(x+1,y+1)
  &&(EstDansVide(x+1,y)
  || EstDansVide(x,y+1)))
  {
    p_x=x+1;
    p_y=y+1;
    return true;
  }
  //regarde en bas
  if(x != pas_bon_x
  || y+1 != pas_bon_y)
  if(!EstDansVide(x,y+1)
  &&(EstDansVide(x-1,y+1)
  || EstDansVide(x+1,y+1)))
  {
    p_x=x;
    p_y=y+1;
    return true;
  }
  //regarde en bas à gauche
  if(x-1 != pas_bon_x
  || y+1 != pas_bon_y)
  if(!EstDansVide(x-1,y+1)
  &&(EstDansVide(x-1,y)
  || EstDansVide(x,y+1)))
  {
    p_x=x-1;
    p_y=y+1;
    return true;
  }
  //regarde à gauche
  if(x-1 == pas_bon_x
  && y == pas_bon_y)
  if(!EstDansVide(x-1,y)
  &&(EstDansVide(x-1,y-1)
  || EstDansVide(x-1,y+1)))
  {
    p_x=x-1;
    p_y=y;
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

void Terrain::Draw()
{
#if 0
  int cx = camera.GetX();
  int cy = camera.GetY();
  static int lastx = camera.GetX()-1;
  static int lasty = camera.GetY()-1;  
  if (lastx == cx && lasty == cy) return;
  lastx = cx;
  lasty = cy;
#endif  
  DrawTile();
}

//-----------------------------------------------------------------------------
} // namespace Wormux
