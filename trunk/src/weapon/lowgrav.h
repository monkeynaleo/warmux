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
 * Low gravity
 *****************************************************************************/

#ifndef LOWGRAV_H
#define LOWGRAV_H
#include "weapon.h"

class LowGrav : public Weapon
{
  public:
    LowGrav();
    void SignalTurnEnd();
    void Draw();
    void ActionStopUse();
    void HandleKeyPressed_Shoot();
  protected:
    void Refresh();
    void p_Deselect();
    bool p_Shoot();
};

#endif /* LOWGRAV_H */
