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
 * Submachine gun. Don't fire bullet one by one but with burst fire (like
 * a submachine gun :)
 * The hack in order to firing multiple bullet at once consist in using a 
 * std::list of projectile and overide the Refresh & HandleKeyEvent methods.
 *****************************************************************************/

#include <sstream>
#include "../map/map.h"
#include "../game/time.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../interface/game_msg.h"
#include "../interface/game_msg.h"
#include "../weapon/explosion.h"
#include "../weapon/submachine_gun.h"

const uint    SUBMACHINE_BULLET_SPEED       = 30;
const uint    SUBMACHINE_EXPLOSION_RANGE    = 15;
const double  SUBMACHINE_TIME_BETWEEN_SHOOT = 70;

SubMachineGunBullet::SubMachineGunBullet(ExplosiveWeaponConfig& cfg,
                                         WeaponLauncher * p_launcher) :
  WeaponBullet("m16_bullet", cfg, p_launcher)
{
  cfg.explosion_range = SUBMACHINE_EXPLOSION_RANGE;
}

void SubMachineGunBullet::ShootSound()
{
  jukebox.Play("share", "weapon/m16");
}

//-----------------------------------------------------------------------------

SubMachineGun::SubMachineGun() : WeaponLauncher(WEAPON_SUBMACHINE_GUN, "m16", new ExplosiveWeaponConfig())
{
  m_name = _("Submachine Gun");

  override_keys = true ;
  ignore_collision_signal = true;
  ignore_explosion_signal = true;
  ignore_ghost_state_signal = true;

  weapon_fire = new Sprite(resource_manager.LoadImage(weapons_res_profile,m_id+"_fire"));
  weapon_fire->EnableRotationCache(32);

  ReloadLauncher();
}

// Return a projectile instance for the submachine gun
WeaponProjectile * SubMachineGun::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new SubMachineGunBullet(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

bool SubMachineGun::p_Shoot ()
{  
  if (m_is_active)
    return false;
  
  ReloadLauncher();
  last_fire_time = Time::GetInstance()->Read();
  projectile->Shoot(SUBMACHINE_BULLET_SPEED);
  launcher_is_loaded = false;
  
  m_is_active = true;
  return true;
}

// Overide regular Refresh method
void SubMachineGun::RepeatShoot()
{
  if ( m_is_active )
  {
    uint tmp = Time::GetInstance()->Read();
    uint time = tmp - last_fire_time;

    if (time >= SUBMACHINE_TIME_BETWEEN_SHOOT)
    {
      m_is_active = false;
      NewActionShoot();
      last_fire_time = tmp;
    }
  }
}

// Special handle to allow multiple shoot at a time
void SubMachineGun::HandleKeyEvent(int action, int event_type)
{
  switch (action) {
    case ACTION_SHOOT:
      if (event_type == KEY_REFRESH)
        m_is_active = true;
      if (event_type == KEY_RELEASED)
        m_is_active = false;
      if (m_is_active) RepeatShoot();
      break;
    default:
      break;
  };
}
