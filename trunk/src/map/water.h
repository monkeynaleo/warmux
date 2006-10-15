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
 * Refresh de l'eau pouvant apparaitre en bas du terrain.
 *****************************************************************************/

#ifndef WATER_H
#define WATER_H

#include <vector>
#include "../graphic/surface.h"
#include "../include/base.h"

const uint WATER_INITIAL_HEIGHT = 100;

class Water
{
private:
  uint temps_eau;
  int height_mvt;
  int vague;
  bool actif;
  double shift1;
  uint hauteur_eau;
  uint temps_montee;
  std::vector<int> height;
  Surface surface;
  Surface pattern;
public:
  void Init();
  void Reset();
  void Free();
  void Refresh();
  void Draw();
  bool IsActive();
  int GetHeight(int x);
};
#endif
