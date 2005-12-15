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
 * Ciel : ce qui est affiché en image de fond.
 *****************************************************************************/

#ifndef CIEL_H
#define CIEL_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "tile.h"
#ifdef CL
# include <ClanLib/display.h>
#endif

struct SDL_Surface;

//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

   
class Ciel : public Tile
{  
 private:
  int lastx, lasty;
  void CompleteDraw();

public:
  Ciel();
  void Init();
  void Reset();
  void Draw();
  void Free() { FreeMem(); } 

 private:
#ifndef CL
   SDL_Surface *image;
#endif
};

}
//-----------------------------------------------------------------------------
#endif
