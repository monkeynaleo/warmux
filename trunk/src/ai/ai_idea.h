/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * A factory for AI strategies. It contains no turn specfic data.
 *****************************************************************************/

#ifndef AI_IDEA_H
#define AI_IDEA_H

#include "ai/ai_strategy.h"
#include "weapon/weapon.h"


class AIIdea
{
  protected:
    bool CanUseWeapon(Weapon * weapon);
  public:
    virtual AIStrategy * CreateStrategy() = 0;
    virtual ~AIIdea() {}
};

class SkipTurnIdea : public AIIdea
{
  public:
    virtual AIStrategy * CreateStrategy();
};

class WasteAmmoUnitsIdea : public AIIdea
{
  public:
    virtual AIStrategy * CreateStrategy();
};

class ShootDirectlyAtEnemyIdea : public AIIdea
{
  private:
    Character & shooter;
    Character & enemy;
    Weapon::Weapon_type weapon_type;
    double max_distance;
  public:
    ShootDirectlyAtEnemyIdea(Character & shooter, Character & enemy, Weapon::Weapon_type weapon_type, double max_distance);
    virtual AIStrategy * CreateStrategy();
};

#endif
