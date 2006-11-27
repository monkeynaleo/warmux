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
 * AirHammer - Use it to dig
 *****************************************************************************/

#ifndef AIRHAMMER_H
#define AIRHAMMER_H
//-----------------------------------------------------------------------------
#include <SDL.h>
#include "../include/base.h"
#include "weapon.h"
//-----------------------------------------------------------------------------

class AirhammerConfig : public WeaponConfig
{
  public:
    uint range;
    uint damage;
  public:
    AirhammerConfig();
    void LoadXml(xmlpp::Element *elem);
};

//-----------------------------------------------------------------------------

class Airhammer : public Weapon
{
  private:
    uint m_last_jolt;
    Surface impact;
    void RepeatShoot();

  protected:
    void p_Deselect();
    bool p_Shoot();
    void Refresh();

  public:
    Airhammer();
    void HandleKeyEvent(Action::Action_t action, Clavier::Key_Event_t event_type);
    AirhammerConfig &cfg();
};

//-----------------------------------------------------------------------------
#endif /* AIRHAMMER_H */
