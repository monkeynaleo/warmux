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
 * WeaponLauncher: generic weapon to launch a projectile
 *****************************************************************************/

#ifndef WEAPON_LAUNCHER_H
#define WEAPON_LAUNCHER_H
#include "weapon.h"
#include "../graphic/surface.h"
#include "../gui/progress_bar.h"
#include "../include/base.h"
#include "../object/physical_obj.h"

class WeaponLauncher;

class WeaponProjectile : public PhysicalObj
{
  public:
    bool is_active;

  protected:
    Sprite *image;

    bool explode_colliding_character; // before timeout.
    double begin_time;
  
    ExplosiveWeaponConfig& cfg;

  public:
    Character* dernier_ver_touche;
    PhysicalObj* dernier_obj_touche;
    WeaponLauncher * launcher;
    int m_timeout_modifier ;

  public:
    WeaponProjectile(const std::string &nom, 
                     ExplosiveWeaponConfig& cfg,
                     WeaponLauncher * p_launcher);
    virtual ~WeaponProjectile();

    virtual void Draw();
    virtual void Refresh();
    virtual void Shoot(double strength);
    virtual void Explosion();

    void IncrementTimeOut();
    void DecrementTimeOut();
    void SetTimeOut(int timeout);
    int GetTotalTimeout();
    void ResetTimeOut();
    bool change_timeout_allowed();
  
  protected:
    virtual void SignalCollision() = 0;
    bool TestImpact ();
    virtual void ShootSound();
  private:
    void SignalGhostState (bool was_dead);
    void SignalFallEnding();
    void SignalCollisionObject();
};

class WeaponBullet : public WeaponProjectile
{
  public:
    WeaponBullet(const std::string &name,
                 ExplosiveWeaponConfig& cfg,
                 WeaponLauncher * p_launcher);
    virtual ~WeaponBullet(){};
    virtual void Refresh();
  protected:
    void SignalCollision();
    void Explosion(); 
};


class WeaponLauncher : public Weapon
{
  protected:
    WeaponProjectile * projectile;
    uint nb_active_projectile;
  protected:
    virtual bool p_Shoot();
    virtual void p_Select();
    virtual void p_Deselect();
    bool m_allow_change_timeout;
  private:
    void DirectExplosion();
  
  public:
    WeaponLauncher(Weapon_type type, 
                   const std::string &id,
                   EmptyWeaponConfig * params,
                   weapon_visibility_t visibility = ALWAYS_VISIBLE);
    virtual ~WeaponLauncher();

    void Refresh();
    virtual void Draw();
    void HandleKeyEvent(int action, int event_type);
 
  // Handle of projectile events
    void SignalProjectileCollision();
    void IncActiveProjectile();
    void DecActiveProjectile();
  
  //Misc actions
    void ActionUp ();//called by mousse.cpp when mousewhellup
    void ActionDown ();//called by mousse.cpp when mousewhelldown

    WeaponProjectile* GetProjectile() { return projectile; };
    ExplosiveWeaponConfig& cfg();
};

#endif
