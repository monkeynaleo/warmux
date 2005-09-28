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
 * Tile : découpe une grosse image en petit morceaux indépendants pour
 * permettre un chargement des donnéees plus rapide. On peut ensuite creuser
 * des trous dans cette image (en touchant à la couche alpha).
 *****************************************************************************/

#include "tile.h"
//-----------------------------------------------------------------------------
#include "../graphic/graphism.h"
#ifdef CL
# include <ClanLib/core.h>
#else
#include <SDL.h>
#include "../tool/Point.h"
#include "../include/app.h"
#include "../map/camera.h"
#include "../include/constant.h"
#endif
#include <iostream>
using namespace Wormux;
//-----------------------------------------------------------------------------


// Calcule l'offset a l'interieur d'un canvas
#define OPP 4 // octets/pixel, on travaille en 32 bits/pixels

// La fonction CreuseCellule utilise la couche alpha de l'image 'creuse'
// ou alors utilise simplement les états opaques/transparents ?
#define CREUSE_UTILISE_ALPHA

#ifdef DEBUG
   // Messages de debug du cellping ?
//#define MSG_DBG_CLIPPING

// Débogue la préparation d'une copie ?
//#define DEBUG_PREPARE_COPIE

#define COUT_DBG std::cout << "[Tile] "
#endif

//-----------------------------------------------------------------------------

#ifdef CL
TileItem::TileItem (uint width, uint height, CL_PixelFormat format)
 : m_locked(false), m_changed(false), m_width(width), m_height(height)
{
  m_buffer = CL_PixelBuffer(width, height, width*4, format);

  //#if CL_CURRENT_VERSION <= 0x0708 
  //m_surface = CL_Surface(&m_buffer, false);
  //#else
  m_surface = CL_Surface(m_buffer, 0);
  //#endif
}
#else
TileItem::TileItem (uint width, uint height)
{
   m_locked = false;
   m_changed = false;
   m_width = width;
   m_height = height;

   m_surface = SDL_CreateRGBSurface( /*SDL_SWSURFACE|*/SDL_SRCALPHA, width, height, 
				     32, //sdlscreen->format->BitsPerPixel,
				     0x000000ff, //sdlscreen->format->Rmask,
				     0x0000ff00,
				     0x00ff0000,
				     0xff000000);

   m_buffer = new unsigned char[m_height*m_width];
}
#endif
//-----------------------------------------------------------------------------

#ifdef CL
TileItem::TileItem (const TileItem &copy)
 : m_width(copy.m_width), m_height(copy.m_height)
{
  //#if CL_CURRENT_VERSION <= 0x0708 
  //m_surface = CL_Surface(new CL_PixelBuffer(copy.m_buffer), true);
  //#else
  m_surface = CL_Surface(CL_PixelBuffer(copy.m_buffer), 0);
  //#endif

  m_buffer = m_surface.get_pixeldata();
   
  m_changed = false;
  m_locked = false;
}
#else
TileItem::TileItem (const TileItem &copy)
{
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface = SDL_CreateRGBSurface( /*SDL_SWSURFACE|*/SDL_SRCALPHA, m_width, m_height, 
				     32,
				     0x000000ff,
				     0x0000ff00,
				     0x00ff0000,
				     0xff000000);

   SDL_Rect dest_rect = {0,0, copy.m_surface->w, copy.m_surface->h};
   SDL_BlitSurface( copy.m_surface, NULL, m_surface, &dest_rect);

   m_buffer = new unsigned char[m_height*m_width];
   memcpy( copy.m_buffer, m_buffer, m_height*m_width*sizeof( unsigned char));
   
   m_changed = false;
   m_locked = false;
}

#endif

//-----------------------------------------------------------------------------

void TileItem::Lock() 
{
  if (m_locked) return;
#ifdef CL
  m_buffer.lock(); 
#endif
  m_locked = true;
}

//-----------------------------------------------------------------------------

TileItem::~TileItem()
{
  Unlock (true);
}

//-----------------------------------------------------------------------------

