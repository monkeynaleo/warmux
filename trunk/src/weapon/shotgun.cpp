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
 * Shotgun. Shoot a bunch of buckshot at each fire
 *****************************************************************************/

#include <sstream>
#include "../map/map.h"
#include "../game/time.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../interface/game_msg.h"
#include "../network/randomsync.h"
#include "explosion.h"
#include "shotgun.h"

const uint   SHOTGUN_BUCKSHOT_SPEED  = 20;
const uint   SHOTGUN_EXPLOSION_RANGE = 1;
const double SHOTGUN_RANDOM_ANGLE    = 0.2;
const double SHOTGUN_RANDOM_STRENGTH = 2.0;

ShotgunBuckshot::ShotgunBuckshot(ExplosiveWeaponConfig& cfg,
                                 WeaponLauncher * p_launcher) :
  WeaponBullet("buckshot", cfg, p_launcher)
{
  cfg.explosion_range = SHOTGUN_EXPLOSION_RANGE;
}

void ShotgunBuckshot::RandomizeShoot(double &angle,double &strength)
{
  angle *= 1.0 + randomSync.GetDouble(-SHOTGUN_RANDOM_ANGLE,SHOTGUN_RANDOM_ANGLE);
  strength += randomSync.GetDouble(-SHOTGUN_RANDOM_STRENGTH,SHOTGUN_RANDOM_STRENGTH);
}

bool ShotgunBuckshot::IsOverlapping(PhysicalObj* obj)
{
  if(typeid(*obj) == typeid(ShotgunBuckshot)) return true;
  return m_overlapping_object == obj;
}

//-----------------------------------------------------------------------------

Shotgun::Shotgun() : WeaponLauncher(WEAPON_SHOTGUN, "shotgun", new ExplosiveWeaponConfig())
{
  m_name = _("Shotgun");

  override_keys = true ;
  m_weapon_fire = new Sprite(resource_manager.LoadImage(weapons_res_profile,m_id+"_fire"));
  m_weapon_fire->EnableRotationCache(32);

  ReloadLauncher();
}

// Return a buckshot instance for the shotgun
WeaponProjectile * Shotgun::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new ShotgunBuckshot(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

void Shotgun::ShootSound()
{
  jukebox.Play("share", "weapon/gun");
}

bool Shotgun::p_Shoot ()
{  
  if (m_is_active)
    return false;
  
  for(int i = 0; i < 4 ; i++) {
    projectile->Shoot(SHOTGUN_BUCKSHOT_SPEED);
    projectile = NULL;
    ReloadLauncher();
  }
  ShootSound();
  m_is_active = true;
  return true;
}
