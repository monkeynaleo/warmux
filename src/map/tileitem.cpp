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

void TileItem_NonEmpty::Dig(const Point2i &center, const uint radius)
{
  need_check_empty = true;
  unsigned char* buf   = m_surface.GetPixels();
  const uint line_size = m_surface.GetPitch();

  int y = center.y - (int)(radius+EXPLOSION_BORDER_SIZE);
  if (y < 0) y = 0;
  buf += y * line_size;

  need_check_empty = true;
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
      Darken(center.x-blength, center.x+blength, buf);
      continue;
    }

    //Zone of the line which needs to be emptied
    int length = round(sqrt(Double(radius*radius - dac*dac)));

    // Left half of the circle
    Darken(center.x-blength, center.x-length, buf);

    // Rigth half of the circle
    Darken(center.x+length, center.x+blength, buf);

    Empty(center.x-length, center.x+length, buf);
  }
}

void TileItem_NonEmpty::MergeSprite(const Point2i &position, Surface& spr)
{
  m_surface.MergeSurface(spr, position);
}

// === Implemenation of TileItem_ColorKey ==============================
Uint32 TileItem_ColorKey::ColorKey = 0xFF00FF;

TileItem_ColorKey::TileItem_ColorKey()
{
  SDL_Surface     *surf = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCCOLORKEY,
                                               CELL_SIZE.x, CELL_SIZE.y, 16,
                                               0, 0, 0, 0);
  m_surface = Surface(SDL_DisplayFormat(surf));
  color_key = SDL_MapRGBA(m_surface.GetSurface()->format, 255, 0, 255, 0);
  m_surface.SetColorKey(SDL_SRCCOLORKEY, color_key);
  SDL_FreeSurface(surf);

  need_check_empty = true;
}

TileItem_ColorKey::TileItem_ColorKey(void *pixels, int pitch, uint threshold)
{
  uint8_t *ptr  = (uint8_t*)pixels;
  int      x, y;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#  define OFFSET  3
#else
#  define OFFSET  0
#endif

  // Set pixels considered as transparent as colorkey
  for (y=0; y<CELL_SIZE.y; y++)
  {
    for (x=0; x<CELL_SIZE.x; x++)
      if (ptr[x*4 + OFFSET] < threshold)
        *((Uint32*)(ptr + x*4)) = ColorKey;
    ptr += pitch;
  }

  SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(pixels, CELL_SIZE.x, CELL_SIZE.y, 32, pitch,
                                               0xFF0000, 0xFF00, 0xFF, 0xFF000000);
  SDL_SetAlpha(surf, 0, 0);
  m_surface = Surface(SDL_DisplayFormat(surf));
  color_key = SDL_MapRGBA(m_surface.GetSurface()->format, 255, 0, 255, 0);
  m_surface.SetColorKey(SDL_SRCCOLORKEY, color_key);
  SDL_FreeSurface(surf);

  need_check_empty = true;
}

TileItem_NonEmpty* TileItem_ColorKey::NewEmpty(void)
{
  TileItem_ColorKey *ti = new TileItem_ColorKey();
  ti->m_surface.Fill(ti->color_key);

  return ti;
}

void TileItem_ColorKey::Darken(int start_x, int end_x, unsigned char* buf)
{
  if( start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    uint16_t *ptr = (uint16_t*)buf;
    ptr += start_x;
    while(end_x--) {
      uint16_t s = ptr[0];
      if (s != color_key)
        ptr[0] = (s>>1)&0x7BEF;
      ptr++;
    }
  }
}

void TileItem_ColorKey::Empty(int start_x, int end_x, unsigned char* buf)
{
  if( start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    uint16_t *ptr = (uint16_t*)buf;
    ptr += start_x;
    while (end_x--)
      *(ptr++) = color_key;
  }
}

void TileItem_ColorKey::Dig(const Point2i &position, const Surface& dig)
{
  int starting_x = position.x >= 0 ? position.x : 0;
  int starting_y = position.y >= 0 ? position.y : 0;
  int ending_x = position.x+dig.GetWidth() <= m_surface.GetWidth() ? position.x+dig.GetWidth() : m_surface.GetWidth();
  int ending_y = position.y+dig.GetHeight() <= m_surface.GetHeight() ? position.y+dig.GetHeight() : m_surface.GetHeight();

  need_check_empty = true;
  for (int py = starting_y ; py < ending_y ; py++) {
    for (int px = starting_x ; px < ending_x ; px++) {
      if (dig.GetPixel(px-position.x, py-position.y) != 0)
        m_surface.PutPixel(px, py, color_key);
    }
  }
}

