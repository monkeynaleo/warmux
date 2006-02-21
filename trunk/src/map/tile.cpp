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

const Point2i CELL_SIZE(128, 128);

class TileItem
{
public:
  TileItem () {};
  virtual ~TileItem () {};
   
  bool IsEmpty ();
  virtual unsigned char GetAlpha (const int x,const int y) = 0;
  virtual void Dig(const Point2i &position, Surface& dig) = 0;
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
  void Dig(const Point2i &position, Surface& dig) {};
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
  void Dig(const Point2i &position, Surface& dig);
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
  void Dig(const Point2i &position, Surface& dig);
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
  void Dig(const Point2i &position, Surface& dig);
  void SyncBuffer();
 
private:
  unsigned int m_width, m_height;
  Surface m_surface;
  unsigned char *m_buffer;
};  


// === Common to all TileItem_* except TileItem_Emtpy ==============================

void TileItem::Draw(const int x,const int y){
  Surface i_surface = GetSurface();
  Rectanglei ds( Point2i(0, 0), CELL_SIZE);
  Point2i dp( Point2i(x, y) * CELL_SIZE - camera.GetPosition() );
  app.video.window.Blit(i_surface, ds, dp);
}

bool TileItem::IsEmpty(){
  for ( int x = 0 ; x < CELL_SIZE.x ; x++)
     for ( int y = 0 ; y < CELL_SIZE.y ; y++)
       if ( GetAlpha (x,y) == 255)
         return false;
   
   return true;
}


// === Implemenation of TileItem_Software_ALpha ==============================

