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
#include <SDL_endian.h>
#include "../game/config.h"
#include "../include/app.h"
#include "../map/camera.h"
#include "../tool/error.h"
#include "../tool/point.h"


const int cell_width = 128;
const int cell_height = 128;

class TileItem
{
public:
  TileItem () {};
  virtual ~TileItem () {};
   
  bool IsEmpty ();
  virtual unsigned char GetAlpha (const int x,const int y) = 0;
  virtual void Dig (int ox, int oy, Surface& dig) = 0;
  virtual Surface GetSurface () = 0;
  virtual void SyncBuffer () = 0; // (if needed)
  virtual void Draw (const int x,const int y);
};

class TileItem_Empty : public TileItem
{
public:
  TileItem_Empty () {};
  ~TileItem_Empty () {};
   
  unsigned char GetAlpha (const int x,const int y) {return 0;};
  void Dig (int ox, int oy, Surface& dig) {};
  Surface GetSurface() {return *new Surface();};
  void SyncBuffer() {};
  void Draw(const int x,const int y) {};
};

class TileItem_AlphaSoftware : public TileItem
{
public:
  TileItem_AlphaSoftware (unsigned int width, unsigned int height);
  TileItem_AlphaSoftware (const TileItem_AlphaSoftware &copy);
  ~TileItem_AlphaSoftware ();
    
  unsigned char GetAlpha (const int x, const int y);
  Surface GetSurface();
  void Dig(int ox, int oy, Surface& dig);
  void SyncBuffer();
     
private:
  unsigned char (TileItem_AlphaSoftware::*_GetAlpha)(const int x, const int y) ;
  unsigned char GetAlpha_Index0 (const int x, const int y);
  inline unsigned char GetAlpha_Index3 (const int x, const int y);
  inline unsigned char GetAlpha_Generic (const int x, const int y);

  unsigned int m_width, m_height;
  Surface m_surface;
};  

class TileItem_AlphaHardware : public TileItem
{
public:
  TileItem_AlphaHardware (unsigned int width, unsigned int height);
  TileItem_AlphaHardware (const TileItem_AlphaHardware &copy);
  ~TileItem_AlphaHardware ();
    
  unsigned char GetAlpha (const int x, const int y);
  Surface GetSurface();
  void Dig (int ox, int oy, Surface& dig);
  void SyncBuffer ();
   
private:
  unsigned int m_width, m_height;
  Surface m_surface;
  unsigned char *m_buffer;
};  

class TileItem_ColorkeySoftware : public TileItem
{
public:
  TileItem_ColorkeySoftware (unsigned int width, unsigned int height);
  TileItem_ColorkeySoftware (const TileItem_ColorkeySoftware &copy);
  ~TileItem_ColorkeySoftware ();
    
  unsigned char GetAlpha (const int x, const int y);
  Surface GetSurface ();
  void Dig(int ox, int oy, Surface& dig);
  void SyncBuffer();
 
private:
  unsigned int m_width, m_height;
  Surface m_surface;
  unsigned char *m_buffer;
};  


// === Common to all TileItem_* except TileItem_Emtpy ==============================

void TileItem::Draw(const int x,const int y)
{
  Surface i_surface = GetSurface();
  SDL_Rect ds = {0,0,cell_width,cell_height};
  SDL_Rect dr = {x*cell_width-camera.GetX(),y*cell_height-camera.GetY(),cell_width,cell_height};
  app.video.window.Blit(i_surface, &ds, &dr); 	     
}

bool TileItem::IsEmpty()
{
  for ( int x = 0 ; x < cell_width ; x++)
     for ( int y = 0 ; y < cell_height ; y++)
	  if ( GetAlpha (x,y) == 255)
	    return false;
   
   return true;
}


// === Implemenation of TileItem_Software_ALpha ==============================