#ifdef CL
CL_PixelBuffer& TileItem::get_pixelbuffer() 
{
  Lock();
  return m_buffer;
}
#else
unsigned char * TileItem::get_pixelbuffer() 
{
  return m_buffer;
}
#endif

//-----------------------------------------------------------------------------

uchar* TileItem::get_data() 
{
  Lock();
#ifdef CL
  return (uchar*)m_buffer.get_data();
#else
  return (unsigned char *) m_buffer; 
#endif
}


#ifndef CL

unsigned char TileItem::GetAlpha(const int x,const int y) const
{
   return *(((unsigned char *)m_surface->pixels) + y*m_surface->pitch + x * 4 + 3);
}

#endif

//-----------------------------------------------------------------------------

void TileItem::Touch() 
{
  m_changed = true;
}

//-----------------------------------------------------------------------------

void TileItem::Unlock(bool force) 
{
  if (!m_changed && !force) return;
  if (!m_locked) return;
#ifdef CL
  m_buffer.unlock(); 
  m_locked = false;
  if (m_changed) 
  {
    m_surface.set_pixeldata(m_buffer); 
    m_buffer = CL_PixelBuffer(m_surface.get_pixeldata()); // is it needed ???
    m_changed = false;
  }
#endif
}

//-----------------------------------------------------------------------------

#ifdef CL
CL_Surface& TileItem::get_surface()
{
  Unlock();
  return m_surface;
}
#else
SDL_Surface *TileItem::get_surface()
{
   Unlock();
   return m_surface;
}
#endif
//-----------------------------------------------------------------------------

void TileItem::Sync()
{
  Unlock();
}

