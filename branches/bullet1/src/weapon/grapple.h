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
 * Grapple
 *****************************************************************************/

#ifndef GRAPPLE_H
#define GRAPPLE_H
//-----------------------------------------------------------------------------
#include "weapon.h"
#include "include/base.h"
#include <list>
//-----------------------------------------------------------------------------

class GrappleConfig;

class Grapple : public Weapon
{
  private:
    Sprite* m_hook_sprite;
    Sprite* m_node_sprite;

    SoundSample cable_sound;

  protected:
    void Refresh();
    bool p_Shoot();
  public:
    Grapple();
    ~Grapple();
    void Draw();

    GrappleConfig& cfg();

    void UpdateTranslationStrings();
    std::string GetWeaponWinString(const char *TeamName, uint items_count) const;

    virtual void StartShooting();
    virtual void StopShooting();

    virtual bool IsPreventingLRMovement();
    virtual bool IsPreventingWeaponAngleChanges();
};

//-----------------------------------------------------------------------------
#endif /* GRAPPLE_H */
