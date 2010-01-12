/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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

#include "weapon/lowgrav.h"
#include "weapon/explosion.h"
#include "weapon/weapon_cfg.h"

#include "character/character.h"
#include "game/game.h"
#include "physic/physical_obj.h"
#include "sound/jukebox.h"
#include "interface/game_msg.h"
#include "team/teams_list.h"
#include "include/action_handler.h"

const double LOW_GRAVITY_FACTOR = 0.4;

LowGrav::LowGrav() :
  Weapon(WEAPON_LOWGRAV, "lowgrav", new WeaponConfig(), false),
  active(false)
{
  UpdateTranslationStrings();

  m_category = MOVE;

  use_unit_on_first_shoot = false;
}

void LowGrav::UpdateTranslationStrings()
{
  m_name = _("LowGrav");
  /* TODO: FILL IT */
  /* m_help = _(""); */
}

void LowGrav::p_Deselect()
{
  use.Stop();
  // TODO physic
  // ActiveCharacter().ResetConstants();
  ActiveCharacter().SetClothe("normal");
  ActiveCharacter().SetMovement("breathe");
  active = false;
}

bool LowGrav::p_Shoot()
{
  ActiveCharacter().GetPhysic()->SetGravityFactor(LOW_GRAVITY_FACTOR);
  ActiveCharacter().SetClothe("helmet");
  use.Play("default","weapon/lowgrav",-1);
  active = true;
  return true;
}

void LowGrav::StopShooting()
{
  if (active) {
    Deselect();
  } else {
    Weapon::StopShooting();
  }
}

std::string LowGrav::GetWeaponWinString(const char *TeamName, uint items_count ) const
{
  return Format(ngettext(
            "%s team has won %u lowgrav! Small step for man, giant leap for free software!",
            "%s team has won %u lowgravs! Small step for man, giant leap for free software!",
            items_count), TeamName, items_count);
}

