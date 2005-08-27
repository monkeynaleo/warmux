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
 * Monde ou plateau de jeu.
 *****************************************************************************/

#ifndef MONDE_H
#define MONDE_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../object/physical_obj.h"
#include "sky.h"
#include "water.h"
#include "ground.h"
//-----------------------------------------------------------------------------

extern const uint MAX_WIND_OBJECTS;

class Monde
{
public:
  Wormux::Terrain terrain;
  Wormux::Ciel ciel;
  double dst_min_entre_vers;
  Water water;

private:
  bool raz_terrain;

public:
  void Init();
  void Reset();
  void Refresh();
  void FreeMem();
  void Draw();
  void DrawWater();
  void DrawSky();
  void DrawAuthorName();

  // Est-on dans le monde ou dans le vide ?
  bool EstDansVide (int x, int y);
  bool LigneH_EstDansVide (int left, int y,  int right);
  bool LigneV_EstDansVide (int x,  int top, int bottom);
  bool RectEstDansVide (const CL_Rect &rect);

  // La ligne du haut/bas d'un objet physique est dans le vide ?
  // Le test se fait sur le rectangle de test décalé de dx et dy.
  bool EstDansVide_haut (const PhysicalObj &obj, int dx, int dy);
  bool EstDansVide_bas (const PhysicalObj &obj, int dx, int dy);

  // Est en dehors du monde ?
  bool EstHorsMondeX (int x) const;
  bool EstHorsMondeY (int x) const;
  bool EstHorsMondeXlarg (int x, uint larg) const;
  bool EstHorsMondeYhaut (int x, uint haut) const;
  bool EstHorsMondeXY (int x, int y) const;
  bool EstHorsMonde (const CL_Point &pos) const;

  // C'est un terrain ouvert ?
  bool EstOuvert() const { return terrain.EstOuvert(); }

  // Creuse un pixel
  void Creuse(uint x, uint y, CL_Surface &alpha_sur);

  // Lit la taille du monde
  uint GetWidth() const { return terrain.GetWidth(); }
  uint GetHeight() const { return terrain.GetHeight(); }
};

extern Monde monde;
//-----------------------------------------------------------------------------
#endif
