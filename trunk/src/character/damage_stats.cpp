/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Damage statistics for a character
 *****************************************************************************/

#include "damage_stats.h"
#include "../character/character.h"
#include "../team/team.h"
#include "../tool/debug.h"

DamageStatistics::DamageStatistics(const Character& _owner) :
  owner(_owner),
  damage_other_team(0),
  damage_own_team(0),
  max_damage(0),
  current_total_damage(0)
{}

DamageStatistics::DamageStatistics(const DamageStatistics& adamage_stats, const Character& _owner) :
  owner(_owner),
  damage_other_team(adamage_stats.damage_other_team),
  damage_own_team(adamage_stats.damage_own_team),
  max_damage(adamage_stats.max_damage),
  current_total_damage(adamage_stats.current_total_damage)
{}

void DamageStatistics::HandleMostDamage()
{
  if (current_total_damage > max_damage)
  {
    max_damage = current_total_damage;
  }
  //MSG_DEBUG("damage", "%s most damage: %d\n", m_name, max_damage);
  current_total_damage = 0;
}


void DamageStatistics::MadeDamage(const int Dmg, const Character &other)
{
  if (owner.GetTeam().IsSameAs(other.GetTeam()))
  {
    //MSG_DEBUG("damage", "%s damaged own team with %d\n", m_name, Dmg);

    if (owner.IsSameAs(other))
      damage_own_team += Dmg;
  }
  else
  {
    //MSG_DEBUG("damage", "%s damaged other team with %d\n", m_name, Dmg);
    damage_other_team += Dmg;
  }

  current_total_damage += Dmg;
}
