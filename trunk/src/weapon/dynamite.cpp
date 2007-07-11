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
 * Weapon dynamite : When fired, explode after a short laps of time. Then make a
 * big hole, eject character and made them lost energy.
 * Like a dynamite after all :)
 *****************************************************************************/

#include "dynamite.h"
#include "explosion.h"
#include "character/character.h"
#include "game/config.h"
#include "graphic/sprite.h"
#include "include/app.h"
#include "object/objects_list.h"
#include "team/teams_list.h"
#include "tool/i18n.h"
#include "tool/resource_manager.h"
#include "tool/debug.h"

#ifdef __MINGW32__
#undef LoadImage
#endif

DynamiteStick::DynamiteStick(ExplosiveWeaponConfig& cfg,
                             WeaponLauncher * p_launcher) :
  WeaponProjectile("dynamite_bullet", cfg, p_launcher)
{
  explode_with_collision = false;

  image->animation.SetLoopMode(false);
  SetSize(image->GetSize());
  SetTestRect (0, 0, 2, 3);
}

void DynamiteStick::Shoot(double strength)
{
  unsigned int delay = (1000 * WeaponProjectile::GetTotalTimeout())/image->GetFrameCount();
  image->SetFrameSpeed(delay);

  image->Scale(ActiveCharacter().GetDirection(), 1);
  image->SetCurrentFrame(0);
  image->Start();
  WeaponProjectile::Shoot(strength);
}

void DynamiteStick::Refresh()
{
  image->Update();
  if (image->IsFinished()) Explosion();
}

void DynamiteStick::ShootSound()
{
  timeout_sound.Play("share","weapon/dynamite_fuze", -1);
}

void DynamiteStick::SignalExplosion()
{
  timeout_sound.Stop();
}

void DynamiteStick::SignalOutOfMap()
{
  timeout_sound.Stop();
}

void DynamiteStick::SignalDrowning()
{
  timeout_sound.Stop();
}
//-----------------------------------------------------------------------------

Dynamite::Dynamite() :
    WeaponLauncher(WEAPON_DYNAMITE, "dynamite", new ExplosiveWeaponConfig(), VISIBLE_ONLY_WHEN_INACTIVE)
{
  m_name = _("Dynamite");
  m_category = THROW;
  ReloadLauncher();
}

WeaponProjectile * Dynamite::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new DynamiteStick(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

// drop a dynamite
bool Dynamite::p_Shoot ()
{
  projectile->Shoot(0);
  // add the character speed
  if(ActiveCharacter().GetDirection() == 1)
    projectile->SetSpeed(3.0, -M_PI_4);
  else
    projectile->SetSpeed(3.0, -3.0 * M_PI_4);

  projectile = NULL;
  ReloadLauncher();
  return true;
}

std::string Dynamite::GetWeaponWinString(const char *TeamName, uint items_count )
{
  return Format(ngettext(
            "%s team has won %u dynamite!",
            "%s team has won %u dynamites!",
            items_count), TeamName, items_count);
}

