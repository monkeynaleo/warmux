/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
#include <iostream>
#include <SDL_video.h>
#include <SDL_gfxPrimitives.h>
#include <limits.h>
#include "camera.h"
#include "map.h"
#include "maps_list.h"
#include "../graphic/surface.h"
#include "../graphic/video.h"
#include "../include/app.h"
#include "../include/constant.h"
#include "../tool/i18n.h"
#include "../tool/resource_manager.h"

Ground::Ground()
{ //FIXME (a effacer)
}

//calculated with a good old TI-83... more or less precision might be wanted...
float Ground::table[5][5] = {
    {.78539,  .46364,M_PI,  -.46364+M_PI,  -.78539+M_PI},
    {1.1071,  .78539,M_PI,  -.78539+M_PI,  -1.1071+M_PI},
    {M_PI / 2.0,M_PI / 2.0,M_PI / 2.0,M_PI / 2.0,M_PI / 2.0},
    {-1.1071+M_PI,  -.78539+M_PI,  M_PI,  78539,  1.1071},
    {-.78539+M_PI,  -.46364+M_PI,  M_PI,  .46364,  .78539}};

void Ground::Init(){
  std::cout << "o " << _("Ground initialization...") << ' ';
  std::cout.flush();

  // Load ground data
  Surface m_image = ActiveMap().ReadImgGround();
  LoadImage ( m_image );

  // V�ifie la taille du terrain
  assert(Constants::MAP_MIN_SIZE <= GetSize());
  assert(GetSizeX()*GetSizeY() <= Constants::MAP_MAX_SIZE);

  // V�ifie si c'est un terrain ouvert ou ferm�
  ouvert = ActiveMap().IsOpened();

  std::cout << _("done") << std::endl;
}

void Ground::Reset(){
  Init();
  lastPos.SetValues(INT_MAX, INT_MAX);
}

// Lit la valeur alpha du pixel (x,y)
bool Ground::IsEmpty(const Point2i &pos) const{
	assert( !world.EstHorsMondeXY(pos.x, pos.y) );

	// Lit le monde
	return GetAlpha( pos ) != 255; // IsTransparent
}

/*
 * Returns the angle between the tangent at point (x,y) of the ground and
 * horizontal
 * the angle is always > 0.
 * returns -1.0 if no tangent was found (pixel (x,y) does not touch any
 * other piece of ground
 */
double Ground::Tangeante(int x,int y){
  //Approxiamtion : on renvoie la corde de la courbe form�
  //par le terrain...

  /* We try to find 2 points on the ground on each side of (x,y)
   * the points should be at the limit between land and vaccum
   * (p1 =  point on the left
   * p2 =  point on the right
   */
  Point2i p1,p2;
  if(!PointContigu(x,y, p1.x,p1.y, -1,-1))
    return NAN;

  if(!PointContigu(x,y, p2.x,p2.y, p1.x,p1.y))
  {
    p2.x = x;
    p2.y = y;
  }
/*
  if(p1.x == p2.x)
    return M_PI / 2.0;
  if(p1.y == p2.y)
    return M_PI;
*/
  //assert (p1.x != p2.x);

  /* double tangeante = atan((double)(p2.y-p1.y)/(double)(p2.x-p1.x));

  while(tangeante <= 0.0)
    tangeante += M_PI;
  while(tangeante > 2 * M_PI)
    tangeante -= M_PI;

  return tangeante; */
  return table[p2.y-p1.y][p2.x-p1.x];
}

