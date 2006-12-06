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
 * Artificial intelligence Shoot module
 *****************************************************************************/

#ifndef AI_SHOOT_MODULE_H
#define AI_SHOOT_MODULE_H

#include "../character/character.h"

class AIShootModule
{
 private:
  typedef enum {
    NO_STRATEGY,
    NEAR_FROM_ENEMY,
    SHOOT_FROM_POINT
  } strategy_t;

  uint m_current_time;

  Character* m_enemy;
  strategy_t m_current_strategy;
  bool m_has_finished;

  double m_angle;
  uint m_last_shoot_time;

  // for shooting weapons like gun, shotgun, sniper rifle, m16, ...
  bool IsDirectlyShootable(Character& character);
  bool FindShootableEnemy();

  // for proximity weapons like dynamite, mine, ...
  bool IsNear(Character& character);
  bool FindProximityEnemy();

  // Watch the choosen enemy
  void ChooseDirection();
  Character* FindEnemy();

  void Shoot();
 public:
  AIShootModule();
  void BeginTurn();

  bool Refresh(uint current_time);
};

#endif
