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

#ifndef DYNAMITE_H
#define DYNAMITE_H

#include "launcher.h"
#include "../graphic/sprite.h"
#include "../include/base.h"
#include "../character/character.h"

class Dynamite;

class BatonDynamite : public WeaponProjectile
{
  int channel;

public:
  BatonDynamite(ExplosiveWeaponConfig& cfg,
                WeaponLauncher * p_launcher);
  void Reset();
  void Refresh();
  void Shoot(double strength);

protected:
  void ShootSound();
  void SignalGhostState(bool already_dead);
};


// L'arme dynamite
class Dynamite : public WeaponLauncher
{
private:
  bool p_Shoot();
protected:
  WeaponProjectile * GetProjectileInstance();
public:
  Dynamite();
};
#endif
