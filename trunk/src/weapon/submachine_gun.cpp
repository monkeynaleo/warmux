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

SubMachineGunBullet::SubMachineGunBullet(ExplosiveWeaponConfig& cfg) :
    WeaponBullet("m16_bullet", cfg)
{
  cfg.explosion_range = SUBMACHINE_EXPLOSION_RANGE;
}

void SubMachineGunBullet::ShootSound()
{
  jukebox.Play("share", "weapon/uzi"); // TODO: change for m16
}

void SubMachineGunBullet::SignalCollision()
{ 
  if ( GetLastCollidingObject() == NULL )
  {
    GameMessages::GetInstance()->Add (_("Your shot has missed!"));
  }
  is_active = false;
  lst_objects.RemoveObject(this);
  if (!IsGhost()) Explosion();
}

//-----------------------------------------------------------------------------

SubMachineGun::SubMachineGun() : WeaponLauncher(WEAPON_SUBMACHINE_GUN, "m16", new ExplosiveWeaponConfig())
{
  m_name = _("Submachine Gun");

  override_keys = true ;
  m_first_shoot = 0;
  is_loaded = false;
  Reload();
}

bool SubMachineGun::p_Shoot ()
{  
  if (m_is_active)
    return false;
  
  Reload();
  projectile->Shoot(SUBMACHINE_BULLET_SPEED);
  is_loaded = false;
  
  m_is_active = true;
  return true;
}

// Overide regular Refresh method
void SubMachineGun::RepeatShoot()
{
  if ( m_is_active )
  {
    uint tmp = Time::GetInstance()->Read();
    uint time = tmp - m_first_shoot;
    
    if (time >= SUBMACHINE_TIME_BETWEEN_SHOOT)
    {
      m_is_active = false;
      NewActionShoot();
      m_first_shoot = tmp;
    }
  }
}

// Load the machine gun with a projectile which is not in use
void SubMachineGun::Reload()
{
  if (is_loaded) return;
  projectile = new SubMachineGunBullet(cfg());
  projectile_list.push_back(dynamic_cast<WeaponBullet *> (projectile));
}

// Special handle to allow multiple shoot at a time
void SubMachineGun::HandleKeyEvent(int action, int event_type)
{
  switch (action) {
    case ACTION_SHOOT:
      if (event_type == KEY_REFRESH)
        RepeatShoot();
      if (event_type == KEY_RELEASED)
        m_is_active = false;
      break;
    default:
      break;
  };
}
