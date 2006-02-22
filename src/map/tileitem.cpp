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
 *****************************************************************************/

#include "tileitem.h"
#include <iostream>
#include <SDL.h>
#include <SDL_endian.h>
#include "../game/config.h"
#include "../include/app.h"
#include "../map/camera.h"
#include "../tool/error.h"
#include "../tool/point.h"

// === Common to all TileItem_* except TileItem_Emtpy ==============================
void TileItem::Draw(const Point2i &pos){
  app.video.window.Blit(GetSurface(),
        Rectanglei(Point2i(0, 0), CELL_SIZE),
        pos * CELL_SIZE - camera.GetPosition());
}

bool TileItem::IsEmpty(){
  Point2i pos;

  for( pos.x = 0 ; pos.x < CELL_SIZE.x; pos.x++ )
     for( pos.y = 0 ; pos.y < CELL_SIZE.y; pos.y++ )
       if( GetAlpha(pos) == 255 )
         return false;

   return true;
}

// === Implemenation of TileItem_Software_ALpha ==============================
TileItem_AlphaSoftware::TileItem_AlphaSoftware(const Point2i &size){
    m_size = size;
    m_surface = Surface(size, SDL_SWSURFACE|SDL_SRCALPHA, true).DisplayFormatAlpha();

    _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Generic;
    if( m_surface.GetBytesPerPixel() == 4 ){
       if( m_surface.GetSurface()->format->Amask == 0x000000ff ){
           if( SDL_BYTEORDER == SDL_LIL_ENDIAN )
               _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index0;
           else
               _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index3;
       }else{
           if( m_surface.GetSurface()->format->Amask == 0xff000000 ){
                if (SDL_BYTEORDER == SDL_LIL_ENDIAN )
                    _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index3;
                else
                    _GetAlpha = &TileItem_AlphaSoftware::GetAlpha_Index0;
            }
        }
    }
}

TileItem_AlphaSoftware::~TileItem_AlphaSoftware(){
}

unsigned char TileItem_AlphaSoftware::GetAlpha(const Point2i &pos){
    return (this->*_GetAlpha)(pos);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Index0 (const Point2i &pos){
    return *(m_surface.GetPixels() + pos.y*m_surface.GetPitch() + pos.x * 4 + 0);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Index3 (const Point2i &pos){
    return *(m_surface.GetPixels() + pos.y*m_surface.GetPitch() + pos.x * 4 + 3);
}

unsigned char TileItem_AlphaSoftware::GetAlpha_Generic (const Point2i &pos){
    unsigned char r, g, b, a;

    Uint32 pixel = *(Uint32 *)(m_surface.GetPixels() + pos.y*m_surface.GetPitch() + pos.x*m_surface.GetBytesPerPixel()); 
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

TileItem_AlphaHardware::TileItem_AlphaHardware(const Point2i &size){
    m_size = size;

    m_surface.NewSurface(size, SDL_HWSURFACE|SDL_SRCALPHA, true);

    m_buffer = new unsigned char[size.x*size.y];
}

TileItem_AlphaHardware::~TileItem_AlphaHardware(){
    delete[] m_buffer;
}

unsigned char TileItem_AlphaHardware::GetAlpha(const Point2i &pos){
    return m_buffer[pos.y * m_size.x + pos.x];
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
                m_buffer[py*m_size.x + px] = 0;
            }

   m_surface.Unlock();
}

Surface TileItem_AlphaHardware::GetSurface(){
    return m_surface;
}

void TileItem_AlphaHardware::SyncBuffer(){
    m_surface.Lock();

    for( int y = 0 ; y < m_size.y ; y++ )
        for( int x = 0 ; x < m_size.x ; x++ )
            m_buffer[y*m_size.x + x] = *(m_surface.GetPixels() + y*m_surface.GetPitch() + x * 4 + 3);

    m_surface.Unlock();
}

// === Implemenation of TileItem_Software_Colorkey  ==============================

TileItem_ColorkeySoftware::TileItem_ColorkeySoftware(const Point2i &size){
    m_size = size;

    m_surface.NewSurface(size, SDL_SWSURFACE);
    m_surface.SetAlpha(0, 0);
    m_surface.SetColorKey( SDL_SRCCOLORKEY, 0, 0, 0, 0);

    m_buffer = new unsigned char[m_size.x * m_size.y];
}

TileItem_ColorkeySoftware::~TileItem_ColorkeySoftware(){
    delete[] m_buffer;
}

unsigned char TileItem_ColorkeySoftware::GetAlpha(const Point2i &pos){
    return m_buffer[pos.y * m_size.x + pos.x];
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
                m_buffer[py*m_size.x + px] = 0;
            }
}

Surface TileItem_ColorkeySoftware::GetSurface(){
    return m_surface;
}

void TileItem_ColorkeySoftware::SyncBuffer(){
   m_surface.Lock();

    for( int y = 0 ; y < m_size.y ; y++ )
        for( int x = 0 ; x < m_size.x ; x++ )
            m_buffer[y*m_size.x + x] = *(m_surface.GetPixels() + y*m_surface.GetPitch() + x * 4 + 3);

    m_surface.Unlock();
}