void TileItem::Dig( int ox, int oy, SDL_Surface *dig)
{
   int starting_x = ox >= 0 ? ox : 0;   
   int starting_y = oy >= 0 ? oy : 0; 
   int ending_x = ox+dig->w <= m_surface->w ? ox+dig->w : m_surface->w;
   int ending_y = oy+dig->h <= m_surface->h ? oy+dig->h : m_surface->h;
   
   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       {
	  
	  if ( *(((unsigned char *)dig->pixels) + (py-oy)*dig->pitch + (px-ox) * 4 + 3) != 0)
	    
	    *(((unsigned char *)m_surface->pixels) + py*m_surface->pitch + px * 4 + 3) = 0;


       }
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Tile::Tile()
{
  larg_cell = 100;
  haut_cell = 100;
}

//-----------------------------------------------------------------------------

void Tile::FreeMem()
{
  item.clear();
}

//-----------------------------------------------------------------------------

Tile::~Tile()
{ 
  FreeMem();
}

//-----------------------------------------------------------------------------

void Tile::InitTile (uint plarg_cell, uint phaut_cell, 
		     uint plarg, uint phaut)
{
  larg_cell = plarg_cell;
  haut_cell = phaut_cell;

  nbr_cell_larg = plarg / plarg_cell;
  if ((plarg % plarg_cell) != 0) nbr_cell_larg++;
  nbr_cell_haut = phaut / phaut_cell;
  if ((phaut % phaut_cell) != 0) nbr_cell_haut++;

  larg = plarg;
  haut = phaut;

  nbr_cell = nbr_cell_larg * nbr_cell_haut; 

#ifdef MSG_DBG_CLIPPING
  COUT_DBG << "Initialisation : " << larg << "x" << haut
	   << " découpé en " 
	   << nbr_cell_larg << "x" << nbr_cell_haut
	   << " cellules." << std::endl;
  COUT_DBG << "Chaque cellule a une taille de "
	   << larg_cell << "x" << haut_cell << std::endl;
#endif
}

//-----------------------------------------------------------------------------

// Creuse un trou dans un cell
// Le cell sera creusé dans la pièce 'cell'
// Les coordonnées dans ce trou sont x,y, et la taille est larg x haut
// !!! Ne vérifie ou ne corrige aucune valeur, c'est à vous de le faire !!!
#ifdef CL
void CreuseCellule (CL_PixelBuffer &creuse, 
		    CL_Point pos_creuse,
		    uint larg, uint haut,
		    TileItem &item,
		    CL_Point pos_cell)
{
  // Lecture des données du motif utilisé pour creuser
  const uint creuse_pitch = creuse.get_pitch();
  creuse.lock();
  uchar *creuse_data = (uchar *)creuse.get_data();
  creuse_data += pos_creuse.x*OPP;
  creuse_data += pos_creuse.y*creuse_pitch;
  creuse_data += 3;

  // Lecture des données du cell
  CL_PixelBuffer cell = item.get_pixelbuffer();
  item.Touch();
  const uint cell_pitch = cell.get_pitch();
  cell.lock();
  uchar *cell_data = (uchar *)cell.get_data();
  cell_data += pos_cell.x*OPP;
  cell_data += pos_cell.y*cell_pitch;
  cell_data += 3;

  // Sauve les données
  uchar *sauve_creuse_data = creuse_data;
  uchar *sauve_cell_data = cell_data;
  
  for (uint dy=0; dy < haut; ++dy)
  {
    for (uint dx=0; dx < larg; ++dx)
    {
      // Traite un pixel
#ifdef CREUSE_UTILISE_ALPHA
      float alpha = *cell_data;

      //Apply the alpha value of the hole on the map
      if(*creuse_data<*cell_data)
        alpha -= *creuse_data;
      else
        alpha = 0;

      //Darken the map if the alpha of the hole isn't invisible;
      if(*creuse_data>0)
      {
        *(cell_data-3)/=2;
        *(cell_data-2)/=2;
        *(cell_data-1)/=2;
      }
//      alpha *= (255 - *creuse_data)/255;
      *cell_data = (uchar)alpha;
#else
      if (!EstTransparent(*creuse_data)) *cell_data = 0;
#endif
      cell_data += OPP;
      creuse_data += OPP;
    }

    // Passe à la ligne suivante
    sauve_creuse_data += creuse_pitch;
    creuse_data = sauve_creuse_data;

    sauve_cell_data += cell_pitch;
    cell_data = sauve_cell_data;
  }
  cell.unlock();
  creuse.unlock();
}

#else // CL is undefinded

#endif // CL not defined

//-----------------------------------------------------------------------------

// Prépare une surface pour la copie
// Renvoie false si il n'y a rien à copier
#ifdef CL
bool PrepareSurfaceCopie (CL_PixelBuffer &source,
			  CL_Point &pos, CL_Point &pos2,
			  uint &larg, uint &haut)
{
#ifdef DEBUG_PREPARE_COPIE
#  define COUT_PREPARE COUT_DBG << "[PrepareCopie "<<pos.x<<","<<pos.y<<"] "
#endif
  // On est hors de la source : exit
  int src_larg = source.get_width();
  int src_haut = source.get_height();
  if (src_larg<pos.x) return false;
  if (src_haut<pos.y) return false;
  if (pos.x+larg < 0) return false;
  if (pos.y+haut < 0) return false;

  // Corrige x et/ou y si nécessaire
  if (pos.x < 0) 
  {
    uint diff = -pos.x;
#ifdef DEBUG_PREPARE_COPIE
    COUT_PREPARE << "Corrige x (diff="<< diff << ")." << std::endl;
#endif
    pos.x += diff;
    pos2.x += diff;
    larg -= diff;
  }
  if (pos.y < 0) 
  {
    uint diff = -pos.y;
#ifdef DEBUG_PREPARE_COPIE
    COUT_PREPARE << "Corrige y (diff=" 
		 << diff << ")." << std::endl;
#endif
    pos.y += diff;
    pos2.y += diff;
    haut -= diff;
  }
  if (src_larg<pos.x+(int)larg) 
  {
#ifdef DEBUG_PREPARE_COPIE
    COUT_PREPARE << "Corrige larg ("
		 << larg << " -> " << src_larg - pos.x
		 << ")" << std::endl;
#endif
    larg = src_larg - pos.x;
  }
  if (src_haut < pos.y+(int)haut) 
  {
#ifdef DEBUG_PREPARE_COPIE
    COUT_PREPARE << "Corrige haut ("
	     << haut << " -> " << src_haut - pos.y
	     << ")" <<std::endl;
#endif
    haut = src_haut - pos.y;
  }
  return true;
}
#else
//TODO
#endif
//-----------------------------------------------------------------------------

// Copie source dans destination
// (larg x haut) donne les dimensions de la zone copiée depuis source
#ifdef CL
void CopieCell (CL_PixelBuffer source,
		CL_Point pos_src,
		uint larg, uint haut,
		TileItem& item_dest,
		CL_Point pos_dst)
{
  CL_PixelBuffer &dest = item_dest.get_pixelbuffer();
  item_dest.Touch();
	
  if (!PrepareSurfaceCopie (source,pos_src, pos_dst, larg, haut)) return;
  if (!PrepareSurfaceCopie (dest,pos_dst, pos_src, larg, haut)) return;

  // Lock datas
  source.lock();
  dest.lock();
  
  // Lecture des données source
  const uint source_pitch = source.get_pitch();
  uchar *source_data = (uchar *)source.get_data();
  source_data += pos_src.x*OPP;
  source_data += pos_src.y*source_pitch;

  // Lecture des données destination
  const uint dest_pitch = dest.get_pitch();
  uchar *dest_data = (uchar *)dest.get_data();
  dest_data += pos_dst.x*OPP;
  dest_data += pos_dst.y*source_pitch;

  // Sauve les données
  uchar *sauve_source_data = source_data;
  uchar *sauve_dest_data = dest_data;

  for (uint dy=0; dy < haut; ++dy)
  {
    memcpy (dest_data, source_data, larg*OPP);

    // Passe à la ligne suivante dans source
    sauve_source_data += source_pitch;
    source_data = sauve_source_data;

    // Passe à la ligne suivante dans dest
    sauve_dest_data += dest_pitch;
    dest_data = sauve_dest_data;
  }

  // Unlock datas
  dest.unlock();
  source.unlock();
}
#else
void CopieCell (SDL_Surface *source,
		Point2i pos_src,
		uint larg, uint haut,
		TileItem& item_dest,
		Point2i pos_dst)
{
/*  SDL_Surface *dest = item_dest.get_pixelbuffer();
  item_dest.Touch();
	
  if (!PrepareSurfaceCopie (source,pos_src, pos_dst, larg, haut)) return;
  if (!PrepareSurfaceCopie (dest,pos_dst, pos_src, larg, haut)) return;

  // Lock datas
//  source.lock();
//  dest.lock();
  
  // Lecture des données source
  const uint source_pitch = source.get_pitch();
  uchar *source_data = (uchar *)source.get_data();
  source_data += pos_src.x*OPP;
  source_data += pos_src.y*source_pitch;

  // Lecture des données destination
  const uint dest_pitch = dest.get_pitch();
  uchar *dest_data = (uchar *)dest.get_data();
  dest_data += pos_dst.x*OPP;
  dest_data += pos_dst.y*source_pitch;

  // Sauve les données
  uchar *sauve_source_data = source_data;
  uchar *sauve_dest_data = dest_data;

  for (uint dy=0; dy < haut; ++dy)
  {
    memcpy (dest_data, source_data, larg*OPP);

    // Passe à la ligne suivante dans source
    sauve_source_data += source_pitch;
    source_data = sauve_source_data;

    // Passe à la ligne suivante dans dest
    sauve_dest_data += dest_pitch;
    dest_data = sauve_dest_data;
  }

  // Unlock datas
  dest.unlock();
  source.unlock();
*/
}

#endif
//-----------------------------------------------------------------------------

long Offset(long x, long larg)
{
  long off = x/larg;
  if (x < 0) off--;
  return off; 
}

//-----------------------------------------------------------------------------

ulong ArronditDefaut (long x, ulong larg)
{
  if ((x % larg) == 0) return x;
  if (x < 0)
    return x -larg - (x % (long)larg);
  else
    return x - (x % (long)larg);
}

//-----------------------------------------------------------------------------

#ifdef CL
void Tile::Dig (int x, int y, uint width, uint height, CL_PixelBuffer dig)
{
  int x_max_cell = x+width;
  int y_max_cell = y+height;

  // En dehors de l'écran ? Ne dessine rien !
  if ((int)larg < x) return;
  if ((int)haut < y) return;
  if (x_max_cell < 0) return;
  if (y_max_cell < 0) return;

#ifdef MSG_DBG_CLIPPING
  COUT_DBG << "--- Début creuse ---" << std::endl;
  COUT_DBG << "x,y=" << x <<","<<y
	   << ", taille=" << width <<","<<height
	   << std::endl;
#endif

  uint off_x, off_y;
  int creuse_x, creuse_y;
  if (0 <= x) {
    off_x = 0;
    creuse_x = x;
  } else {
    off_x = -x;
    creuse_x = 0;
  }
  if (0 <= y) {
    off_y = 0;
    creuse_y = y;
  } else {
    off_y = -y;
    creuse_y = 0;
  }

  // Calcule la coordonnée X dans la pièce et le numéro X de la pièce
  int piece_x;
  int x_max = ArronditDefaut(x_max_cell, larg_cell);
  int x_maxMAX = ArronditDefaut(larg-1, larg_cell);
  if (x_maxMAX < x_max) x_max = x_maxMAX;
  if (0 <= x) {
    piece_x = Offset(x,larg_cell);
    x = ArronditDefaut(x, larg_cell);
  } else {
    piece_x = 0;
    x = 0;
  }

  // Calcule la coordonnée Y dans la pièce et le numéro Y de la pièce
  int piece_y;
  int y_max = ArronditDefaut(y_max_cell, haut_cell);
  int y_maxMAX = ArronditDefaut(haut-1, haut_cell);
  if (y_maxMAX < y_max) y_max = y_maxMAX;
  if (0 <= y) {
    piece_y = Offset(y,haut_cell);
    y = ArronditDefaut(y, haut_cell);
  } else {
    piece_y = 0;
    y = 0;
  }

  int sauve_x = x;
  uint sauve_piece_x=piece_x;
  int sauve_creuse_x = creuse_x;
  int sauve_creuse_y = creuse_y;

  do
  {
    piece_x = sauve_piece_x;
    x = sauve_x;
    creuse_x = sauve_creuse_x;
    do
    {
      if ((0 <= x) && (x <= (int)larg) 
	  && (0 <= y) && (y <= (int)haut))
      {
#ifdef MSG_DBG_CLIPPING
	COUT_DBG << "x,y=" << x << "," << y
		 << "; x_max,y_max=" << x_max << "," << y_max
		 << std::endl;

	COUT_DBG << "creuse_x,creuse_y=" << creuse_x << "," << creuse_y
		 << "; x_max_cell,x_max_cell=" <<  x_max_cell
		 << "," << y_max_cell
		 << std::endl;
	COUT_DBG << "piece_x,piece_y=" << piece_x << "," << piece_y << std::endl;
#endif

	assert ((0 <= piece_x) && (piece_x < (int)nbr_cell_larg));
	assert ((0 <= piece_y) && (piece_y < (int)nbr_cell_haut));

	uint piece = piece_y*nbr_cell_larg +piece_x;
	assert (piece < nbr_cell);

	int creuse_larg=larg_cell;
	int creuse_haut=haut_cell;

	// Quand on est au milieu d'une cellule, tronque sur le bord
	if (x+(int)larg_cell<creuse_x+creuse_larg) 
	  creuse_larg = (x+(int)larg_cell) - creuse_x;
	if (y+(int)haut_cell<creuse_y+creuse_haut) 
	  creuse_haut = (y+(int)haut_cell) - creuse_y;

	// Quand on est arrive à la fin, tronque ce qui dépasse
	if (x_max_cell<creuse_x+creuse_larg) 
	  creuse_larg = x_max_cell - creuse_x;
	if (y_max_cell<creuse_y+creuse_haut) 
	  creuse_haut = y_max_cell - creuse_y;

	CL_Point pos_creuse (creuse_x - sauve_creuse_x +off_x,
			     creuse_y - sauve_creuse_y +off_y);
	CL_Point pos_cell (creuse_x-x,creuse_y-y);

#ifdef MSG_DBG_CLIPPING
	COUT_DBG << "pos_creuse = " << pos_creuse.x <<","<< pos_creuse.y
		 << ", pos_cell = "<<pos_cell.x <<","<< pos_cell.y
		 <<std::endl;
	COUT_DBG << "taille = " << creuse_larg << "x" << creuse_haut
		 <<std::endl;
#endif

	CreuseCellule (dig,
		       pos_creuse,
		       creuse_larg, creuse_haut,
		       item[piece],
		       pos_cell);
      }

      // Passe au cell suivant en X
      ++piece_x;
      x += larg_cell;
      if ((creuse_x % larg_cell) != 0) {
	creuse_x += larg_cell-(creuse_x % larg_cell);
      } else {
	creuse_x += larg_cell;
      }
    } while (x<=x_max);

    // Passe à la ligne suivante des cells
    ++piece_y;
    y += haut_cell;
    if ((creuse_y % haut_cell) != 0) {
      creuse_y += haut_cell-creuse_y % haut_cell;
    } else {
      creuse_y += haut_cell;
    }
  } while (y<=y_max);

#ifdef MSG_DBG_CLIPPING
  COUT_DBG << "--- Fin creuse ---" << std::endl;
#endif
}
#else // CL is not defined /////////////////////////////////////////////////

int clamp (const int val, const int min, const int max)
{   
   return ( val > max ) ? max : ( val < min ) ? min : val ;
}

void Tile::Dig (int ox, int oy, SDL_Surface *dig)
{  
   Rectanglei rect = Rectanglei( ox, oy, dig->w, dig->h); 

   int first_cell_x = clamp( ox/larg_cell,          0, nbr_cell_larg);
   int first_cell_y = clamp( oy/haut_cell,          0, nbr_cell_haut);
   int last_cell_x  = clamp( (ox+dig->w)/larg_cell, 0, nbr_cell_larg);
   int last_cell_y  = clamp( (oy+dig->h)/haut_cell, 0, nbr_cell_haut);
   
   for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
     for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++)
       {
	  // compute offset
	  
	  int offset_x = ox - cx * larg_cell;
	  int offset_y = oy - cy * haut_cell;
	  
	  item[cy*nbr_cell_larg+cx].Dig( offset_x, offset_y, dig);
       }
}
#endif