TileItem_AlphaSoftware::TileItem_AlphaSoftware (unsigned int width, unsigned int height){
   m_width = width;
   m_height = height;

   Surface _m_surface = Surface(m_width, m_height, SDL_SWSURFACE|SDL_SRCALPHA, true);
	
   m_surface = _m_surface.DisplayFormatAlpha();

   _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Generic;
   if ( m_surface.GetBytesPerPixel() == 4)
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

TileItem_AlphaSoftware::TileItem_AlphaSoftware (const TileItem_AlphaSoftware &copy){
   Error( "TileItem_Alphasoftware: copy constructor not implemented");
}

TileItem_AlphaSoftware::~TileItem_AlphaSoftware (){
}

unsigned char TileItem_AlphaSoftware::GetAlpha(const int x,const int y){
   return (this->*_GetAlpha)( x, y);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Index0 (const int x, const int y){
   return *(m_surface.GetPixels() + y*m_surface.GetPitch() + x * 4 + 0);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Index3 (const int x, const int y){
   return *(m_surface.GetPixels() + y*m_surface.GetPitch() + x * 4 + 3);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Generic (const int x, const int y){
   unsigned char r, g, b, a;
 
   Uint32 pixel = *(Uint32 *)(m_surface.GetPixels() + y*m_surface.GetPitch() + x*m_surface.GetBytesPerPixel()); 
   m_surface.GetRGBA(pixel, r, g, b, a);
   
   return a;
}

void TileItem_AlphaSoftware::Dig(const Point2i &position, Surface& dig){
   int starting_x = position.x >= 0 ? position.x : 0;   
   int starting_y = position.y >= 0 ? position.y : 0; 
   int ending_x = position.x+dig.GetWidth() <= m_surface.GetWidth() ? position.x+dig.GetWidth() : m_surface.GetWidth();
   int ending_y = position.y+dig.GetHeight() <= m_surface.GetHeight() ? position.y+dig.GetHeight() : m_surface.GetHeight();
   
   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       if ( *(dig.GetPixels() + (py-position.y)*dig.GetPitch() + (px-position.x) * 4 + 3) != 0)
         *(m_surface.GetPixels() + py*m_surface.GetPitch() + px * 4 + 3) = 0;
}

Surface TileItem_AlphaSoftware::GetSurface(){
   return m_surface;
}

void TileItem_AlphaSoftware::SyncBuffer(){
   // nothing to do
}

// === Implemenation of TileItem_ALphaHardware ============================

TileItem_AlphaHardware::TileItem_AlphaHardware (unsigned int width, unsigned int height){
   m_width = width;
   m_height = height;

   m_surface.NewSurface(m_width, m_height, SDL_HWSURFACE|SDL_SRCALPHA, true);

   m_buffer = new unsigned char[m_height*m_width];
}

TileItem_AlphaHardware::TileItem_AlphaHardware (const TileItem_AlphaHardware &copy){
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface.NewSurface(m_width, m_height, SDL_HWSURFACE|SDL_SRCALPHA, true);
   m_surface.Blit( copy.m_surface, Point2i(0, 0));

   m_buffer = new unsigned char[m_height*m_width];
   memcpy( copy.m_buffer, m_buffer, m_height*m_width*sizeof( unsigned char));
}

TileItem_AlphaHardware::~TileItem_AlphaHardware (){
   delete[] m_buffer;
}

unsigned char TileItem_AlphaHardware::GetAlpha(const int x,const int y){
   return *(m_buffer + y * m_width + x);
}

void TileItem_AlphaHardware::Dig(const Point2i &position, Surface& dig){
   int starting_x = position.x >= 0 ? position.x : 0;   
   int starting_y = position.y >= 0 ? position.y : 0; 
   int ending_x = position.x+dig.GetWidth() <= m_surface.GetWidth() ? position.x+dig.GetWidth() : m_surface.GetWidth();
   int ending_y = position.y+dig.GetHeight() <= m_surface.GetHeight() ? position.y+dig.GetHeight() : m_surface.GetHeight();
   
   m_surface.Lock();
   
   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       if ( *(dig.GetPixels() + (py-position.y)*dig.GetPitch() + (px-position.x) * 4 + 3) != 0){
        *(m_surface.GetPixels() + py*m_surface.GetPitch() + px * 4 + 3) = 0;
        m_buffer[py*m_width+px] = 0;
     }
   
   m_surface.Unlock();
}

Surface TileItem_AlphaHardware::GetSurface(){
   return m_surface;
}

void TileItem_AlphaHardware::SyncBuffer(){
   m_surface.Lock();
   
   for ( unsigned int y = 0 ; y < m_height ; y++)  
     for ( unsigned int x = 0 ; x < m_width ; x++)
       m_buffer[y*m_width+x] = *(m_surface.GetPixels() + y*m_surface.GetPitch() + x * 4 + 3);
   
   m_surface.Unlock();
}

// === Implemenation of TileItem_Software_Colorkey  ==============================

TileItem_ColorkeySoftware::TileItem_ColorkeySoftware (unsigned int width, unsigned int height){
   m_width = width;
   m_height = height;

   m_surface.NewSurface(m_width, m_height, SDL_SWSURFACE);
   m_surface.SetAlpha(0, 0);
   m_surface.SetColorKey( SDL_SRCCOLORKEY, 0, 0, 0, 0);
   m_buffer = new unsigned char[m_height*m_width];
}

TileItem_ColorkeySoftware::TileItem_ColorkeySoftware (const TileItem_ColorkeySoftware &copy){
   m_width = copy.m_width;
   m_height = copy.m_height;

   m_surface.NewSurface(m_width, m_height, SDL_SWSURFACE);
   m_surface.Blit( copy.m_surface, Point2i(0, 0));
   m_surface.SetAlpha(0, 0);
   m_surface.SetColorKey(SDL_SRCCOLORKEY, 0, 0, 0, 0);

   m_buffer = new unsigned char[m_height*m_width];
   memcpy( copy.m_buffer, m_buffer, m_height*m_width*sizeof( unsigned char));
}

TileItem_ColorkeySoftware::~TileItem_ColorkeySoftware (){
   delete[] m_buffer;
}

unsigned char TileItem_ColorkeySoftware::GetAlpha(const int x,const int y){
   return m_buffer[ y * m_width + x];
}

void TileItem_ColorkeySoftware::Dig(const Point2i &position, Surface& dig){
   int starting_x = position.x >= 0 ? position.x : 0;   
   int starting_y = position.y >= 0 ? position.y : 0; 
   int ending_x = position.x+dig.GetWidth() <= m_surface.GetWidth() ? position.x+dig.GetWidth() : m_surface.GetWidth();
   int ending_y = position.y+dig.GetHeight() <= m_surface.GetHeight() ? position.y+dig.GetHeight() : m_surface.GetHeight();

   Uint32 transparent_color = m_surface.MapRGBA(0, 0, 0, 0);

   for( int py = starting_y ; py < ending_y ; py++) 
     for( int px = starting_x ; px < ending_x ; px++)
       if ( *(dig.GetPixels() + (py-position.y)*dig.GetPitch() + (px-position.x) * 4 + 3) != 0){ 
         *(Uint32 *)(m_surface.GetPixels()+py*m_surface.GetPitch()+px*4) = transparent_color;
         m_buffer[py*m_width+px] = 0;
       }
}

Surface TileItem_ColorkeySoftware::GetSurface(){
   return m_surface;
}

void TileItem_ColorkeySoftware::SyncBuffer(){
   m_surface.Lock();
   
   for ( unsigned int y = 0 ; y < m_height ; y++)  
     for ( unsigned int x = 0 ; x < m_width ; x++)
       m_buffer[y*m_width+x] = *(m_surface.GetPixels() + y*m_surface.GetPitch() + x * 4 + 3);
   
   m_surface.Unlock();
}

// = Tile implementation
Tile::Tile(){
}

void Tile::FreeMem(){
  for (uint i=0; i<nbr_cell; ++i)
    delete item[i];
  nbr_cell = 0;
  item.clear();
}

Tile::~Tile(){ 
  FreeMem();
}

void Tile::InitTile (unsigned int width, unsigned int height){
  nbr_cell_width = width / CELL_SIZE.x;
  if ((width % CELL_SIZE.x) != 0)
    nbr_cell_width++;
  nbr_cell_height = height / CELL_SIZE.y;
  if ((height % CELL_SIZE.y) != 0)
    nbr_cell_height++;

  size.x = width;
  size.y = height;

  nbr_cell = nbr_cell_width * nbr_cell_height; 
}

int clamp (const int val, const int min, const int max){   
   return ( val > max ) ? max : ( val < min ) ? min : val ;
}

void Tile::Dig(const Point2i &position, Surface& dig){  
   Rectanglei rect = Rectanglei( position.x, position.y, dig.GetWidth(), dig.GetHeight()); 

   int first_cell_x = clamp( position.x/CELL_SIZE.x,          0, nbr_cell_width-1);
   int first_cell_y = clamp( position.y/CELL_SIZE.y,          0, nbr_cell_height-1);
   int last_cell_x  = clamp( (position.x+dig.GetWidth())/CELL_SIZE.x, 0, nbr_cell_width-1);
   int last_cell_y  = clamp( (position.y+dig.GetHeight())/CELL_SIZE.y, 0, nbr_cell_height-1);
   
   for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
     for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++){
	  int offset_x = position.x - cx * CELL_SIZE.x;
	  int offset_y = position.y - cy * CELL_SIZE.y;
	  
	  item[cy*nbr_cell_width+cx]->Dig( Point2i(offset_x, offset_y), dig);
     }
}

void Tile::LoadImage (Surface& terrain){
  FreeMem();

  InitTile (terrain.GetWidth(), terrain.GetHeight());
  assert (nbr_cell != 0);

  // Create the TileItem objects
  for (uint i=0; i<nbr_cell; ++i)
     if ( config.transparency == Config::ALPHA )
       item.push_back ( new TileItem_AlphaSoftware(CELL_SIZE.x, CELL_SIZE.y));
     else
       item.push_back ( new TileItem_ColorkeySoftware(CELL_SIZE.x, CELL_SIZE.y));
   
   // Fill the TileItem objects
   for( unsigned int iy = 0 ; iy < nbr_cell_height ; iy++ )
     for( unsigned int ix = 0 ; ix < nbr_cell_width ; ix++ ){
       unsigned int piece = iy * nbr_cell_width + ix;

       Rectanglei sr(ix * CELL_SIZE.x, iy * CELL_SIZE.y, CELL_SIZE.x, CELL_SIZE.y);

       terrain.SetAlpha(0, 0);
       item[piece]->GetSurface().Blit( terrain, sr, Point2i(0, 0));
       item[piece]->SyncBuffer();
    }	

   // Replace transparent tiles by TileItem_Empty tiles

  uint freed = 0;

  for( uint i=0; i<nbr_cell; ++i )
    if( item[i]->IsEmpty() ){
      freed++;
      delete item[i];
      item[i] = (TileItem*)new TileItem_Empty;
    }
}

uchar Tile::GetAlpha (const int x, const int y) const{
   return item[y/CELL_SIZE.y*nbr_cell_width+x/CELL_SIZE.x]->GetAlpha( x%CELL_SIZE.x, y%CELL_SIZE.y);
}

void Tile::DrawTile() const{
   Point2i camPos = camera.GetPosition();

   int first_cell_x = clamp( camPos.x/CELL_SIZE.x,                      0, nbr_cell_width-1);
   int first_cell_y = clamp( camPos.y/CELL_SIZE.y,                      0, nbr_cell_height-1);
   int last_cell_x  = clamp( (camPos.x+camera.GetWidth())/CELL_SIZE.x,  0, nbr_cell_width-1);
   int last_cell_y  = clamp( (camPos.y+camera.GetHeight())/CELL_SIZE.y, 0, nbr_cell_height-1);  

   for ( int iy = first_cell_y ; iy <= last_cell_y ; iy++ )
     for ( int ix = first_cell_x ; ix <= last_cell_x ; ix++)
	 item[iy*nbr_cell_width+ix]->Draw(ix,iy);
}   

void Tile::DrawTile_Clipped( Rectanglei clip_r_world) const
{
   // Select only the items that are under the clip area
   int first_cell_x = clamp( (clip_r_world.GetPositionX())/CELL_SIZE.x,                0, nbr_cell_width-1);
   int first_cell_y = clamp( (clip_r_world.GetPositionY())/CELL_SIZE.y,                0, nbr_cell_height-1);
   int last_cell_x  = clamp( (clip_r_world.GetPositionX()+clip_r_world.GetSizeX() +1)/CELL_SIZE.x, 0, nbr_cell_width-1);
   int last_cell_y  = clamp( (clip_r_world.GetPositionY()+clip_r_world.GetSizeY() +1)/CELL_SIZE.y, 0, nbr_cell_height-1);

   for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
     for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++)
     {
       // For all selected items, clip source and destination
	   // blitting rectangles 
       int dest_x = cx * CELL_SIZE.x;  
       int dest_y = cy * CELL_SIZE.y;
       int dest_w = CELL_SIZE.x;
       int dest_h = CELL_SIZE.y;
       int src_x = 0;
       int src_y = 0;
	  
       if ( dest_x < clip_r_world.GetPositionX() ){ // left clipping
	     src_x  += clip_r_world.GetPositionX() - dest_x;
	     dest_w -= clip_r_world.GetPositionX() - dest_x;
	     dest_x  = clip_r_world.GetPositionX();
	   }
	  
	   if ( dest_y < clip_r_world.GetPositionY() ){  // top clipping
	     src_y  += clip_r_world.GetPositionY() - dest_y;
	     dest_h -= clip_r_world.GetPositionY() - dest_y;
	     dest_y  = clip_r_world.GetPositionY();
	   }
	  
	   if ( dest_x + dest_w > clip_r_world.GetPositionX() + clip_r_world.GetSizeX() +1) // right clipping
	       dest_w -= ( dest_x + dest_w ) - ( clip_r_world.GetPositionX() + clip_r_world.GetSizeX() +1);
	  
	   if ( dest_y + dest_h > clip_r_world.GetPositionY() + clip_r_world.GetSizeY() +1) // bottom clipping
	       dest_h -= ( dest_y + dest_h ) - ( clip_r_world.GetPositionY() + clip_r_world.GetSizeY() +1);
	  
	   Rectanglei sr(src_x, src_y, dest_w, dest_h);
	       
	   // Decall the destination rectangle along the camera offset
	   Point2i dr( Point2i(dest_x, dest_y) - camera.GetPosition());
	  
	   app.video.window.Blit( item[cy*nbr_cell_width+cx]->GetSurface(), sr, dr);
   }
}   
