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
#include <iostream>
#include <SDL.h>
//#include "../graphic/graphism.h"
#include "../tool/Point.h"
#include "../include/app.h"
#include "../map/camera.h"
//#include "../include/constant.h"
#include "../game/config.h"

using namespace Wormux;

// === Implemenation of TileItem_Software_ALpha ==============================

TileItem_AlphaSoftware::TileItem_AlphaSoftware (unsigned int width, unsigned int height)
{
   m_width = width;
   m_height = height;

   m_surface = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, m_width, m_height, 
				     32, // force to 32 bits per pixel
				     0x000000ff,  // red mask
				     0x0000ff00,  // green mask
				     0x00ff0000,  // blue mask
				     0xff000000); // alpha mask
}

TileItem_AlphaSoftware::TileItem_AlphaSoftware (const TileItem_AlphaSoftware &copy)
{
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, m_width, m_height, 
				     32, // force to 32 bits per pixel
				     0x000000ff,  // red mask
				     0x0000ff00,  // green mask
				     0x00ff0000,  // blue mask
				     0xff000000); // alpha mask

   SDL_Rect dest_rect = {0,0, copy.m_surface->w, copy.m_surface->h};
   SDL_BlitSurface( copy.m_surface, NULL, m_surface, &dest_rect);
}

TileItem_AlphaSoftware::~TileItem_AlphaSoftware ()
{
   SDL_FreeSurface( m_surface);
}

unsigned char TileItem_AlphaSoftware::GetAlpha(const int x,const int y) const
{
   return *(((unsigned char *)m_surface->pixels) + y*m_surface->pitch + x * 4 + 3);
}

void TileItem_AlphaSoftware::Dig( int ox, int oy, SDL_Surface *dig)
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

SDL_Surface *TileItem_AlphaSoftware::GetSurface()
{
   return m_surface;
}

void TileItem_AlphaSoftware::SyncBuffer()
{
   // nothing to do
}


// === Implemenation of TileItem_ALphaHardware ============================

TileItem_AlphaHardware::TileItem_AlphaHardware (unsigned int width, unsigned int height)
{
   m_width = width;
   m_height = height;

   m_surface = SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, m_width, m_height, 
				     32, // force to 32 bits per pixel
				     0x000000ff,  // red mask
				     0x0000ff00,  // green mask
				     0x00ff0000,  // blue mask
				     0xff000000); // alpha mask

   m_buffer = new unsigned char[m_height*m_width];
}

TileItem_AlphaHardware::TileItem_AlphaHardware (const TileItem_AlphaHardware &copy)
{
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface = SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, m_width, m_height, 
				     32, // force to 32 bits per pixel
				     0x000000ff,  // red mask
				     0x0000ff00,  // green mask
				     0x00ff0000,  // blue mask
				     0xff000000); // alpha mask

   SDL_Rect dest_rect = {0,0, copy.m_surface->w, copy.m_surface->h};
   SDL_BlitSurface( copy.m_surface, NULL, m_surface, &dest_rect);

   m_buffer = new unsigned char[m_height*m_width];
   memcpy( copy.m_buffer, m_buffer, m_height*m_width*sizeof( unsigned char));
}

TileItem_AlphaHardware::~TileItem_AlphaHardware ()
{
   SDL_FreeSurface( m_surface);
   delete[] m_buffer;
}

unsigned char TileItem_AlphaHardware::GetAlpha(const int x,const int y) const
{
   return *(m_buffer+y*m_width+x);
}

void TileItem_AlphaHardware::Dig( int ox, int oy, SDL_Surface *dig)
{
   int starting_x = ox >= 0 ? ox : 0;   
   int starting_y = oy >= 0 ? oy : 0; 
   int ending_x = ox+dig->w <= m_surface->w ? ox+dig->w : m_surface->w;
   int ending_y = oy+dig->h <= m_surface->h ? oy+dig->h : m_surface->h;
   
   SDL_LockSurface( m_surface);
   
   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       {
	  if ( *(((unsigned char *)dig->pixels) + (py-oy)*dig->pitch + (px-ox) * 4 + 3) != 0)	    
	    {
	       *(((unsigned char *)m_surface->pixels) + py*m_surface->pitch + px * 4 + 3) = 0;
	       m_buffer[py*m_width+px] = 0;
	    }
       }
   
   SDL_UnlockSurface( m_surface);
}

