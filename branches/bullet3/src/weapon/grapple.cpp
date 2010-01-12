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
 * Grapple
 *****************************************************************************/

#include "weapon/grapple.h"
#include "weapon/weapon_cfg.h"
#include "character/character.h"
#include "graphic/sprite.h"
#include "physic/physical_engine.h"
#include "physic/physical_shape.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

/*
+  #include <math.h>
+  #include "weapon/explosion.h"
+  #include "game/config.h"
+  #include "game/game.h"
+  #include "game/time.h"
+  #include "sound/jukebox.h"
+  #include "team/team.h"
+  #include "tool/math_tools.h"
+*/

class GrappleConfig : public EmptyWeaponConfig
{
 public:
  uint max_rope_length; // Max rope length in pixels
  uint automatic_growing_speed; // Pixel per 1/100 second.
  int push_force;

 public:
  GrappleConfig();
  void LoadXml(const xmlNode* elem);
};

//-----------------------------------------------------------------------------

Grapple::Grapple() :
  Weapon(WEAPON_GRAPPLE, "grapple", new GrappleConfig()),
  attached(false)
{
  UpdateTranslationStrings();

  m_category = MOVE;
  use_unit_on_first_shoot = false;
}

void Grapple::UpdateTranslationStrings()
{
  m_name = _("Grapple");
  /* TODO: FILL IT */
  /* m_help = _(""); */
}

Grapple::~Grapple()
{

}

bool Grapple::p_Shoot()
{
  // TODO physic ...
  return true;
}


void Grapple::Refresh()
{
  // TODO physic ...
}

void Grapple::Draw()
{
  Weapon::Draw();

  if (!attached) {
    return;
  }

  // TODO physic ...
}

bool Grapple::IsPreventingLRMovement()
{
  return attached;
}

bool Grapple::IsPreventingWeaponAngleChanges()
{
  return attached;
}

void Grapple::StartShooting()
{
  if (!attached)
    Weapon::StartShooting();
}

void Grapple::StopShooting()
{
  if (attached) {
    // DetachRope();
  } else {
    Weapon::StopShooting();
  }
}

std::string Grapple::GetWeaponWinString(const char *TeamName, uint items_count ) const
{
  return Format(ngettext(
            "%s team has won %u grapple!",
            "%s team has won %u grapples!",
            items_count), TeamName, items_count);
}

//-----------------------------------------------------------------------------

GrappleConfig& Grapple::cfg()
{
  return static_cast<GrappleConfig&>(*extra_params);
}
//-----------------------------------------------------------------------------

GrappleConfig::GrappleConfig()
{
  max_rope_length = 450;
  automatic_growing_speed = 12;
  push_force = 10;
}

void GrappleConfig::LoadXml(const xmlNode* elem)
{
  EmptyWeaponConfig::LoadXml(elem);
  XmlReader::ReadUint(elem, "max_rope_length", max_rope_length);
  XmlReader::ReadUint(elem, "automatic_growing_speed", automatic_growing_speed);
  XmlReader::ReadInt(elem, "push_force", push_force);
}
