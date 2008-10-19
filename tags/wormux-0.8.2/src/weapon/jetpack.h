/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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

#include "weapon.h"

class JetPack : public Weapon
{
  private:
    double m_x_force;
    double m_y_force;

    SoundSample flying_sound;

    // Jetpack fuel.
    uint m_last_fuel_down;

  public:
    JetPack();
    void Reset();
    virtual void SignalTurnEnd() { p_Deselect(); };
    virtual void ActionStopUse();

    virtual void HandleKeyPressed_Up(bool shift);
    virtual void HandleKeyReleased_Up(bool shift);
    virtual void HandleKeyPressed_MoveLeft(bool shift);
    virtual void HandleKeyReleased_MoveLeft(bool shift);
    virtual void HandleKeyPressed_MoveRight(bool shift);
    virtual void HandleKeyReleased_MoveRight(bool shift);
    virtual void HandleKeyPressed_Shoot(bool shift);

    void UpdateTranslationStrings();
    std::string GetWeaponWinString(const char *TeamName, uint items_count ) const;

  protected:
    void Refresh();
    void p_Select();
    void p_Deselect();
    bool p_Shoot();

  private:
    void GoUp();
    void GoLeft();
    void GoRight();
    void StopUp() { m_y_force = 0.0; StopUse(); };
    void StopLeft() { m_x_force = 0.0; StopUse(); };
    void StopRight() { m_x_force = 0.0; StopUse(); };
    void StartUse();
    void StopUse();
};

#endif /* JETPACK_H */
