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
 * Blowtorch - burns holes into walls
 *****************************************************************************/

#include "weapon/blowtorch.h"
#include "weapon/explosion.h"
#include "weapon/weapon_cfg.h"

#include "character/character.h"
#include "character/body.h"
#include "map/map.h"
#include "game/game_mode.h"
#include "game/time.h"
#include "graphic/sprite.h"
#include "sound/jukebox.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

static const uint MIN_TIME_BETWEEN_DIG = 200;        // milliseconds

class BlowtorchConfig : public WeaponConfig
{
  public:
    BlowtorchConfig();
    virtual void LoadXml(const xmlNode* elem);

    uint range;
};

Blowtorch::Blowtorch() : Weapon(WEAPON_BLOWTORCH, "blowtorch", new BlowtorchConfig())
{
  UpdateTranslationStrings();

  m_category = TOOL;
  m_time_between_each_shot = MIN_TIME_BETWEEN_DIG;
  m_weapon_fire = new Sprite(GetResourceManager().LoadImage(weapons_res_profile, "blowtorch_fire"));
}

void Blowtorch::UpdateTranslationStrings()
{
  m_name = _("Blowtorch");
  m_help = _("How to use it : keep the space key pressed\nAngle : Up/Down\nan ammo per turn");
}

void Blowtorch::p_Deselect()
{
  ActiveTeam().AccessNbUnits() = 0;
}

bool Blowtorch::p_Shoot()
{
  Point2i hole = ActiveCharacter().GetPhysic()->GetPosition();

  double angle = ActiveCharacter().GetFiringAngle();
  uint h = cfg().range;
  double dx = cos(angle) * h;
  double dy = sin(angle) * h;

  Point2i pos = Point2i(hole.x+(int)dx, hole.y+(int)dy);
  double size = ActiveCharacter().GetSize().Norm()/2;
  GetWorld().Dig(pos, size);
  JukeBox::GetInstance()->Play("default", "weapon/blowtorch");

  return true;
}

void Blowtorch::StartShooting()
{
  m_is_active = true;
}


void Blowtorch::StopShooting()
{
  m_is_active = false;
  SignalTurnEnd();
}

void Blowtorch::Refresh()
{
  if (EnoughAmmoUnit()) {
    Weapon::RepeatShoot();
  }
}

//-------------------------------------------------------------------------------------

BlowtorchConfig::BlowtorchConfig()
{
  range = 2;
}

BlowtorchConfig& Blowtorch::cfg()
{
  return static_cast<BlowtorchConfig&>(*extra_params);
}

void BlowtorchConfig::LoadXml(const xmlNode* elem)
{
  WeaponConfig::LoadXml(elem);
  XmlReader::ReadUint(elem, "range", range);
}

std::string Blowtorch::GetWeaponWinString(const char *TeamName, uint items_count) const
{
  return Format(ngettext(
            "%s team has won %u blowtorch! If you're under 18, ask your parents to use it.",
            "%s team has won %u blowtorchs! If you're under 18, ask your parents to use it.",
            items_count), TeamName, items_count);
}
