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
  protected:
    Sprite *image;

    bool explode_colliding_character; // before timeout.
    bool explode_with_timeout;
    bool explode_with_collision;
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

    void IncrementTimeOut();
    void DecrementTimeOut();
    void SetTimeOut(int timeout);
    int GetTotalTimeout();
    void ResetTimeOut();
    bool change_timeout_allowed();
  
  protected:
    virtual void SignalObjectCollision(PhysicalObj * obj);
    virtual void SignalGroundCollision();
    virtual void SignalCollision();
    virtual void SignalOutOfMap();

    virtual void SignalTimeout();
    virtual void SignalExplosion();
    void SignalGhostState (bool was_dead);

    virtual void ShootSound();
    virtual void Explosion();
    virtual void RandomizeShoot(double &angle,double &strength);
    virtual void DoExplosion();
    void RemoveFromPhysicalEngine();
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
    virtual void SignalGroundCollision();
    virtual void SignalObjectCollision(PhysicalObj * obj);
    void DoExplosion();
};


class WeaponLauncher : public Weapon
{
  public:
    bool ignore_timeout_signal;
    bool ignore_collision_signal;
    bool ignore_explosion_signal;
    bool ignore_ghost_state_signal;
  protected:
    WeaponProjectile * projectile;
    uint nb_active_projectile;
    bool m_allow_change_timeout;
    bool launcher_is_loaded;
    typedef std::list<WeaponProjectile *> ProjectileList;
    ProjectileList projectile_list;
  protected:
    virtual bool p_Shoot();
    virtual void p_Select();
    virtual void p_Deselect();
    virtual WeaponProjectile * GetProjectileInstance() = 0;
    virtual bool ReloadLauncher();
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
    virtual void SignalProjectileExplosion();
    virtual void SignalProjectileCollision();
    virtual void SignalProjectileGhostState(WeaponProjectile * proj);
    virtual void SignalProjectileTimeout();

    virtual Point2i GetGunHolePosition();
    void IncActiveProjectile();
    void DecActiveProjectile();

  //Misc actions
    void ActionUp ();//called by mousse.cpp when mousewhellup
    void ActionDown ();//called by mousse.cpp when mousewhelldown

    WeaponProjectile* GetProjectile() { return projectile; };
    ExplosiveWeaponConfig& cfg();
};

#endif
