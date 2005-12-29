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
#ifdef CL
#include <ClanLib/display.h>
#else
#endif
//-----------------------------------------------------------------------------

class PhysicalObj;
struct Sprite;

class CurseurVer
{
public:
   Sprite *image;

private:
  uint time_begin_anim;
  uint last_update;
  PhysicalObj *obj_designe;
  int dy;
  bool actif;
  bool want_hide;

public:
  CurseurVer();
  ~CurseurVer();

  void Init();
  void Reset();
  void Draw();
  void Refresh();

  // Pointe un objet
  void PointeObj (PhysicalObj *obj);

  // Pointe un objet
  void SuitVerActif ();

  // Cache le curseur
  void Cache();

private:
  bool IsDisplayed() const;
};

extern CurseurVer curseur_ver;
//-----------------------------------------------------------------------------
#endif
