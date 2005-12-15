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
 * Terrain de jeu.
 *****************************************************************************/

#ifndef TERRAIN_H
#define TERRAIN_H
//-----------------------------------------------------------------------------
#include "tile.h"
#include <list>
#ifdef CL
# include <ClanLib/display.h>
#endif
//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

class Terrain : public Tile 
{
private:
  bool ouvert;
  bool PointContigu(int x,int y, int & p_x,int & p_y,
                    int pas_bon_x,int pas_bon_y);
  int lastx;
  int lasty;

public:
  Terrain();
  void Init();
  void Draw();
  void Reset();
  void Free() { FreeMem(); }

  // Le point (x,y) est dans le vide ?
  bool EstDansVide (int x, int y);

  // C'est un terrain ouvert ?
  bool EstOuvert() const { return ouvert; }
  
  //Renvoie l'angle de la tangeante au terrain
  double Tangeante(int x,int y);
};

//-----------------------------------------------------------------------------
} // namespace Wormux
#endif
