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
 * gun Weapon : The bullet made a great hole if we hit the ground or made damage
 * if we hit a character.
 *****************************************************************************/

#ifndef GUN_H
#define GUN_H

#include <SDL.h>
#include <vector>
#include "launcher.h"
#include "../include/base.h"
#include "../tool/point.h"

class GunBullet : public WeaponBullet
{
  public:
    GunBullet(ExplosiveWeaponConfig& cfg,
              WeaponLauncher * p_launcher);
  protected:
    void ShootSound();
};

class Gun : public WeaponLauncher
{
  public:
    Gun();
  protected:
    bool p_Shoot();
    WeaponProjectile * GetProjectileInstance();
};

#endif /* GUN_H */