//-----------------------------------------------------------------------------

#ifdef CL
void Tile::LoadImage (CL_Surface &terrain)
{
  FreeMem();

  InitTile (256, 256, terrain.get_width(), terrain.get_height());
  assert (nbr_cell != 0);

  // Crée les canvas et les 'canvas_modifie'
  //item.clear(); // on a deja fait un FreeMem() non ?!?
  for (uint i=0; i<nbr_cell; ++i)
  {
    TileItem tmp_item(larg_cell, haut_cell);
    item.push_back(tmp_item);
  }

  // Copie les données de la carte dans les 'canvas'
  CL_PixelBuffer data = terrain.get_pixeldata();
  data.lock(); 
  for (uint iy=0; iy<nbr_cell_haut; ++iy)
  {
    CL_Point pos_src;
    CL_Point pos_dst(0,0);
    for (uint ix=0; ix<nbr_cell_larg; ++ix)
    {
      uint piece = iy*nbr_cell_larg +ix;
      pos_src.x = ix*larg_cell;
      pos_src.y = iy*haut_cell;

      CopieCell(data, pos_src, larg_cell, haut_cell, item[piece], pos_dst);
    }
  }
  data.unlock();
}
#else
void Tile::LoadImage (SDL_Surface *terrain)
{
  FreeMem();

  InitTile (256, 256, terrain->w, terrain->h);
  assert (nbr_cell != 0);

  // Crée les canvas et les 'canvas_modifie'
  //item.clear(); // on a deja fait un FreeMem() non ?!?
  for (uint i=0; i<nbr_cell; ++i)
  {
    TileItem tmp_item(larg_cell, haut_cell);
    item.push_back(tmp_item);
  }
   
   // Copie les données de la carte dans les 'canvas'
   for ( unsigned int iy = 0 ; iy < nbr_cell_haut ; iy++ )
     {
	for ( unsigned int ix = 0 ; ix < nbr_cell_larg ; ix++)
	  {
	     unsigned int piece = iy * nbr_cell_larg + ix;

	     SDL_Rect sr = {ix*larg_cell,iy*haut_cell,larg_cell,haut_cell};
	     SDL_Rect dr = {0,0,larg_cell,haut_cell};
	     
	     SDL_SetAlpha( terrain, 0, 0);
	     SDL_BlitSurface( terrain, &sr, item[piece].get_surface(), &dr);
	     
	     // TODO  CopieCell(terrain, pos_src, larg_cell, haut_cell, item[piece], pos_dst);	     
	  }	
     }
}

