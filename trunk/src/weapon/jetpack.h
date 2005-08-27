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
 * Jet Pack :-)
 *****************************************************************************/

#ifndef JETPACK_H
#define JETPACK_H
//-----------------------------------------------------------------------------
#include "weapon.h"
#include <ClanLib/sound.h>
//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

class JetPack : public Weapon
{
private:
  double m_x_force;
  double m_y_force;

  // Jetpack fuel.
  uint m_last_fuel_down;

public:
  JetPack();
  void Reset();
  void p_Init();
  void Refresh();
  void p_Select();
  void p_Deselect();
  void HandleKeyEvent(int key, int event_type) ;
  bool p_Shoot();
  void SignalTurnEnd();

protected:
  void GoUp();
  void GoLeft();
  void GoRight();
  void StopUp();
  void StopLeft();
  void StopRight();
  void StartUse();
  void StopUse();

};

extern JetPack jetpack;
//-----------------------------------------------------------------------------
} // namespace Wormux
#endif
