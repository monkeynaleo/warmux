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
 * Snipe Rifle
 *****************************************************************************/

#ifndef __SNIPE_RIFLE_H
#define __SNIPE_RIFLE_H

#include <vector>
#include "launcher.h"
#include "../include/base.h"

class SnipeBullet : public WeaponBullet
{
  public:
    SnipeBullet(ExplosiveWeaponConfig& cfg); 
  private:
    void ShootSound();
};

class SnipeRifle : public WeaponLauncher
{
  ParticleEngine particle;
  double last_angle;
  Point2i last_bullet_pos;
  Point2i last_rifle_pos;
  Point2i * cross_point;
  bool targeting_something;
  Sprite * m_laser_image;
  private:
    bool p_Shoot();

  protected:
    uint m_first_shoot;

  public:
    SnipeRifle();
    void Draw();  // In order to draw the laser beam.
    Point2i * GetCrossPoint();
    bool ComputeCrossPoint();
    bool isTargetingSomething();
};

#endif /* __SNIPE_RIFLE_H */
