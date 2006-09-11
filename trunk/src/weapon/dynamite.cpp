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
 * Weapon dynamite : When fired, explode after a short laps of time. Then make a
 * big hole, eject character and made them lost energy.
 * Like a dynamite after all :)
 *****************************************************************************/

#include "dynamite.h"
#include "explosion.h"
#include "../game/config.h"
#include "../include/app.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../tool/resource_manager.h"
#include "../tool/debug.h"

#ifdef __MINGW32__
#undef LoadImage
#endif

BatonDynamite::BatonDynamite(ExplosiveWeaponConfig& cfg,
                             WeaponLauncher * p_launcher) :
  WeaponProjectile("dynamite_bullet", cfg, p_launcher)
{
  channel = -1;
  explode_with_collision = false;

  image->animation.SetLoopMode(false);
  SetSize(image->GetSize());
  SetCollisionModel(false,false,false);
  SetTestRect (0, 0, 2, 3);
}

void BatonDynamite::Shoot(double strength)
{
  Reset();
  WeaponProjectile::Shoot(strength);
}

void BatonDynamite::Reset()
{
  StopMoving();
  unsigned int delay = (1000 * WeaponProjectile::GetTotalTimeout())/image->GetFrameCount();
  image->SetFrameSpeed(delay);

  image->Scale(ActiveCharacter().GetDirection(), 1);
  image->SetCurrentFrame(0);
  image->Start();
}

void BatonDynamite::Refresh()
{
  image->Update();
  if (image->IsFinished()) Explosion();
}

void BatonDynamite::ShootSound()
{
  channel = jukebox.Play("share","weapon/dynamite_fuze", -1);
}

void BatonDynamite::SignalGhostState(bool already_dead)
{
  jukebox.Stop(channel);
  channel = -1;
  launcher->SignalProjectileGhostState();
}

//-----------------------------------------------------------------------------

Dynamite::Dynamite() :
    WeaponLauncher(WEAPON_DYNAMITE, "dynamite", new ExplosiveWeaponConfig(), VISIBLE_ONLY_WHEN_INACTIVE)
{
  m_name = _("Dynamite");
  ReloadLauncher();
}

WeaponProjectile * Dynamite::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new BatonDynamite(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

// drop a dynamite
bool Dynamite::p_Shoot ()
{
  dynamic_cast<BatonDynamite *>(projectile)->Reset();
  projectile->Shoot(0);

  // add the character speed
  if(ActiveCharacter().GetDirection() == 1)
    projectile->SetSpeed(3.0, -M_PI_4);
  else
    projectile->SetSpeed(3.0, -3.0 * M_PI_4);

  return true;
}
