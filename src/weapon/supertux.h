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
 * Arme Supertux : Look ! it's the famous flying magic pinguin !
 *****************************************************************************/

#ifndef SUPERTUX_H
#define SUPERTUX_H

#include "launcher.h"
#include "../graphic/surface.h"
#include "../gui/progress_bar.h"
#include "../include/base.h"
#include "../object/physical_obj.h"

class TuxLauncher;

class SuperTuxWeaponConfig : public ExplosiveWeaponConfig
{
public:
  uint speed;
  SuperTuxWeaponConfig();
  virtual void LoadXml(xmlpp::Element *elem);
};

class SuperTux : public WeaponProjectile
{
 private:
  ParticleEngine particle_engine;

public:
  double angle;
  uint speed;
  uint time_now;
  uint time_next_action;
  uint last_move;

  SuperTux(SuperTuxWeaponConfig& cfg,
           WeaponLauncher * p_launcher);
  void Refresh();

  void turn_left();
  void turn_right();
  void Shoot(double strength);
protected:
  void SignalOutOfMap();
};

class TuxLauncher : public WeaponLauncher
{
 private:
  SuperTux * current_tux;
 public:
  TuxLauncher();
  void HandleKeyEvent(Action::Action_t action, Clavier::Key_Event_t event_type);
 protected:
  WeaponProjectile * GetProjectileInstance();
  bool p_Shoot();
 private:
  SuperTuxWeaponConfig& cfg();
};

#endif