TileItem_AlphaSoftware::TileItem_AlphaSoftware (unsigned int width, unsigned int height)
{
   m_width = width;
   m_height = height;

   Surface _m_surface = Surface(m_width, m_height, SDL_SWSURFACE|SDL_SRCALPHA, true);
	
   m_surface = _m_surface.DisplayFormatAlpha();

   _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Generic;
   if ( m_surface.GetSurface()->format->BytesPerPixel == 4)
   {
       if (m_surface.GetSurface()->format->Amask == 0x000000ff) {
           if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
               _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index0;
           else
               _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index3;
       } else if (m_surface.GetSurface()->format->Amask == 0xff000000) {
           if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
               _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index3;
           else
               _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index0;
       }
   }
}

TileItem_AlphaSoftware::TileItem_AlphaSoftware (const TileItem_AlphaSoftware &copy)
{
   Error( "TileItem_Alphasoftware: copy constructor not implemented");
}

TileItem_AlphaSoftware::~TileItem_AlphaSoftware ()
{
}

unsigned char TileItem_AlphaSoftware::GetAlpha(const int x,const int y)
{
   return (this->*_GetAlpha)( x, y);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Index0 (const int x, const int y)
{
   return *(((unsigned char *)m_surface.GetSurface()->pixels) + y*m_surface.GetSurface()->pitch + x * 4 + 0);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Index3 (const int x, const int y) 
{
   return *(((unsigned char *)m_surface.GetSurface()->pixels) + y*m_surface.GetSurface()->pitch + x * 4 + 3);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Generic (const int x, const int y)
{
   unsigned char r, g, b, a;
 
   Uint32 pixel = *(Uint32 *)((unsigned char *)m_surface.GetSurface()->pixels+y*m_surface.GetSurface()->pitch+x*m_surface.GetSurface()->format->BytesPerPixel); 
   SDL_GetRGBA( pixel, m_surface.GetSurface()->format, &r, &g, &b, &a);
   
   return a;
}

 
void TileItem_AlphaSoftware::Dig( int ox, int oy, Surface& dig){
   int starting_x = ox >= 0 ? ox : 0;   
   int starting_y = oy >= 0 ? oy : 0; 
   int ending_x = ox+dig.GetWidth() <= m_surface.GetWidth() ? ox+dig.GetWidth() : m_surface.GetWidth();
   int ending_y = oy+dig.GetHeight() <= m_surface.GetHeight() ? oy+dig.GetHeight() : m_surface.GetHeight();
   
   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       {
	  if ( *(((unsigned char *)dig.GetSurface()->pixels) + (py-oy)*dig.GetSurface()->pitch + (px-ox) * 4 + 3) != 0)
	    
	    *(((unsigned char *)m_surface.GetSurface()->pixels) + py*m_surface.GetSurface()->pitch + px * 4 + 3) = 0;
       }
}

Surface TileItem_AlphaSoftware::GetSurface(){
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

   m_surface.NewSurface(m_width, m_height, SDL_HWSURFACE|SDL_SRCALPHA, true);

   m_buffer = new unsigned char[m_height*m_width];
}

TileItem_AlphaHardware::TileItem_AlphaHardware (const TileItem_AlphaHardware &copy)
{
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface.NewSurface(m_width, m_height, SDL_HWSURFACE|SDL_SRCALPHA, true);
   SDL_Rect dest_rect = {0,0, copy.m_surface.GetWidth(), copy.m_surface.GetHeight()};
   m_surface.Blit( copy.m_surface, NULL, &dest_rect);

   m_buffer = new unsigned char[m_height*m_width];
   memcpy( copy.m_buffer, m_buffer, m_height*m_width*sizeof( unsigned char));
}

TileItem_AlphaHardware::~TileItem_AlphaHardware ()
{
   delete[] m_buffer;
}

unsigned char TileItem_AlphaHardware::GetAlpha(const int x,const int y)
{
   return *(m_buffer+y*m_width+x);
}

void TileItem_AlphaHardware::Dig( int ox, int oy, Surface& dig)
{
   int starting_x = ox >= 0 ? ox : 0;   
   int starting_y = oy >= 0 ? oy : 0; 
   int ending_x = ox+dig.GetWidth() <= m_surface.GetWidth() ? ox+dig.GetWidth() : m_surface.GetWidth();
   int ending_y = oy+dig.GetHeight() <= m_surface.GetHeight() ? oy+dig.GetHeight() : m_surface.GetHeight();
   
   m_surface.Lock();
   
   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       {
	  if ( *(((unsigned char *)dig.GetSurface()->pixels) + (py-oy)*dig.GetSurface()->pitch + (px-ox) * 4 + 3) != 0)	    
	    {
	       *(((unsigned char *)m_surface.GetSurface()->pixels) + py*m_surface.GetSurface()->pitch + px * 4 + 3) = 0;
	       m_buffer[py*m_width+px] = 0;
	    }
       }
   
   m_surface.Unlock();
}

Surface TileItem_AlphaHardware::GetSurface()
{
   return m_surface;
}

void TileItem_AlphaHardware::SyncBuffer()
{
   m_surface.Lock();
   
   for ( unsigned int y = 0 ; y < m_height ; y++)  
     for ( unsigned int x = 0 ; x < m_width ; x++)
	  m_buffer[y*m_width+x] = *(((unsigned char *)m_surface.GetSurface()->pixels) + y*m_surface.GetSurface()->pitch + x * 4 + 3);
   
   m_surface.Unlock();
}

// === Implemenation of TileItem_Software_Colorkey  ==============================

TileItem_ColorkeySoftware::TileItem_ColorkeySoftware (unsigned int width, unsigned int height)
{
   m_width = width;
   m_height = height;

   m_surface.NewSurface(m_width, m_height, SDL_SWSURFACE);
   m_surface.SetAlpha(0, 0);
   m_surface.SetColorKey( SDL_SRCCOLORKEY, 0, 0, 0, 0);
   m_buffer = new unsigned char[m_height*m_width];
}

TileItem_ColorkeySoftware::TileItem_ColorkeySoftware (const TileItem_ColorkeySoftware &copy)
{
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface.NewSurface(m_width, m_height, SDL_SWSURFACE);
   SDL_Rect dest_rect = {0, 0, copy.m_surface.GetWidth(), copy.m_surface.GetHeight()};
   m_surface.Blit( copy.m_surface, NULL, &dest_rect);
   m_surface.SetAlpha(0, 0);
   m_surface.SetColorKey(SDL_SRCCOLORKEY, 0, 0, 0, 0);

   m_buffer = new unsigned char[m_height*m_width];
   memcpy( copy.m_buffer, m_buffer, m_height*m_width*sizeof( unsigned char));
}

TileItem_ColorkeySoftware::~TileItem_ColorkeySoftware ()
{
   delete[] m_buffer;
}

unsigned char TileItem_ColorkeySoftware::GetAlpha(const int x,const int y) 
{
   return m_buffer[y*m_width+x];
}

void TileItem_ColorkeySoftware::Dig( int ox, int oy, Surface& dig)
{
   int starting_x = ox >= 0 ? ox : 0;   
   int starting_y = oy >= 0 ? oy : 0; 
   int ending_x = ox+dig.GetWidth() <= m_surface.GetWidth() ? ox+dig.GetWidth() : m_surface.GetWidth();
   int ending_y = oy+dig.GetHeight() <= m_surface.GetHeight() ? oy+dig.GetHeight() : m_surface.GetHeight();

   Uint32 transparent_color = m_surface.MapRGBA(0, 0, 0, 0);

   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       {
	  if ( *(((unsigned char *)dig.GetSurface()->pixels) + (py-oy)*dig.GetSurface()->pitch + (px-ox) * 4 + 3) != 0)
	    { 
	       
	       *(Uint32 *)((unsigned char *)m_surface.GetSurface()->pixels+py*m_surface.GetSurface()->pitch+px*4) = transparent_color;
	       m_buffer[py*m_width+px] = 0;
	    }
       }
}

Surface TileItem_ColorkeySoftware::GetSurface()
{
   return m_surface;
}

void TileItem_ColorkeySoftware::SyncBuffer()
{
   m_surface.Lock();
   
   for ( unsigned int y = 0 ; y < m_height ; y++)  
     for ( unsigned int x = 0 ; x < m_width ; x++)
	  m_buffer[y*m_width+x] = *(((unsigned char *)m_surface.GetSurface()->pixels) + y*m_surface.GetSurface()->pitch + x * 4 + 3);
   
   m_surface.Unlock();
}

// = Tile implementation                                                 =

Tile::Tile()
{
}

void Tile::FreeMem()
{
  for (uint i=0; i<nbr_cell; ++i)
    delete item[i];
  nbr_cell = 0;
  item.clear();
}

Tile::~Tile()
{ 
  FreeMem();
}

void Tile::InitTile (unsigned int width, unsigned int height)
{
  nbr_cell_width = width / cell_width;
  if ((width % cell_width) != 0) nbr_cell_width++;
  nbr_cell_height = height / cell_height;
  if ((height % cell_height) != 0) nbr_cell_height++;

  this->width = width;
  this->height = height;

  nbr_cell = nbr_cell_width * nbr_cell_height; 

#ifdef MSG_DBG_CLIPPING
  COUT_DBG << "Initialisation : " << width << "x" << height
	   << " tiled in " 
	   << nbr_cell_width << "x" << nbr_cell_height
           << " cellules of "
	   << cell_width << "x" << cell_height << std::endl;
#endif
}

int clamp (const int val, const int min, const int max)
{   
   return ( val > max ) ? max : ( val < min ) ? min : val ;
}

void Tile::Dig (int ox, int oy, Surface& dig)
{  
   Rectanglei rect = Rectanglei( ox, oy, dig.GetWidth(), dig.GetHeight()); 

   int first_cell_x = clamp( ox/cell_width,          0, nbr_cell_width-1);
   int first_cell_y = clamp( oy/cell_height,          0, nbr_cell_height-1);
   int last_cell_x  = clamp( (ox+dig.GetWidth())/cell_width, 0, nbr_cell_width-1);
   int last_cell_y  = clamp( (oy+dig.GetHeight())/cell_height, 0, nbr_cell_height-1);
   
   for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
     for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++)
       {
	  // compute offset
	  
	  int offset_x = ox - cx * cell_width;
	  int offset_y = oy - cy * cell_height;
	  
	  item[cy*nbr_cell_width+cx]->Dig( offset_x, offset_y, dig);
       }
}

void Tile::LoadImage (Surface& terrain)
{
  FreeMem();

  InitTile (terrain.GetWidth(), terrain.GetHeight());
  assert (nbr_cell != 0);

  // Create the TileItem objects
  for (uint i=0; i<nbr_cell; ++i)
     if ( config.transparency == Config::ALPHA )
       item.push_back ( new TileItem_AlphaSoftware(cell_width, cell_height));
     else
       item.push_back ( new TileItem_ColorkeySoftware(cell_width, cell_height));
   
   // Fill the TileItem objects
   for ( unsigned int iy = 0 ; iy < nbr_cell_height ; iy++ )
     {
	for ( unsigned int ix = 0 ; ix < nbr_cell_width ; ix++)
	  {
	     unsigned int piece = iy * nbr_cell_width + ix;

	     SDL_Rect sr = {ix*cell_width,iy*cell_height,cell_width,cell_height};
	     SDL_Rect dr = {0, 0, cell_width, cell_height};
	     
	     terrain.SetAlpha(0, 0);
	     item[piece]->GetSurface().Blit( terrain, &sr, &dr);
	     item[piece]->SyncBuffer();
	  }	
     }

   // Replace transparent tiles by TileItem_Empty tiles

  uint freed = 0;

  for (uint i=0; i<nbr_cell; ++i)
  {
    if(item[i]->IsEmpty())
    {
      freed++;
      delete item[i];
      item[i] = (TileItem*)new TileItem_Empty;
    }
  }
}

uchar Tile::GetAlpha (const int x, const int y) const
{
   //if ( x < 0 || x >= larg || y < 0 || y >= haut )
   //  return 0;
   
   return item[y/cell_height*nbr_cell_width+x/cell_width]->GetAlpha( x%cell_width, y%cell_height);
}

void Tile::DrawTile() const
{
   int ox = camera.GetX();
   int oy = camera.GetY();
   
   int first_cell_x = clamp( ox/cell_width,                      0, nbr_cell_width-1);
   int first_cell_y = clamp( oy/cell_height,                      0, nbr_cell_height-1);
   int last_cell_x  = clamp( (ox+camera.GetWidth())/cell_width,  0, nbr_cell_width-1);
   int last_cell_y  = clamp( (oy+camera.GetHeight())/cell_height, 0, nbr_cell_height-1);  

   for ( int iy = first_cell_y ; iy <= last_cell_y ; iy++ )
     for ( int ix = first_cell_x ; ix <= last_cell_x ; ix++)
       {
	 item[iy*nbr_cell_width+ix]->Draw(ix,iy);
       }
}   

void Tile::DrawTile_Clipped( Rectanglei clip_r_world) const
{
   // Select only the items that are under the clip area
   int first_cell_x = clamp( (clip_r_world.x)/cell_width,                0, nbr_cell_width-1);
   int first_cell_y = clamp( (clip_r_world.y)/cell_height,                0, nbr_cell_height-1);
   int last_cell_x  = clamp( (clip_r_world.x+clip_r_world.w +1)/cell_width, 0, nbr_cell_width-1);
   int last_cell_y  = clamp( (clip_r_world.y+clip_r_world.h +1)/cell_height, 0, nbr_cell_height-1);

   for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
     for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++)
       {
	  // For all selected items, clip source and destination
	  // blitting rectangles 
	  int dest_x = cx * cell_width;  
	  int dest_y = cy * cell_height;
	  int dest_w = cell_width;
	  int dest_h = cell_height;
	  int src_x = 0;
	  int src_y = 0;
	  
	  if ( dest_x < clip_r_world.x ) // left clipping
	    {
	       src_x  += clip_r_world.x - dest_x;
	       dest_w -= clip_r_world.x - dest_x;
	       dest_x  = clip_r_world.x;
	    }
	  if ( dest_y < clip_r_world.y ) // top clipping
	    {
	       src_y  += clip_r_world.y - dest_y;
	       dest_h -= clip_r_world.y - dest_y;
	       dest_y  = clip_r_world.y;
	    }
	  if ( dest_x + dest_w > clip_r_world.x + clip_r_world.w +1) // right clipping
	    {
	       dest_w -= ( dest_x + dest_w ) - ( clip_r_world.x + clip_r_world.w +1);
	    }
	  if ( dest_y + dest_h > clip_r_world.y + clip_r_world.h +1) // bottom clipping
	    {
	       dest_h -= ( dest_y + dest_h ) - ( clip_r_world.y + clip_r_world.h +1);
	    }
	  
	  
	  SDL_Rect sr = { src_x, src_y, dest_w, dest_h};
	       
	  // Decall the destination rectangle along the camera offset
	  SDL_Rect dr = { dest_x-camera.GetX(), dest_y-camera.GetY(), dest_w, dest_h};
	  
	  app.video.window.Blit(item[cy*nbr_cell_width+cx]->GetSurface(), &sr, &dr);
       }
}   
