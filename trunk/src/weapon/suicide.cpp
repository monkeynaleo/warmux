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
 * Suicide.
 *****************************************************************************/

#include "suicide.h"
//-----------------------------------------------------------------------------
#include <iostream>
#include "weapon_tools.h"
#include "../game/game_loop.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
//-----------------------------------------------------------------------------

// Espace entre l'espace en l'image
const uint ESPACE = 5;

//-----------------------------------------------------------------------------

Suicide::Suicide() : Weapon(WEAPON_SUICIDE, "suicide")
{  
  m_name = _("Commit suicide");
  extra_params = new ExplosiveWeaponConfig();
  sound_channel = -1;  
  hole_image = resource_manager.LoadImage( weapons_res_profile, "suicide_hole"); 
}

//-----------------------------------------------------------------------------

void Suicide::p_Select()
{
  is_dying = false;
}

//-----------------------------------------------------------------------------

bool Suicide::p_Shoot()
{ 
  sound_channel = jukebox.Play ("share", "weapon/suicide");

  game_loop.interaction_enabled=false;
  is_dying = true;

  return true;
}

//-----------------------------------------------------------------------------

void Suicide::Refresh()
{
  if (!is_dying) return;

  if ( sound_channel != -1 && Mix_Playing(sound_channel) ) {
    m_is_active = true;
  } else {
    m_is_active = false;
  }

  if (!m_is_active) {
    if ( !ActiveCharacter().IsDead() ) ActiveCharacter().Die();
  }
}

//-----------------------------------------------------------------------------

ExplosiveWeaponConfig& Suicide::cfg()
{ return static_cast<ExplosiveWeaponConfig&>(*extra_params); }

//-----------------------------------------------------------------------------
