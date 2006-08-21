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

#ifndef SUBMACHINE_GUN_H
#define SUBMACHINE_GUN_H

#include <SDL.h>
#include <vector>
#include "launcher.h"
#include "../include/base.h"
#include "../tool/point.h"

class SubMachineGunBullet : public WeaponBullet
{
  public:
    SubMachineGunBullet(ExplosiveWeaponConfig& cfg,
                        WeaponLauncher * p_launcher);
  private:
    void ShootSound();
};

class SubMachineGun : public WeaponLauncher
{
  private:
    typedef std::list<WeaponBullet *> ProjectileList;
    typedef ProjectileList::iterator ProjectileIterator;
    ProjectileList projectile_list;
    ProjectileIterator projectile_iterator;
    uint m_first_shoot;
    bool is_loaded;
  private:
    void Reload();
    void RepeatShoot();
  public:
    SubMachineGun();
    bool p_Shoot();
    void HandleKeyEvent(int action, int event_type);
    void SignalProjectileCollision();
};

#endif /* SUBMACHINE_GUN_H */
