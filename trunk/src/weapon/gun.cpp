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
 * gun Weapon : The bullet made a great hole if we hit the ground or made damage
 * if we hit a character.
 *****************************************************************************/

#include "../weapon/gun.h"
#include <sstream>
#include "../map/map.h"
#include "../game/time.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../interface/game_msg.h"
#include "../interface/game_msg.h"
#include "../weapon/gun.h"
#include "../tool/math_tools.h"
#include "../weapon/explosion.h"

const uint GUN_BULLET_SPEED = 20;

GunBullet::GunBullet(ExplosiveWeaponConfig& cfg,
                     WeaponLauncher * p_launcher) :
  WeaponBullet("gun_bullet", cfg, p_launcher)
{
}

void GunBullet::ShootSound()
{
  jukebox.Play("share","weapon/gun");
}

//-----------------------------------------------------------------------------

Gun::Gun() : WeaponLauncher(WEAPON_GUN, "gun", new ExplosiveWeaponConfig())
{
  m_name = _("Gun");
  m_weapon_fire = new Sprite(resource_manager.LoadImage(weapons_res_profile,m_id+"_fire"));
  m_weapon_fire->EnableRotationCache(32);
  ReloadLauncher();
}

WeaponProjectile * Gun::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new GunBullet(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

bool Gun::p_Shoot()
{
  if (m_is_active)
    return false;  

  m_is_active = true;
  projectile->Shoot (GUN_BULLET_SPEED);
  projectile = NULL;
  ReloadLauncher();
  return true;
}

