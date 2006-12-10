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
 * Weapon Syringe
 *****************************************************************************/

#include "syringe.h"
#include "../game/game_loop.h"
#include "../team/macro.h"
#include "../tool/point.h"
#include "../tool/i18n.h"
#include "explosion.h"

Syringe::Syringe() : Weapon(WEAPON_SYRINGE, "syringe", new SyringeConfig())
{
  m_name = _("Syringe");
}

void Syringe::Draw() {
  Weapon::Draw();
}

bool Syringe::p_Shoot (){
  double angle = ActiveCharacter().GetFiringAngle();
  double radius = 0.0;
  bool end = false;

  jukebox.Play ("share","weapon/syringe");

  do
  {
    // Did we have finished the computation
    radius += 1.0;
    if (cfg().range < radius)
    {
      radius = cfg().range;
      end = true;
    }

    // Compute point coordinates
    Point2i relative_pos(static_cast<int>(radius * cos(angle)),
                         static_cast<int>(radius * sin(angle)) );
    Point2i pos_to_check = ActiveCharacter().GetHandPosition() + relative_pos;
    FOR_ALL_LIVING_CHARACTERS(team, character)
    if (&(*character) != &ActiveCharacter())
    {
      // Did we touch somebody ?
      if( character->ObjTouche(pos_to_check) )
      {
	// Apply damage (*ver).SetEnergyDelta (-cfg().damage);
	character->SetDiseaseDamage(cfg().damage, cfg().turns);
	end = true;
      }
    }
  } while (!end);

  return true;
}

void Syringe::Refresh(){
  if (m_is_active)
    m_is_active = false;
}

SyringeConfig& Syringe::cfg() {
  return static_cast<SyringeConfig&>(*extra_params);
}

SyringeConfig::SyringeConfig(){
  range =  45;
  turns = 10;
  damage = 10;
}

void SyringeConfig::LoadXml(xmlpp::Element *elem){
  WeaponConfig::LoadXml(elem);
  LitDocXml::LitUint (elem, "range", range);
  LitDocXml::LitUint (elem, "turns", turns);
  LitDocXml::LitUint (elem, "damage", damage);
}
