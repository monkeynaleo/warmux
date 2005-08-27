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
 * Refresh des ressources images.
 *****************************************************************************/

#include "graphism.h"
//-----------------------------------------------------------------------------
#include "../game/config.h"
//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------
Police police_grand;
Police police_petit;
Police police_mix;
Police police_weapon;
Graphisme graphisme;
//-----------------------------------------------------------------------------

bool EstTransparent(uchar alpha) 
{ return alpha != 255 ; }

//-----------------------------------------------------------------------------

void Graphisme::Init()
{
  res = CL_ResourceManager(config.data_dir+"graphism.xml", false);
  weapons = CL_ResourceManager(config.data_dir+"weapons.xml", false);
  police_grand.Load ("police_grand", &res);
  police_petit.Load ("police_petit", &res);
  police_mix.Load ("police_mix", &res);
  police_weapon.Load ("police_mix", &res);
  police_weapon.Acces().set_color(1,0.70,0.70,1);
}

//-----------------------------------------------------------------------------

// Accès aux ressources
CL_ResourceManager *Graphisme::LitRes()
{
  // Resources chargées ?
  //  assert (res);
  return &res;
}

//-----------------------------------------------------------------------------

// Ecrit un texte et l'encadre
void TexteEncadre (Police &police, int txt_x, int txt_y, 
		   const std::string &txt, uint espace)
{
  int x,y,larg,haut;
  larg = police.GetWidth(txt)+espace*2;
  x = txt_x - larg / 2;
  haut = police.GetHeight(txt)+espace*2;
  y = txt_y - haut / 2;
  txt_y -= police.GetHeight(txt)/2;
  CL_Display::fill_rect (CL_Rect(x, y, x+larg, y+haut), CL_Color(0, 0, 0, 255*7/10));
  CL_Display::draw_rect (CL_Rect(x, y, x+larg, y+haut), CL_Color::red);

  police.WriteCenterTop (txt_x, txt_y, txt);
}

//-----------------------------------------------------------------------------
} // namespace Wormux