SDL_Surface *TileItem_AlphaHardware::GetSurface()
{
   return m_surface;
}

void TileItem_AlphaHardware::SyncBuffer()
{
   SDL_LockSurface( m_surface);
   
   for ( unsigned int y = 0 ; y < m_height ; y++)  
     for ( unsigned int x = 0 ; x < m_width ; x++)
       {       
	  m_buffer[y*m_width+x] = *(((unsigned char *)m_surface->pixels) + y*m_surface->pitch + x * 4 + 3);
       }
   
   SDL_UnlockSurface( m_surface);
}

// === Implemenation of TileItem_Software_Colorkey  ==============================

TileItem_ColorkeySoftware::TileItem_ColorkeySoftware (unsigned int width, unsigned int height)
{
   m_width = width;
   m_height = height;

   m_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, m_width, m_height, 
				     32, // force to 32 bits per pixel
				     0x000000ff,  // red mask
				     0x0000ff00,  // green mask
				     0x00ff0000,  // blue mask
				     0xff000000); // alpha mask

   SDL_SetAlpha (m_surface,0,0);
   SDL_SetColorKey (m_surface, SDL_SRCCOLORKEY/*|SDL_RLEACCEL*/, SDL_MapRGBA (m_surface->format,0,0,0,0));
   m_buffer = new unsigned char[m_height*m_width];
}

TileItem_ColorkeySoftware::TileItem_ColorkeySoftware (const TileItem_ColorkeySoftware &copy)
{
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, m_width, m_height, 
				     32, // force to 32 bits per pixel
				     0x000000ff,  // red mask
				     0x0000ff00,  // green mask
				     0x00ff0000,  // blue mask
				     0x00000000); // alpha mask

   SDL_Rect dest_rect = {0,0, copy.m_surface->w, copy.m_surface->h};
   SDL_BlitSurface( copy.m_surface, NULL, m_surface, &dest_rect);
   SDL_SetAlpha (m_surface,0,0);
   SDL_SetColorKey (m_surface, SDL_SRCCOLORKEY/*|SDL_RLEACCEL*/, SDL_MapRGBA (m_surface->format,0,0,0,0));

   m_buffer = new unsigned char[m_height*m_width];
   memcpy( copy.m_buffer, m_buffer, m_height*m_width*sizeof( unsigned char));
}

TileItem_ColorkeySoftware::~TileItem_ColorkeySoftware ()
{
   SDL_FreeSurface( m_surface);
   delete[] m_buffer;
}

unsigned char TileItem_ColorkeySoftware::GetAlpha(const int x,const int y) const
{
   return m_buffer[y*m_width+x];
}

void TileItem_ColorkeySoftware::Dig( int ox, int oy, SDL_Surface *dig)
{
   int starting_x = ox >= 0 ? ox : 0;   
   int starting_y = oy >= 0 ? oy : 0; 
   int ending_x = ox+dig->w <= m_surface->w ? ox+dig->w : m_surface->w;
   int ending_y = oy+dig->h <= m_surface->h ? oy+dig->h : m_surface->h;

   Uint32 transparent_color = SDL_MapRGBA (m_surface->format,0,0,0,0);

   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       {
	  if ( *(((unsigned char *)dig->pixels) + (py-oy)*dig->pitch + (px-ox) * 4 + 3) != 0)
	    { 
	       
	       *(Uint32 *)((unsigned char *)m_surface->pixels+py*m_surface->pitch+px*4) = transparent_color;
	       m_buffer[py*m_width+px] = 0;
	    }
       }
}

SDL_Surface *TileItem_ColorkeySoftware::GetSurface()
{
   return m_surface;
}

void TileItem_ColorkeySoftware::SyncBuffer()
{
   SDL_LockSurface( m_surface);
   
   for ( unsigned int y = 0 ; y < m_height ; y++)  
     for ( unsigned int x = 0 ; x < m_width ; x++)
       {       
	  m_buffer[y*m_width+x] = *(((unsigned char *)m_surface->pixels) + y*m_surface->pitch + x * 4 + 3);
       }
   
   SDL_UnlockSurface( m_surface);
}



