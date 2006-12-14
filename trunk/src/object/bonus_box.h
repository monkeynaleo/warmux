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
 * Bonus Box
 *****************************************************************************/

#ifndef BONUS_BOX_H
#define BONUS_BOX_H
//-----------------------------------------------------------------------------
#include <SDL.h>
#include "../include/base.h"
#include "../object/physical_obj.h"
#include "../team/team.h"
#include "../weapon/weapons_list.h"
//-----------------------------------------------------------------------------

class BonusBox : public PhysicalObj
{
  private:
    static bool enable;
    static uint time;
    int nb_ammo;

    bool parachute; 
    Sprite *anim;

    Weapon::Weapon_type contents;

  private:
    static bool PlaceBonusBox (BonusBox& bonus_box);
    void ApplyBonus (Team &team, Character &character);
    void PickRandomWeapon();
  public:
    BonusBox();
    ~BonusBox();

    // Activate bonus box ?
    static void Enable (bool _enable);
    static bool NewBonusBox();
    //bool LoadXml(xmlpp::Element * object); //the next update will include this

    void Draw();
    void Refresh();

  protected:
    // Signal Fall ending
    virtual void SignalCollision();
    void SignalDeath();
};

//-----------------------------------------------------------------------------
#endif /* BONUS_BOX_H */
