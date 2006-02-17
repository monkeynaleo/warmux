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
 * Uzi !
 *****************************************************************************/

#include "uzi.h"
//-----------------------------------------------------------------------------
#include <sstream>
#include "weapon_tools.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../interface/game_msg.h"
#include "../map/map.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
//-----------------------------------------------------------------------------

const double SOUFFLE_BALLE = 1;
const double MIN_TIME_BETWEEN_SHOOT = 70; // in milliseconds

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BalleUzi::BalleUzi(GameLoop &p_game_loop, WeaponLauncher * p_launcher) :
  WeaponProjectile(p_game_loop, "uzi_bullet", p_launcher)
{ 
  touche_ver_objet = true; 
}

//-----------------------------------------------------------------------------

void BalleUzi::SignalCollision()
{   
  if ((LitDernierVerTouche() == NULL) && (LitDernierObjTouche() == NULL))
  {
    game_messages.Add (_("Your shot has missed!"));
  }
  is_active = false; 
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Uzi::Uzi() :
  WeaponLauncher(WEAPON_UZI,"uzi", new WeaponConfig())
{
  m_name = _("Uzi");
  override_keys = true ;

  m_first_shoot = 0;

  projectile = new BalleUzi(game_loop, this);
}

//-----------------------------------------------------------------------------

void Uzi::p_Deselect()
{
  m_is_active = false;
}

//-----------------------------------------------------------------------------

void Uzi::RepeatShoot()
{        
  uint time = global_time.Read() - m_first_shoot; 
  uint tmp = global_time.Read();

  if (time >= MIN_TIME_BETWEEN_SHOOT)
  {
    m_is_active = false;
    NewActionShoot();
    m_first_shoot = tmp;
  }
}

//-----------------------------------------------------------------------------

bool Uzi::p_Shoot()
{
  jukebox.Play("share", "weapon/uzi");

  // Calculate movement of the bullet
  
  // Set the initial position.
  int x,y;
  ActiveCharacter().GetHandPosition(x, y);

  // Equation of movement : y = ax + b
  double angle,a,b ;
  angle = ActiveTeam().crosshair.GetAngleRad();
  a=sin(angle)/cos(angle);
  b= y-(a*x) ;

  // Move the bullet !!
  //balle.PrepareTir();  
  projectile->SetXY (x,y);

  while (projectile->is_active) {
    y = int(double((a*x) + b)) ;

    projectile->SetXY(x,y);

    // the bullet in gone outside the map
    if (projectile->IsGhost()) {
      projectile->is_active=false;
      return true;
    }
    
    // is there a collision ??
    if (projectile->CollisionTest(0,0)) 
    {
      projectile->is_active=false;

      // Si la balle a touché un ver, lui inflige des dégats
      Character* ver = projectile->LitDernierVerTouche();
      PhysicalObj* obj = projectile->LitDernierObjTouche();
      if (ver) obj = ver;
      if (ver)
      {
        ver -> SetEnergyDelta (-cfg().damage);
      }
      if (obj) 
      {
	obj -> AddSpeed (SOUFFLE_BALLE, angle);
      }

      // Creuse le world
      if (!obj)
      {
	world.Creuse (projectile->GetX() - projectile->impact.GetWidth()/2,
		      projectile->GetY() - projectile->impact.GetHeight()/2,
		      projectile->impact);
      }
      return true;
    }
    x+=ActiveCharacter().GetDirection();
  }

  return true;
}

//-----------------------------------------------------------------------------

void Uzi::HandleKeyEvent(int action, int event_type)
{

  switch (action) {    

  case ACTION_SHOOT:
    
    if (event_type == KEY_REFRESH)
      RepeatShoot();
    
    if (event_type == KEY_RELEASED)
      m_is_active = false;

    break ;
    
  default:
    break ;
  } ;
}

//-----------------------------------------------------------------------------
