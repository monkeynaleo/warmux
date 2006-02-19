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
 * Weapon's crosshair
 *****************************************************************************/

#include "crosshair.h"
#include <SDL.h>
#include <iostream>
#include "weapon.h"
#include "../game/game_loop.h"
#include "../graphic/surface.h"
#include "../include/app.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../tool/math_tools.h"


// Distance entre le pointeur et le ver
#define RAYON 40 // pixels

#define HAUT_POINTEUR 11
#define LARG_POINTEUR 11

CrossHair::CrossHair()
{
  enable = false;
  angle = 0;
}

void CrossHair::Reset()
{
  ChangeAngleVal (45);
}

void CrossHair::ChangeAngle (int delta)
{
  ChangeAngleVal (angle+delta);
}

void CrossHair::ChangeAngleVal (int val)
{
  angle = BorneLong(val, - (ActiveTeam().GetWeapon().max_angle),
		    - (ActiveTeam().GetWeapon().min_angle) );
  
  const double angleRAD = Deg2Rad(angle);

  // Calcul des coordonnées du point
  calcul_dx = (int)(RAYON*cos( angleRAD ));
  calcul_dy = (int)(RAYON*sin( angleRAD ));
}

void CrossHair::Draw()
{
  if( !enable )
	return;
  if( ActiveCharacter().IsDead() )
	return;
  if( game_loop.ReadState() != gamePLAYING )
	return;

  Point2i pos = ActiveCharacter().GetHandPosition();
  
  pos.x += calcul_dx*ActiveCharacter().GetDirection();
  pos.y += calcul_dy;
 
  pos = pos - image.GetSize()/2;
  app.video.window.Blit(image, pos - camera.GetPosition());

  world.ToRedrawOnMap(Rectanglei(pos, image.GetSize()));
}

int CrossHair::GetAngle() const
{ 
	if (ActiveCharacter().GetDirection() == -1) 
		return int( InverseAngleDeg (angle) );
	else
		return angle; 
}

int CrossHair::GetAngleVal() const
{ return angle; }

double CrossHair::GetAngleRad() const
{
  double angleR = Deg2Rad(angle);

  if (ActiveCharacter().GetDirection() == -1) angleR = InverseAngle (angleR);
  return angleR;
}

void CrossHair::Init()
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  image = resource_manager.LoadImage(res, "gfx/pointeur1");
  resource_manager.UnLoadXMLProfile( res); 
}

