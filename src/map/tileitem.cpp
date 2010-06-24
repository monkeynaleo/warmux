/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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

#include "map/tileitem.h"
#include <iostream>
#include <SDL.h>
#include <SDL_endian.h>
#include "map/tile.h"
#include "game/config.h"
#include "graphic/video.h"
#include "include/app.h"
#include "map/camera.h"
#include "tool/math_tools.h"
#include <WORMUX_point.h>
#ifdef DBG_TILE
#include "graphic/colors.h"
#endif

void TileItem_Empty::Draw(const Point2i &/*pos*/)
{
#ifdef DBG_TILE
  GetMainWindow().FillRect(Rectanglei(pos * CELL_SIZE - Camera::GetInstance()->GetPosition(),
                                      CELL_SIZE),
                           c_red);
#endif
}

// === Common to all TileItem_* except TileItem_Emtpy ==============================
void TileItem_NonEmpty::Draw(const Point2i &pos)
{
  GetMainWindow().Blit(GetSurface(),
                       pos * CELL_SIZE - Camera::GetInstance()->GetPosition());
}

void TileItem_NonEmpty::ResetEmptyCheck()
{
  need_check_empty = true;
  last_filled_pixel = m_surface.GetPixels();
}

void TileItem_NonEmpty::Empty(const int start_x, const int end_x, unsigned char* buf, const int bpp) const
{
  if( start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    int tile_start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    ASSERT( tile_start_x >= 0 && tile_start_x < CELL_SIZE.x);
    int tile_lenght = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - tile_start_x : end_x - tile_start_x + 1;
    ASSERT( tile_lenght > 0);
    ASSERT( tile_start_x + tile_lenght <= CELL_SIZE.x);

    ASSERT(buf + tile_start_x * bpp + bpp * (tile_lenght-1) < m_surface.GetPixels() + CELL_SIZE.x * CELL_SIZE.y * bpp); //Check for overflow
    memset(buf + tile_start_x * bpp, 0 , bpp * tile_lenght);
  }
}

void TileItem_NonEmpty::Dig(const Point2i &position, const Surface& dig)
{
  int starting_x = position.x >= 0 ? position.x : 0;
  int starting_y = position.y >= 0 ? position.y : 0;
  int ending_x = position.x+dig.GetWidth() <= m_surface.GetWidth() ? position.x+dig.GetWidth() : m_surface.GetWidth();
  int ending_y = position.y+dig.GetHeight() <= m_surface.GetHeight() ? position.y+dig.GetHeight() : m_surface.GetHeight();

  need_check_empty = true;
  for (int py = starting_y ; py < ending_y ; py++) {
    for (int px = starting_x ; px < ending_x ; px++) {
      if (dig.GetPixel(px-position.x, py-position.y) != 0)
        m_surface.PutPixel(px, py, 0);
    }
  }
}

void TileItem_NonEmpty::Dig(const Point2i &center, const uint radius)
{
  need_check_empty = true;
  unsigned char* buf   = m_surface.GetPixels();
  const uint line_size = m_surface.GetPitch();
  const uint bpp       = m_surface.GetBytesPerPixel();

  int y = center.y - (int)(radius+EXPLOSION_BORDER_SIZE);
  if (y < 0) y = 0;
  buf += y * line_size;

  //Empties each line of the tile horizontaly that are in the circle
  for (; (uint)y <= center.y + radius + EXPLOSION_BORDER_SIZE && y < CELL_SIZE.y;
       buf += line_size, y++)
  {
    //Abscisse distance from the center of the circle to the circle
    int dac = center.y - y;

    //Darken the border of the removed ground
    int blength = round(sqrt((Double)(radius+EXPLOSION_BORDER_SIZE)*(radius+EXPLOSION_BORDER_SIZE) - dac*dac));

    //Nothing to empty, just darken
    if ((uint)abs(dac) > radius) {
      Darken(center.x-blength, center.x+blength, buf, bpp);
      continue;
    }

    //Zone of the line which needs to be emptied
    int length = round(sqrt(Double(radius*radius - dac*dac)));

    // Left half of the circle
    Darken(center.x-blength, center.x-length, buf, bpp);

    // Rigth half of the circle
    Darken(center.x+length, center.x+blength, buf, bpp);

    Empty(center.x-length, center.x+length, buf, bpp);
  }
}

void TileItem_NonEmpty::MergeSprite(const Point2i &position, Surface& spr)
{
  m_surface.MergeSurface(spr, position);
}

// === Implemenation of TileItem_SoftwareAlpha ==============================
void TileItem_AlphaSoftware::SetDefaults(void)
{
  need_delete = false;
  ResetEmptyCheck();

  if (m_surface.GetBytesPerPixel() == 4) {
    if (m_surface.GetSurface()->format->Amask == 0x000000ff)
      m_offset = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? 0 : 3;
    else
      m_offset = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? 3 : 0;
  } else {
    fprintf(stderr, "Unexpected depth of %u for TileItem\n", m_surface.GetBytesPerPixel());
    m_offset = 2;
  }
}

TileItem_AlphaSoftware::TileItem_AlphaSoftware() {
  m_surface = Surface(CELL_SIZE, SDL_SWSURFACE|SDL_SRCALPHA, true).DisplayFormatAlpha();
  SetDefaults();
}

