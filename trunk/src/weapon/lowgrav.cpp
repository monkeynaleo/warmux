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
 * Low gravity weapon
 *****************************************************************************/

#include "lowgrav.h"
#include "explosion.h"
#include "../game/game.h"
#include "../game/game_loop.h"
#include "../object/physical_obj.h"
#include "../sound/jukebox.h"
#include "../interface/game_msg.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../include/action_handler.h"

const double LOW_GRAVITY_FACTOR = 0.4;

LowGrav::LowGrav() : Weapon(WEAPON_LOWGRAV, "lowgrav",
			    new WeaponConfig(), NEVER_VISIBLE)
{
  m_name = _("LowGrav");

  override_keys = true ;
  use_unit_on_first_shoot = false;
}

void LowGrav::Refresh()
{
  ActiveCharacter().UpdatePosition();
}

void LowGrav::p_Deselect()
{
  ActiveCharacter().ResetConstants();
  ActiveCharacter().SetClothe("normal");
  m_is_active = false;
}

bool LowGrav::p_Shoot()
{
  ActiveCharacter().SetGravityFactor(LOW_GRAVITY_FACTOR);
  ActiveCharacter().SetClothe("helmet");
  return true;
}

void LowGrav::Draw()
{
}

void LowGrav::HandleKeyEvent(Action::Action_t action, Clavier::Key_Event_t event_type)
{
  switch (action)
    {
      case Action::ACTION_SHOOT:
        if (event_type == Clavier::KEY_PRESSED)
          ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_WEAPON_STOP_USE));
	break ;

      default:
	ActiveCharacter().HandleKeyEvent(action, event_type);
	break ;
    }
}

void LowGrav::SignalTurnEnd()
{
  p_Deselect();
}

void LowGrav::ActionStopUse()
{
  UseAmmoUnit();
}
