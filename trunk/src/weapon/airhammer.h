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
 * AirHammer - Use it to dig
 *****************************************************************************/

#ifndef AIRHAMMER_H
#define AIRHAMMER_H
//-----------------------------------------------------------------------------
#include "include/base.h"
#include "weapon.h"
//-----------------------------------------------------------------------------

class AirhammerConfig : public WeaponConfig
{
  public:
    uint range;
    uint damage;
    AirhammerConfig();
    void LoadXml(xmlpp::Element *elem);
};

//-----------------------------------------------------------------------------

class Airhammer : public Weapon
{
  private:
    Surface impact;
    void RepeatShoot();

  protected:
    void p_Deselect();
    bool p_Shoot();
    void Refresh();

  public:
    Airhammer();
    AirhammerConfig &cfg();
    bool IsInUse() const;
    virtual void ActionStopUse();

    virtual void HandleKeyPressed_Shoot();
    virtual void HandleKeyRefreshed_Shoot();
    virtual void HandleKeyReleased_Shoot();

    DECLARE_GETWEAPONSTRING();
};

//-----------------------------------------------------------------------------
#endif /* AIRHAMMER_H */
