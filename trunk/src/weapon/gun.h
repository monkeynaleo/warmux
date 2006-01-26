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
 * Weapon gun : la balle part tout droit dans la direction donn�e par
 * le viseur. Si la balle ne touche pas un ver, elle va faire un trou dans
 * le terrain. La balle peut �galement toucher les objets du plateau du jeu.
 *****************************************************************************/

#ifndef GUN_H
#define GUN_H

#include <SDL.h>
#include <vector>
#include "weapon.h"
#include "../include/base.h"
#include "../tool/point.h"

namespace Wormux {

class BalleGun : public WeaponProjectile
{
public:
  BalleGun();
  void Tire();
  void Init();
protected:
  void SignalCollision();
};

class Gun : public Weapon
{
private:
  uint temps_capture;
  std::vector<Point2i> lst_points;
  SDL_Surface *impact;
  void p_Init();
  bool p_Shoot();

public:  
  BalleGun balle;

  Gun();
  void Draw();
  void Refresh();
  WeaponConfig& cfg();
};

extern Gun gun;
} // namespace Wormux
#endif