#endif
//-----------------------------------------------------------------------------

#ifdef CL
// Lit la valeur alpha du pixel (x,y)
uchar Tile::GetAlpha (int x, int y)
{
  uint piece_x, piece_y, p_num;
  uchar *data;

  // Jamais de lecture en dehors de la taille
  assert ((0 <= x) && (x <= (int)larg));
  assert ((0 <= y) && (y <= (int)haut));

  piece_x = x/larg_cell;
  piece_y = y/haut_cell;
  p_num = (piece_y*nbr_cell_larg) + piece_x;
  x %= larg_cell;
  y %= haut_cell;

  data = item[p_num].get_data();
  data += y*item[p_num].get_pixelbuffer().get_pitch() + x*OPP;
  data += 3;

  return *data;
}
#else
uchar Tile::GetAlpha (const int x, const int y) const
{
   if ( x < 0 || x >= larg || y < 0 || y >= haut )
     return 0;
   
   unsigned int piece_x = x / larg_cell;
   unsigned int piece_y = y / haut_cell;
   
   return item[piece_y*nbr_cell_larg+piece_x].GetAlpha( x%larg_cell, y%haut_cell);
}

// TODO
#endif
//-----------------------------------------------------------------------------
#ifdef CL
void Tile::DrawTile()
{
  uint index = 0;
  uint x,y;

  y = 0;
  for (uint iy=0; iy<nbr_cell_haut; iy++)
  {
    x = 0;
    for (uint ix=0; ix<nbr_cell_larg; ix++)
    {
      item[index].get_surface().draw (x, y);
      index++;
      x += larg_cell;
    }
    y += haut_cell;
  }

#ifdef DESSINE_BORDURE_CANVAS
  // Cadrille le terrain avec des lignes rouges
#define COUL CL_Color::red
  y = 0;
  for (uint iy=0; iy<nbr_cell_haut; iy++)
  {
    x = 0;
    for (uint ix=0; ix<nbr_cell_larg; ix++)
    {
      CL_Display::draw_line (x, 0, x, haut, COUL);
      x += larg_cell;
    }
    CL_Display::draw_line (0, y, larg, y, COUL);
    y += haut_cell;
  }
#endif
}
#else
void Tile::DrawTile()
{
   uint index = 0;
   uint x = 0;
   uint y = 0;

   int ox = (int)FOND_X-camera.GetX(); 
   int oy = (int)FOND_Y-camera.GetY(); // needed for differential scrolling

   
   
   for (uint iy=0; iy<nbr_cell_haut; iy++)
     {
	x = 0;
	for (uint ix=0; ix<nbr_cell_larg; ix++)
	  {
	     SDL_Surface *s = item[index].get_surface();
	     SDL_Rect dr = {x+ox,y+oy,s->w,s->h};
	     SDL_BlitSurface( s, NULL, app.sdlwindow, &dr);

	     index++;
	     x += larg_cell;	  
	  }
	y += haut_cell;
     }   
}
#endif
//-----------------------------------------------------------------------------