void TileItem_ColorKey::ScalePreview(uint8_t* out, int x, uint opitch, uint shift)
{
  const Uint16 *idata  = (Uint16*)m_surface.GetPixels();
  uint          ipitch = m_surface.GetPitch();

  out  += 4*x*(CELL_SIZE.x>>shift);
  ipitch >>= 1;

  for (int j=0; j<CELL_SIZE.y>>shift; j++) {
    for (int i=0; i<CELL_SIZE.x>>shift; i++) {
      const Uint16* ptr = idata + (i<<shift);
      uint count = 0, p0 = 0, p1 = 0, p2 = 0;

      for (uint u=0; u<(1U<<shift); u++) {
        for (uint v=0; v<(1U<<shift); v++) {
          Uint16 tmp = ptr[v];
          if (tmp != color_key) {
            p0 += (tmp&0xF800)>>(11-3);
            p1 += (tmp&0x07C0)>>(5-2);
            p2 += (tmp&0x001F)<<3;
            count++;
          }
        }
        ptr += ipitch;
      }

      // Convert color_key count to alpha
      if (count) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        out[4*i+0] = p0 / count;
        out[4*i+1] = p1 / count;
        out[4*i+2] = p2 / count;
        out[4*i+3] = (255*count)>>(2*shift);
#else
        out[4*i+0] = (255*count)>>(2*shift);
        out[4*i+1] = p2 / count;
        out[4*i+2] = p1 / count;
        out[4*i+3] = p0 / count;
#endif
      } else {
        // Completely transparent
        *((Uint32*)(out+4*i)) = 0;
      }
    }
    out   += opitch;
    idata += ipitch<<shift;
  }
}

bool TileItem_ColorKey::NeedDelete()
{
  const Uint16 *ptr    = (Uint16*)m_surface.GetPixels();
  int           stride = m_surface.GetPitch()>>1;
  int           x, y;

  assert(need_check_empty);
  need_check_empty = false;

  for (y=0; y<CELL_SIZE.y; y++) {
    for (x=0; x<CELL_SIZE.x; x++)
      if (ptr[x] != color_key)
        return false;

    ptr += stride;
  }

  return true;
}

// === Implemenation of TileItem_SoftwareAlpha ==============================
void TileItem_AlphaSoftware::SetDefaults(void)
{
  need_check_empty = true;

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

void TileItem_AlphaSoftware::ScalePreview(uint8_t *odata, int x, uint opitch, uint shift)
{
  const Uint8 *idata  = m_surface.GetPixels();
  uint         ipitch = m_surface.GetPitch();
  uint         p0, p1, p2, p3;

  odata += 4*x*(CELL_SIZE.x>>shift);
  for (int j=0; j<CELL_SIZE.y>>shift; j++) {
    for (int i=0; i<CELL_SIZE.x>>shift; i++) {
      const Uint8* ptr = idata + (i<<(2+shift));

      p0 = 0; p1 = 0; p2 = 0; p3 = 0;

      for (uint u=0; u<(1U<<shift); u++) {
        for (uint v=0; v<(1U<<shift); v++) {
          p0 += ptr[4*v+0];
          p1 += ptr[4*v+1];
          p2 += ptr[4*v+2];
          p3 += ptr[4*v+3];
        }
        ptr += ipitch;
      }

      odata[4*i+0] = (p0 + (1<<(2*shift-1)))>>(2*shift);
      odata[4*i+1] = (p1 + (1<<(2*shift-1)))>>(2*shift);
      odata[4*i+2] = (p2 + (1<<(2*shift-1)))>>(2*shift);
      odata[4*i+3] = (p3 + (1<<(2*shift-1)))>>(2*shift);
    }
    odata += opitch;
    idata += ipitch<<shift;
  }
}

void TileItem_AlphaSoftware::Empty(int start_x, int end_x, unsigned char* buf)
{
  if( start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    buf += start_x * 4;
    memset(buf, 0 , 4 * end_x);
  }
}

void TileItem_AlphaSoftware::Dig(const Point2i &position, const Surface& dig)
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

void TileItem_AlphaSoftware::Darken(int start_x, int end_x, unsigned char* buf)
{
  if( start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    uint32_t *ptr = (uint32_t*)buf;
    ptr += start_x;
    while(end_x--) {
      uint32_t s = ptr[0];
      // Mask component MSBs and alpha, then readd alpha
  #if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
      ptr[0] = ((s>>1)&0x007F7F7F) | (s&0xFF000000);
  #else
      ptr[0] = ((s>>1)&0x7F7F7F00) | (s&0xFF);
  #endif
      ptr++;
    }
  }
}

#ifdef DBG_TILE
void TileItem_AlphaSoftware::FillWithRGB(Uint8 r, Uint8 g, Uint8 b)
{
  int x=0, y=0;
  while(y < CELL_SIZE.y)
  {
    Uint32 pixel = m_surface.GetPixel();
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

bool TileItem_AlphaSoftware::NeedDelete()
{
  const Uint8 *ptr   = m_surface.GetPixels();

  assert(need_check_empty);
  need_check_empty = false;

  for (int i=0; i<CELL_SIZE.x*CELL_SIZE.y*4; i++)
    if (ptr[i])
      return false;

  return true;
}