// =======================================================================
// = Tile implementation                                                 =
// =======================================================================

Tile::Tile()
{
  larg_cell = 128;
  haut_cell = 128;
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

void Tile::InitTile (unsigned int plarg_cell, unsigned int phaut_cell, 
		     unsigned int plarg, unsigned int phaut)
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

int clamp (const int val, const int min, const int max)
{   
   return ( val > max ) ? max : ( val < min ) ? min : val ;
}

void Tile::Dig (int ox, int oy, SDL_Surface *dig)
{  
   Rectanglei rect = Rectanglei( ox, oy, dig->w, dig->h); 

   int first_cell_x = clamp( ox/larg_cell,          0, nbr_cell_larg-1);
   int first_cell_y = clamp( oy/haut_cell,          0, nbr_cell_haut-1);
   int last_cell_x  = clamp( (ox+dig->w)/larg_cell, 0, nbr_cell_larg-1);
   int last_cell_y  = clamp( (oy+dig->h)/haut_cell, 0, nbr_cell_haut-1);
   
   for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
     for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++)
       {
	  // compute offset
	  
	  int offset_x = ox - cx * larg_cell;
	  int offset_y = oy - cy * haut_cell;
	  
	  item[cy*nbr_cell_larg+cx]->Dig( offset_x, offset_y, dig);
       }
}

void Tile::LoadImage (SDL_Surface *terrain)
{
  FreeMem();

  InitTile (256, 256, terrain->w, terrain->h);
  assert (nbr_cell != 0);

  // Create the TileItem objects
 
  for (uint i=0; i<nbr_cell; ++i)
  {
     if ( config.transparency == Config::ALPHA )
       item.push_back ( new TileItem_AlphaSoftware(larg_cell, haut_cell));
     else
       item.push_back ( new TileItem_ColorkeySoftware(larg_cell, haut_cell));
  }
   
   // Fill the TileItem objects
   
   for ( unsigned int iy = 0 ; iy < nbr_cell_haut ; iy++ )
     {
	for ( unsigned int ix = 0 ; ix < nbr_cell_larg ; ix++)
	  {
	     unsigned int piece = iy * nbr_cell_larg + ix;

	     SDL_Rect sr = {ix*larg_cell,iy*haut_cell,larg_cell,haut_cell};
	     SDL_Rect dr = {0,0,larg_cell,haut_cell};
	     
	     SDL_SetAlpha( terrain, 0, 0);
	     SDL_BlitSurface( terrain, &sr, item[piece]->GetSurface(), &dr);
	     
	     item[piece]->SyncBuffer();
	  }	
     }
}

uchar Tile::GetAlpha (const int x, const int y) const
{
   //if ( x < 0 || x >= larg || y < 0 || y >= haut )
   //  return 0;
   
   return item[y/haut_cell*nbr_cell_larg+x/larg_cell]->GetAlpha( x%larg_cell, y%haut_cell);
}

void Tile::DrawTile()
{
   int ox = camera.GetX();
   int oy = camera.GetY();
   
   int first_cell_x = clamp( ox/larg_cell,                      0, nbr_cell_larg-1);
   int first_cell_y = clamp( oy/haut_cell,                      0, nbr_cell_haut-1);
   int last_cell_x  = clamp( (ox+camera.GetWidth())/larg_cell,  0, nbr_cell_larg-1);
   int last_cell_y  = clamp( (oy+camera.GetHeight())/haut_cell, 0, nbr_cell_haut-1);

   for ( int iy = first_cell_y ; iy <= last_cell_y ; iy++ )
     for ( int ix = first_cell_x ; ix <= last_cell_x ; ix++)
       {
	  SDL_Surface *i_surface = item[iy*nbr_cell_larg+ix]->GetSurface();
	  
	  SDL_Rect dr = {ix*larg_cell-camera.GetX(),iy*haut_cell-camera.GetY(),i_surface->w,i_surface->h};
	  SDL_BlitSurface (i_surface, NULL, app.sdlwindow, &dr); 	     
       }
}   


