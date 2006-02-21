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
 * Arme Supertux : and now the flying magic pinguin !
 *****************************************************************************/

#include "supertux.h"
#include "weapon_tools.h"
#include "../game/config.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../graphic/video.h"
#include "../interface/game_msg.h"
#include "../interface/interface.h"
#include "../map/camera.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/math_tools.h"
#include "../tool/i18n.h"
const uint time_delta = 40;
const uint animation_deltat = 50;

SuperTux::SuperTux(GameLoop &p_game_loop, SuperTuxWeaponConfig& cfg) :
  WeaponProjectile (p_game_loop, "supertux", cfg), 
  particle_engine(particle_STAR,40)
{
  m_gravity_factor = 0.0;
}

void SuperTux::Refresh()
{
  if (!is_active) return;

  if (TestImpact()) { SignalCollision(); return; }

  image->SetRotation_deg((angle+M_PI_2)*180.0/M_PI);
  if ((last_move+animation_deltat)<global_time.Read())
    {
      SetExternForce(static_cast<SuperTuxWeaponConfig&>(cfg).speed, angle);
      image->Update();
      last_move = global_time.Read();
  }

  particle_engine.AddPeriodic(GetPosition(), angle, 0);
}


void SuperTux::turn_left()
{  
  time_now = global_time.Read();
  if (time_next_action<time_now)
    {
      time_next_action=time_now + time_delta;
      angle = angle - 15.0/180.0*M_PI;
    }
}

void SuperTux::turn_right()
{
  time_now = global_time.Read();
  if (time_next_action<time_now)
    {
      time_next_action=time_now + time_delta;
      angle = angle + 15.0/180.0*M_PI;
    }
}

void SuperTux::SignalCollision()
{ 

  particle_engine.Stop();
  
  if (IsGhost())
  {
    game_messages.Add (_("Bye bye tux..."));
  }
  is_active = false; 
}

void SuperTux::Draw()
{ 
  particle_engine.Draw();

  // supertux must be upper the particles
  WeaponProjectile::Draw(); 
}

//-----------------------------------------------------------------------------

SuperTuxWeaponConfig::SuperTuxWeaponConfig()
{
}

void SuperTuxWeaponConfig::LoadXml(xmlpp::Element *elem) 
{
  WeaponConfig::LoadXml (elem);
  LitDocXml::LitUint (elem, "speed", speed);
}

//-----------------------------------------------------------------------------

TuxLauncher::TuxLauncher() : 
  WeaponLauncher(WEAPON_SUPERTUX, "tux", new SuperTuxWeaponConfig(), VISIBLE_ONLY_WHEN_INACTIVE)
{ 
  m_name = _("SuperTux");   
  override_keys = true ;

  projectile = new SuperTux(game_loop, cfg());
}

void TuxLauncher::HandleKeyEvent(int action, int event_type)
{
  switch (action) {
  case ACTION_MOVE_LEFT:
    if (event_type != KEY_RELEASED)
      static_cast<SuperTux *>(projectile)->turn_left();
    break ;
    
  case ACTION_MOVE_RIGHT:
    if (event_type != KEY_RELEASED)
      static_cast<SuperTux *>(projectile)->turn_right();
    break ;
    
  default:
    break ;
  } ;
}

SuperTuxWeaponConfig& TuxLauncher::cfg() 
{ return static_cast<SuperTuxWeaponConfig&>(*extra_params); }
