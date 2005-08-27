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
 * Refresh des ressources graphiques.
 *****************************************************************************/

#ifndef GRAPHISM_H
#define GRAPHISM_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "font.h"
#include <ClanLib/display.h>
#include <ClanLib/core.h>
//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

// Détermine si une valeur alpha est transparent ou non
bool EstTransparent(uchar alpha);

//-----------------------------------------------------------------------------

// Polices de caractère
extern Police police_grand;
extern Police police_petit;
extern Police police_mix;
extern Police police_weapon;

// Ecrit un texte et l'encadre
void TexteEncadre (Police &police, 
		   int x, int y, 
		   const std::string &txt,
		   uint espace=10);

//-----------------------------------------------------------------------------

class Graphisme
{
private:
  // Refreshnaire des ressources
  CL_ResourceManager res;

public:
  CL_ResourceManager weapons;

public:
  // Accès aux ressources
  CL_ResourceManager *LitRes();

  // Initialise le ressource manager
  void Init();
};

extern Graphisme graphisme;
//-----------------------------------------------------------------------------
}
#endif
