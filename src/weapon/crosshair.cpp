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
 * Weapon's crosshair
 *****************************************************************************/

#include "crosshair.h"
#include "weapon.h"
#include "../game/game_loop.h"
#include "../graphic/surface.h"
#include "../include/app.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../tool/math_tools.h"

// Distance between crosshair and character
#define RAYON 40 // pixels

CrossHair::CrossHair()
{
  enable = false;
  angle_rad = 0;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  image = resource_manager.LoadImage(res, "gfx/pointeur1");
  resource_manager.UnLoadXMLProfile( res);
}

void CrossHair::Reset()
{
  ChangeAngleVal (M_PI_4);
}

void CrossHair::ChangeAngle (double delta)
{
  ChangeAngleVal (angle_rad + delta);
}

void CrossHair::ChangeAngleVal (double val)
{
  while(angle_rad > 2*M_PI)
    angle_rad -= 2 * M_PI;
  while(angle_rad <= -2*M_PI)
    angle_rad += 2 * M_PI;

  angle_rad = BorneDouble(val, -(ActiveTeam().GetWeapon().GetMaxAngle()),
                    -(ActiveTeam().GetWeapon().GetMinAngle()) );

}

void CrossHair::Draw()
{
  Point2i calcul_d;
  if( !enable )
    return;
  if( ActiveCharacter().IsDead() )
    return;
  if( GameLoop::GetInstance()->ReadState() != GameLoop::PLAYING )
    return;
  // Compute crosshair position
  calcul_d = Point2i(RAYON, RAYON) * Point2d(cos(angle_rad), sin(angle_rad));
  Point2i pos = ActiveCharacter().GetHandPosition();
  pos += calcul_d * Point2i(ActiveCharacter().GetDirection(), 1);
  pos -= image.GetSize()/2;

  AppWormux::GetInstance()->video.window.Blit(image, pos - camera.GetPosition());
  world.ToRedrawOnMap(Rectanglei(pos, image.GetSize()));
}
/*
int CrossHair::GetAngle() const
{
  if (ActiveCharacter().GetDirection() == -1)
    return int( InverseAngleDeg (angle) );
  else
    return angle;
}

*/
double CrossHair::GetAngleVal() const
{ return angle_rad; }
double CrossHair::GetAngleRad() const
{

  if (ActiveCharacter().GetDirection() == -1)
    return InverseAngle (angle_rad);
  return angle_rad;
}