TileItem_AlphaSoftware::TileItem_AlphaSoftware(void *pixels, int pitch) {
  SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(pixels, CELL_SIZE.x, CELL_SIZE.y, 32, pitch,
                                               0xFF0000, 0xFF00, 0xFF, 0xFF000000);
  m_surface = Surface(SDL_DisplayFormatAlpha(surf));
  SDL_FreeSurface(surf);
  SetDefaults();
}

TileItem_NonEmpty* TileItem_AlphaSoftware::NewEmpty(void)
{
  TileItem_AlphaSoftware *ti = new TileItem_AlphaSoftware();
  ti->m_surface.SetAlpha(0, 0);
  ti->m_surface.Fill(0x00000000);
  ti->m_surface.SetAlpha(SDL_SRCALPHA, 0);

  return ti;
}

void TileItem_AlphaSoftware::ScalePreview(uint8_t *odata, uint opitch, uint shift)
{
  const Uint8 * idata  = m_surface.GetPixels();
  uint          ipitch = m_surface.GetPitch();
  uint p0;
  uint p1;
  uint p2;
  uint p3;

  for (int j=0; j<CELL_SIZE.y>>shift; j++) {
    for (int i=0; i<CELL_SIZE.x>>shift; i++) {
      p0 = 0;
      p1 = 0;
      p2 = 0;
      p3 = 0;
      const Uint8* ptr = idata + (i<<(2+shift));

      for (uint u=0; u<(1U<<shift); u++) {
        for (uint v=0; v<(1U<<shift); v++) {
          p0 += ptr[4*v+0];
          p1 += ptr[4*v+1];
          p2 += ptr[4*v+2];
          p3 += ptr[4*v+3];
        }
        ptr += ipitch;
      }

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
      p3 = (p3 + (1<<(2*shift-1)))>>(2*shift);
      if (p3 < 160) {
        memset(odata+4*i, 0, 4);
      } else {
        odata[4*i+0] = (p0 + (1<<(2*shift-1)))>>(2*shift);
        odata[4*i+1] = (p1 + (1<<(2*shift-1)))>>(2*shift);
        odata[4*i+2] = (p2 + (1<<(2*shift-1)))>>(2*shift);
        odata[4*i+3] = 255;
      }
#else
      p0 = (p0 + (1<<(2*shift-1)))>>(2*shift);
      if (p0 < 160) {
        memset(odata+4*i, 0, 4);
      } else {
        odata[4*i+0] = 255;
        odata[4*i+1] = (p1 + (1<<(2*shift-1)))>>(2*shift);
        odata[4*i+2] = (p2 + (1<<(2*shift-1)))>>(2*shift);
        odata[4*i+3] = (p3 + (1<<(2*shift-1)))>>(2*shift);
      }
#endif
    }
    odata += opitch;
    idata += ipitch<<shift;
  }
}

void TileItem_AlphaSoftware::Darken(const int start_x, const int end_x, unsigned char* buf, const int bpp) const
{
  if( start_x < CELL_SIZE.x && end_x >= 0)
  {
    //Clamp the value to empty only the in this tile
    int tile_start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    ASSERT( tile_start_x >= 0 && tile_start_x < CELL_SIZE.x);
    int tile_lenght = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - tile_start_x : end_x - tile_start_x + 1;
    ASSERT( tile_lenght > 0);
    ASSERT( tile_start_x + tile_lenght <= CELL_SIZE.x);

    buf += tile_start_x * bpp;
    while(tile_lenght--)
    {
#if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
      *(buf++) /= 2;
      *(buf++) /= 2;
      *(buf++) /= 2;
      buf++; //skip alpha chanel
#else
      buf++; //skip alpha chanel
      *(buf++) /= 2;
      *(buf++) /= 2;
      *(buf++) /= 2;
#endif
    }
  }
}

#ifdef DBG_TILE
void TileItem_AlphaSoftware::FillWithRGB(Uint8 r, Uint8 g, Uint8 b)
{
  int x=0, y=0;
  while(y < CELL_SIZE.y)
  {
    Uint32 pixel = *(Uint32 *)(m_surface.GetPixels() + y*m_surface.GetPitch() + x*m_surface.GetBytesPerPixel());
    Uint8 tmp,a;
    m_surface.GetRGBA(pixel, tmp, tmp, tmp, a);
    if(a != SDL_ALPHA_TRANSPARENT)
    {
      Uint32 col = m_surface.MapRGBA(r, g, b, a);
      m_surface.PutPixel(x, y, col);
    }
    x++;
    if(x == CELL_SIZE.y)
    {
      x = 0;
      y++;
    }
  }
}
#endif

bool TileItem_AlphaSoftware::CheckEmpty()
{
  ASSERT(need_check_empty);
  unsigned char alpha = last_filled_pixel[m_offset];

  if (alpha == SDL_ALPHA_TRANSPARENT) {
    last_filled_pixel += 4;
    if (last_filled_pixel >= m_surface.GetPixels() + (m_surface.GetPitch() * CELL_SIZE.y)) {
      need_delete = true;
      need_check_empty = false;
    }
  } else {
    need_check_empty = false;
  }
  return need_delete;
}
