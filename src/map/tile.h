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

#ifndef CLIPPING_H
#define CLIPPING_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include <vector>
#ifdef CL
# include <ClanLib/display.h>
#endif
//-----------------------------------------------------------------------------

struct SDL_Surface;

class TileItem 
{
public:
#ifdef CL
  TileItem (uint width, uint height, CL_PixelFormat format=CL_PixelFormat::rgba8888);
#else
  TileItem (uint width, uint height);
#endif   
  TileItem (const TileItem &copy);
  ~TileItem();
  
  void Lock();
  void Unlock(bool force=false);  
  void Sync(); // Save last changes (if needed)
  void Touch(); // Touch it -> mark it as changed
  
  uchar* get_data();
#ifdef CL
  CL_Surface& get_surface();
  CL_PixelBuffer& get_pixelbuffer();
#else
  unsigned char GetAlpha(const int x,const int y) const;
  SDL_Surface *get_surface();
  unsigned char *get_pixelbuffer();
  void Dig( int ox, int oy, SDL_Surface *dig);
#endif
   
private:
  bool m_locked;
  bool m_changed;
  uint m_width, m_height;
#ifdef CL
  CL_Surface m_surface;
  CL_PixelBuffer m_buffer; // if not NULL, contains update canvas
#else
  SDL_Surface *m_surface;
  unsigned char *m_buffer;
#endif
};  

//-----------------------------------------------------------------------------
 
class Tile
{
protected:
  // Dimension du terrain
  uint larg, haut;

  uint larg_cell, haut_cell;
  uint nbr_cell_larg, nbr_cell_haut;
  uint nbr_cell;

  // Canvas donnant accès aux cellules
  std::vector<TileItem> item;

public:
  Tile();
  ~Tile();

  // Dig a hole
#ifdef CL
   void Dig (int x, int y, uint width, uint height, CL_PixelBuffer provider);
#else
   void Dig (int ox, int oy, SDL_Surface *provider);
#endif
   
  // Load an image
#ifdef CL
  void LoadImage (CL_Surface &terrain);
#else
  void LoadImage (SDL_Surface *terrain);
#endif
  // Get size
  uint GetWidth() const { return larg; }
  uint GetHeight() const { return haut; }

  // Get alpha value of a pixel
  uchar GetAlpha (const int x, const int y) const;

  // Draw it
  void DrawTile();

protected:
  // Initialise la position, la taille des cellules, et la taille du terrain
  void InitTile (uint larg_cell, uint haut_cell, uint larg, uint haut);

  void FreeMem();
};

//-----------------------------------------------------------------------------
#endif
