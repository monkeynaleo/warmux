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
 * Monde ou plateau de jeu.
 *****************************************************************************/

#ifndef MAP_H
#define MAP_H

#include "ground.h"
#include "sky.h"
#include "water.h"
#include "../graphic/surface.h"
#include "../graphic/sprite.h"
#include "../graphic/text.h"
#include "../include/base.h"
#include "../object/physical_obj.h"

extern const uint MAX_WIND_OBJECTS;

class Map{
 private:
  Text * author_info1;
  Text * author_info2;

public:
  Map();
  ~Map();
   
  Ground ground;
  Sky sky;
  double dst_min_entre_vers;
  Water water;

  std::list<Rectanglei> *to_redraw;
  std::list<Rectanglei> *to_redraw_now;
  std::list<Rectanglei> *to_redraw_particles;
  std::list<Rectanglei> *to_redraw_particles_now;

public:
  void Reset();
  void Refresh();
  void FreeMem();
  void Draw();
  void DrawWater();
  void DrawSky();
  void DrawAuthorName();

  // To manage the cache mechanism
  void ToRedrawOnMap(Rectanglei r);
  void ToRedrawOnScreen(Rectanglei r);

  // Est-on dans le monde ou dans le vide ?
  bool EstDansVide(const Point2i &pos) const;
  bool EstDansVide (int x, int y) const;
  bool LigneH_EstDansVide (int left, int y,  int right) const;
  bool LigneV_EstDansVide (int x,  int top, int bottom) const;
  bool RectEstDansVide (const Rectanglei &rect) const;
  bool ParanoiacRectIsInVacuum (const Rectanglei &rect) const;

  // La ligne du haut/bas d'un objet physique est dans le vide ?
  // Le test se fait sur le rectangle de test d�al�de dx et dy.
  bool EstDansVide_haut (const PhysicalObj &obj, int dx, int dy) const;
  bool EstDansVide_bas (const PhysicalObj &obj, int dx, int dy) const;
  bool IsInVacuum_left (const PhysicalObj &obj, int dx, int dy) const;
  bool IsInVacuum_right (const PhysicalObj &obj, int dx, int dy) const;

  // Est en dehors du monde ?
  bool EstHorsMondeX (int x) const;
  bool EstHorsMondeY (int x) const;
  bool EstHorsMondeXlarg (int x, uint larg) const;
  bool EstHorsMondeYhaut (int x, uint haut) const;
  bool EstHorsMondeXY (int x, int y) const;
  bool EstHorsMonde (const Point2i &pos) const;

  // Is it an open or closed world ?
  bool IsOpen() const { return ground.EstOuvert(); }

  // Dig the map using a picture
  void Dig(const Point2i position, const Surface& alpha_sur);
  // Dig a circle hole in the map
  void Dig(const Point2i center, const uint radius);

  // Insert a sprite into the ground
  void PutSprite(const Point2i pos, Sprite* spr);
  // Merge a sprite into the ground
  void MergeSprite(const Point2i pos, Sprite* spr);

  // Lit la taille du monde
  int GetWidth() const { return ground.GetSizeX(); }
  int GetHeight() const { return ground.GetSizeY(); }
  Point2i GetSize() const{ return ground.GetSize(); }
 private:
  void SwitchDrawingCache();
  void SwitchDrawingCacheParticles();
  void OptimizeCache(std::list<Rectanglei>& rectangleCache);
};

extern Map world;
#endif
