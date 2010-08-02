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
 * Bonus Box
 *****************************************************************************/

#ifndef BONUS_BOX_H
#define BONUS_BOX_H
//-----------------------------------------------------------------------------
#include <vector>
#include "include/base.h"
#include "weapon/weapon.h"
#include "objbox.h"

// Forward declarations
class Character;
class Action;
//-----------------------------------------------------------------------------

struct WeaponProba
{
  Weapon* weapon;
  int nb_ammos;
  double probability;
};

class BonusBox : public ObjBox
{
  private:
    uint weapon_num;
    bool ExplodesInsteadOfBonus(Character * c);

    static double total_probability;
    static std::vector<struct WeaponProba> weapon_list;

  private:
    void ApplyBonus (Team &team, Character &character);
    void PickRandomWeapon();
  public:
    BonusBox();
    static void LoadXml(const xmlNode* object);

    void ApplyBonus(Character *);
    void Randomize();
};

//-----------------------------------------------------------------------------
#endif /* BONUS_BOX_H */