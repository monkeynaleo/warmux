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
 * Clipping : découpe une grosse image en petit morceaux indépendants pour
 * permettre un chargement des donnéees plus rapide. On peut ensuite creuser
 * des trous dans cette image (en touchant à la couche alpha).
 *****************************************************************************/

#ifndef _TILE_H
#define _TILE_H

#include "../include/base.h"
#include <vector>
#include "../tool/Rectangle.h"

struct SDL_Surface;

class TileItem
{
public:
  TileItem () {};
  virtual ~TileItem () {};
   
  bool IsEmpty();
  virtual unsigned char GetAlpha(const int x,const int y) const = 0;
  virtual void Dig( int ox, int oy, SDL_Surface *dig) = 0;
  virtual SDL_Surface *GetSurface() = 0;
  virtual void SyncBuffer() = 0; // (if needed)
  virtual void Draw(const int x,const int y);
};

class TileItem_Empty : public TileItem
{
public:
  TileItem_Empty () {};
  ~TileItem_Empty () {};
   
  unsigned char GetAlpha(const int x,const int y) const {return 0;};
  void Dig( int ox, int oy, SDL_Surface *dig) {};
  SDL_Surface *GetSurface() {return NULL;};
  void SyncBuffer() {}; // (if needed)
  void Draw(const int x,const int y) {};
};

class TileItem_AlphaSoftware : public TileItem
{
public:
  TileItem_AlphaSoftware (unsigned int width, unsigned int height);
  TileItem_AlphaSoftware (const TileItem_AlphaSoftware &copy);
  ~TileItem_AlphaSoftware ();
    
  unsigned char GetAlpha (const int x, const int y) const;
  SDL_Surface *GetSurface ();
  void Dig (int ox, int oy, SDL_Surface *dig);
  void SyncBuffer ();
 
private:
  unsigned int m_width, m_height;
  SDL_Surface *m_surface;
};  

class TileItem_AlphaHardware : public TileItem
{
public:
  TileItem_AlphaHardware (unsigned int width, unsigned int height);
  TileItem_AlphaHardware (const TileItem_AlphaHardware &copy);
  ~TileItem_AlphaHardware ();
    
  unsigned char GetAlpha (const int x, const int y) const;
  SDL_Surface *GetSurface ();
  void Dig (int ox, int oy, SDL_Surface *dig);
  void SyncBuffer ();
   
private:
  unsigned int m_width, m_height;
  SDL_Surface *m_surface;
  unsigned char *m_buffer;
};  

class TileItem_ColorkeySoftware : public TileItem
{
public:
  TileItem_ColorkeySoftware (unsigned int width, unsigned int height);
  TileItem_ColorkeySoftware (const TileItem_ColorkeySoftware &copy);
  ~TileItem_ColorkeySoftware ();
    
  unsigned char GetAlpha (const int x, const int y) const;
  SDL_Surface *GetSurface ();
  void Dig (int ox, int oy, SDL_Surface *dig);
  void SyncBuffer ();
 
private:
  unsigned int m_width, m_height;
  SDL_Surface *m_surface;
  unsigned char *m_buffer;
};  

//-----------------------------------------------------------------------------
 
class Tile
{
public:
  Tile ();
  ~Tile ();

  // Dig a hole
  void Dig (int ox, int oy, SDL_Surface *provider);
   
  // Load an image
  void LoadImage (SDL_Surface *terrain);

  // Get size
  unsigned int GetWidth () const { return larg; }
  unsigned int GetHeight () const { return haut; }

  // Get alpha value of a pixel
  unsigned char GetAlpha (const int x, const int y) const;

  // Draw it (on the entire visible part) 
  void DrawTile();
   
  // Draw a part that is inside the given clipping rectangle
  // Clipping rectangle is in World corrdinate not screen coordinates
  // usefull to redraw only a part that is under a sprite that has moved,...
 
  void DrawTile_Clipped( Rectanglei clip_rectangle);
   
protected:
  // Initialise la position, la taille des cellules, et la taille du terrain
  void InitTile (unsigned int larg, unsigned int haut);

  void FreeMem();

  // Dimension du terrain
  unsigned int larg, haut;

  unsigned int nbr_cell_larg, nbr_cell_haut;
  unsigned int nbr_cell;

  // Canvas donnant accès aux cellules
  std::vector<TileItem *> item;
};

#endif // _TILE_H