bool Ground::PointContigu(int x,int y,  int & p_x,int & p_y,
                           int pas_bon_x,int pas_bon_y)
{
  //Cherche un pixel autour du pixel(x,y) qui est �la limite entre
  //le terrin et le vide.
  //renvoie true (+ p_x et p_y) si on a trouv�qqch, sinon false
  if(world.EstHorsMonde(Point2i(x-1,y))
  || world.EstHorsMonde(Point2i(x+1,y))
  || world.EstHorsMonde(Point2i(x,y-1))
  || world.EstHorsMonde(Point2i(x,y+1)) )
    return false;

  //regarde en haut �gauche
  if(x-1 != pas_bon_x
  || y-1 != pas_bon_y)
  if( !IsEmpty(Point2i(x-1,y-1) )
  &&( IsEmpty(Point2i(x-1,y))
  || IsEmpty(Point2i(x,y-1))))
  {
    p_x=x-1;
    p_y=y-1;
    return true;
  }
  //regarde en haut
  if(x != pas_bon_x
  || y-1 != pas_bon_y)
  if(!IsEmpty(Point2i(x,y-1))
  &&(IsEmpty(Point2i(x-1,y-1))
  || IsEmpty(Point2i(x+1,y-1))))
  {
    p_x=x;
    p_y=y-1;
    return true;
  }
  //regarde en haut �droite
  if(x+1 != pas_bon_x
  || y-1 != pas_bon_y)
  if(!IsEmpty(Point2i(x+1,y-1))
  &&(IsEmpty(Point2i(x,y-1))
  || IsEmpty(Point2i(x+1,y))))
  {
    p_x=x+1;
    p_y=y-1;
    return true;
  }
  //regarde �droite
  if(x+1 != pas_bon_x
  || y != pas_bon_y)
  if(!IsEmpty(Point2i(x+1,y))
  &&(IsEmpty(Point2i(x+1,y-1))
  || IsEmpty(Point2i(x,y+1))))
  {
    p_x=x+1;
    p_y=y;
    return true;
  }
  //regarde en bas �droite
  if(x+1 != pas_bon_x
  || y+1 != pas_bon_y)
  if(!IsEmpty(Point2i(x+1,y+1))
  &&(IsEmpty(Point2i(x+1,y))
  || IsEmpty(Point2i(x,y+1))))
  {
    p_x=x+1;
    p_y=y+1;
    return true;
  }
  //regarde en bas
  if(x != pas_bon_x
  || y+1 != pas_bon_y)
  if(!IsEmpty(Point2i(x,y+1))
  &&(IsEmpty(Point2i(x-1,y+1))
  || IsEmpty(Point2i(x+1,y+1))))
  {
    p_x=x;
    p_y=y+1;
    return true;
  }
  //regarde en bas �gauche
  if(x-1 != pas_bon_x
  || y+1 != pas_bon_y)
  if(!IsEmpty(Point2i(x-1,y+1))
  &&(IsEmpty(Point2i(x-1,y))
  || IsEmpty(Point2i(x,y+1))))
  {
    p_x=x-1;
    p_y=y+1;
    return true;
  }
  //regarde �gauche
  if(x-1 == pas_bon_x
  && y == pas_bon_y)
  if(!IsEmpty(Point2i(x-1,y))
  &&(IsEmpty(Point2i(x-1,y-1))
  || IsEmpty(Point2i(x-1,y+1))))
  {
    p_x=x-1;
    p_y=y;
    return true;
  }
  return false;
}

void Ground::Draw()
{
  CheckEmptyTiles();
  AppWormux * app = AppWormux::GetInstance();

  Point2i cPos = camera.GetPosition();
  Point2i windowSize = app->video.window.GetSize();
  Point2i margin = (windowSize - GetSize())/2;

  if( camera.HasFixedX() ){// ground is less wide than screen !
    app->video.window.BoxColor( Rectanglei(0, 0, margin.x, windowSize.y), black_color);
    app->video.window.BoxColor( Rectanglei(windowSize.x - margin.x, 0, margin.x, windowSize.y), black_color);
  }

  if( camera.HasFixedY() ){// ground is less wide than screen !
    app->video.window.BoxColor( Rectanglei(0, 0, windowSize.x, margin.y), black_color);
    app->video.window.BoxColor( Rectanglei(0, windowSize.y - margin.y, windowSize.x, margin.y), black_color);
  }

  if( lastPos != cPos ){
    lastPos = cPos;
    DrawTile();
    return;
  }

  RedrawParticleList(*world.to_redraw_now);

  // Draw on top of sky (redisplayed on top of particles)
  RedrawParticleList(*world.to_redraw_particles_now);

  // Draw on top of new position of particles (redisplayed on top of particles)
  RedrawParticleList(*world.to_redraw_particles);
}

void Ground::RedrawParticleList(std::list<Rectanglei> &list){
	std::list<Rectanglei>::iterator it;

	for( it = list.begin(); it != list.end(); ++it )
		DrawTile_Clipped(*it);
}
