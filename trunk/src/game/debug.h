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
 * Classes aidant au débogage.
 *****************************************************************************/

#ifndef DEBUG_H
#define DEBUG_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
//-----------------------------------------------------------------------------
#ifdef DEBUG

#include <vector>
#include <string>
//-----------------------------------------------------------------------------

namespace Wormux
{

class Debug
{
private:
  int repere_x, repere_y;
  int repere_x_monde, repere_y_monde;

public:
  std::vector<std::string> msg;
  Debug();
  void Refresh();
  void Draw();
};

extern Debug debug;
}
#endif // DEBUG
//-----------------------------------------------------------------------------
#endif // ifndef DEBUG_H
