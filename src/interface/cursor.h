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
 * Curseur clignotant montrant la position d'un ver actif.
 *****************************************************************************/

#ifndef CURSOR_H
#define CURSOR_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../object/physical_obj.h"
#include <ClanLib/display.h>
//-----------------------------------------------------------------------------

class CurseurVer
{
private:
  uint nbr_clignot;
  uint temps;
  bool affiche;
  PhysicalObj *obj_designe;
  bool clignote;
  bool actif;
  bool designe_ver_actif;

  uint nbr_boucle;
  uint y_mouvement;
  bool monter;

public:
  CL_Sprite image;

public:
  CurseurVer();

  void Init();
  void Reset();
  void Draw();
  void Refresh();

  // Suit le ver actif
  void SuitVerActif();

  // Pointe un objet
  void PointeObj (PhysicalObj *obj);

  // Ne pointe plus d'objet (à part si le ver actif était suivi)
  void PointeAucunObj ();

  // Cache le curseur
  void Cache();
};

extern CurseurVer curseur_ver;
//-----------------------------------------------------------------------------
#endif
