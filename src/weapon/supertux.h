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
 * Weapon Supertux : Look ! it's the famous flying magic pinguin !
 *****************************************************************************/

#ifndef SUPERTUX_H
#define SUPERTUX_H

#include "launcher.h"
#include "particles/particle.h"

class TuxLauncher;

class SuperTuxWeaponConfig;

class SuperTux : public WeaponProjectile
{
  private:
    ParticleEngine particle_engine;
    double angle_rad;
    SoundSample flying_sound;

  public:
    uint speed;
    uint time_now;
    uint time_next_action;
    uint last_move;

    SuperTux(SuperTuxWeaponConfig& cfg,
             WeaponLauncher * p_launcher);
    void Refresh();

    inline void SetAngle(double angle) {angle_rad = angle;}
    void turn_left();
    void turn_right();
    void Shoot(double strength);
    virtual void Explosion();
  protected:
    void SignalOutOfMap();
};

class TuxLauncher : public WeaponLauncher
{
  private:
    SuperTux * current_tux;
  public:
    TuxLauncher();
    void EndOfTurn() const; // should be called only by SuperTux
    bool IsInUse() const;

    void SignalEndOfProjectile();
    virtual void HandleKeyPressed_MoveRight(bool shift);
    virtual void HandleKeyRefreshed_MoveRight(bool shift);
    virtual void HandleKeyReleased_MoveRight(bool shift);
    virtual void HandleKeyPressed_MoveLeft(bool shift);
    virtual void HandleKeyRefreshed_MoveLeft(bool shift);
    virtual void HandleKeyReleased_MoveLeft(bool shift);
    DECLARE_GETWEAPONSTRING();

  protected:
    WeaponProjectile * GetProjectileInstance();
    bool p_Shoot();
  private:
    SuperTuxWeaponConfig& cfg();
};

#endif
