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

// === Common to all TileItem_* except TileItem_Emtpy ==============================
TileItem_NonEmpty::TileItem_NonEmpty(uint8_t alpha_threshold)
  : TileItem()
  , m_alpha_threshold(alpha_threshold)
{
  m_empty_bitfield = new uint8_t[(CELL_SIZE.x>>3) * CELL_SIZE.y];
  ForceRecheck();
}

void TileItem_NonEmpty::Draw(const Point2i &pos)
{
  GetMainWindow().Blit(GetSurface(),
                       pos * CELL_SIZE - Camera::GetInstance()->GetPosition());
}

void TileItem_NonEmpty::Dig(const Point2i &center, const uint radius)
{
  uint8_t    *buf       = m_surface.GetPixels();
  const uint  line_size = m_surface.GetPitch();

  int r = radius+EXPLOSION_BORDER_SIZE;
  int y = center.y - r;
  if (y < 0)
    y = 0;
  int endy = center.y + radius + EXPLOSION_BORDER_SIZE + 1;
  if (endy >= CELL_SIZE.y)
    endy = CELL_SIZE.y;

  buf += y * line_size;

  //Empties each line of the tile horizontaly that are in the circle
  int maxx = 0, minx = CELL_SIZE.x;
  for (; y < endy; buf += line_size, y++) {
    //Abscisse distance from the center of the circle to the circle
    int dac = center.y - y;

    //Darken the border of the removed ground
    int blength = round(sqrt(Double(r*r - dac*dac)));

    if (minx>center.x-blength)
      minx = center.x-blength;
    if (maxx<center.x+blength+1)
      maxx = center.x+blength+1;

    //Nothing to empty, just darken
    if ((uint)abs(dac) > radius) {
      Darken(center.x-blength, center.x+blength, buf);
      continue;
    }

    //Zone of the line which needs to be emptied
    int length = round(sqrt(Double(radius*radius - dac*dac)));

    // Left half of the circle
    Darken(center.x-blength, center.x-length, buf);

    // Right half of the circle
    Darken(center.x+length, center.x+blength, buf);

    Empty(center.x-length, center.x+length, buf);
  }

  if (minx < 0)
    minx = 0;
  if (maxx > CELL_SIZE.x)
    maxx = CELL_SIZE.x;

  m_start_check.SetValues(minx, (center.y - r)>0 ? center.y - r : 0);
  m_end_check.SetValues(maxx, endy);

  m_need_check_empty = true;
}

void TileItem_NonEmpty::CheckEmptyField()
{
  const Uint32 *ptr = (Uint32 *)m_empty_bitfield;

  m_need_check_empty = false;

  for (int i=0; i<(CELL_SIZE.y*CELL_SIZE.x)>>(3+2); i++, ptr++) {
    if (*ptr != 0xFFFFFFFF) {
      m_is_empty = false;
      return;
    }
  }
  m_is_empty = true;
}

void TileItem_NonEmpty::ForceRecheck()
{
  m_start_check.SetValues(0, 0);
  m_end_check.SetValues(CELL_SIZE);
  m_need_check_empty = true;
  m_is_empty = true;
}

void TileItem_NonEmpty::ForceEmpty()
{
  memset(m_empty_bitfield, 0xFF, (CELL_SIZE.x>>3)*CELL_SIZE.y);
  m_is_empty = true;
  m_need_check_empty = false;
}

TileItem_NonEmpty* TileItem_NonEmpty::NewEmpty(uint8_t bpp, uint8_t alpha_threshold)
{
  TileItem_NonEmpty *ti;

  switch (bpp) {
  case 2: ti = new TileItem_ColorKey16(alpha_threshold); break;
  // Otherwise, we probably need to merge a sprite in, so let's be clean
  default: ti = new TileItem_AlphaSoftware(alpha_threshold); break;
  }

  ti->ForceEmpty();
  
  return ti;
}

// === Implemenation of TileItem_BaseColorKey ==============================
TileItem_BaseColorKey::TileItem_BaseColorKey(uint8_t bpp, uint8_t alpha_threshold)
  : TileItem_NonEmpty(alpha_threshold)
{
  SDL_Surface     *surf = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCCOLORKEY,
                                               CELL_SIZE.x, CELL_SIZE.y, bpp,
                                               0, 0, 0, 0);
  m_surface = Surface(SDL_DisplayFormat(surf));
  SDL_FreeSurface(surf);
  MapColorKey();
}

void TileItem_BaseColorKey::ForceEmpty()
{
  TileItem_NonEmpty::ForceEmpty();
  m_surface.Fill(color_key);
}

void TileItem_BaseColorKey::MapColorKey()
{
  color_key = m_surface.MapRGBA(255, 0, 255, 0);
  m_surface.SetColorKey(SDL_SRCCOLORKEY, color_key);
}

void TileItem_BaseColorKey::Dig(const Point2i &position, const Surface& dig)
{
  m_start_check.SetValues(position.max(Point2i(0, 0)));
  m_end_check.SetValues(m_surface.GetSize().min(position+dig.GetSize()));

  for (int py = m_start_check.y ; py < m_end_check.y ; py++) {
    for (int px = m_start_check.x ; px < m_end_check.x ; px++) {
      if (dig.GetPixel(px-position.x, py-position.y) != 0)
        m_surface.PutPixel(px, py, color_key);
    }
  }

  m_need_check_empty = true;
}

bool TileItem_BaseColorKey::CheckEmpty()
{
  uint8_t *buf = m_empty_bitfield;
  int sx = m_start_check.x&0xFFFFFFF0,
      ex = (m_end_check.x+7)&0xFFFFFFF0;

  m_need_check_empty = false;
  m_is_empty = true;
  buf += (sx + m_start_check.y*CELL_SIZE.x)>>8;

  for (int py=m_start_check.y; py<m_end_check.y; py++) {
    for (int px=sx; px<ex; px+=8, buf++) {
      uint8_t empty_mask = 0;

      for (int i=0; i<8; i++) {
        if (m_surface.GetPixel(px+i, py) == color_key)
          empty_mask |= 1<<i;
      }

      if (empty_mask != 0xFF)
        m_is_empty = false;
      buf[0] = empty_mask;
    }

    buf += CELL_SIZE.x>>8;
  }

  // Make sure it is empty
  if (m_is_empty)
    CheckEmptyField();
  return m_is_empty;
}

void TileItem_BaseColorKey::MergeSprite(const Point2i &position, Surface& spr)
{
  m_start_check.SetValues(position.max(Point2i(0, 0)));
  m_end_check.SetValues(m_surface.GetSize().min(position+spr.GetSize()));

  spr.Lock();

  for (int py = m_start_check.y ; py < m_end_check.y ; py++) {
    for (int px = m_start_check.x ; px < m_end_check.x ; px++) {
      Uint32 pixel = spr.GetPixel(px-position.x, py-position.y);
      if (pixel) {
        Uint8 r, g, b, a;
        spr.GetRGBA(pixel, r, g, b, a);
        if (a < m_alpha_threshold) {
          pixel = color_key;
        } else {
          pixel = m_surface.MapRGBA(r, g, b, a);
        }
      } else {
        pixel = color_key;
      }

      m_surface.PutPixel(px, py, pixel);
    }
  }

  spr.Unlock();
  m_need_check_empty = true;
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#  define ALPHA_OFFSET  3
#else
#  define ALPHA_OFFSET  0
#endif


// === Implemenation of TileItem_ColorKey16 ==============================
TileItem_ColorKey16::TileItem_ColorKey16(void *pixels, int pitch, uint8_t threshold)
  : TileItem_BaseColorKey(16, threshold)
{
  uint8_t *ptr  = (uint8_t*)pixels;
  int      x, y;

  // Set pixels considered as transparent as colorkey
  for (y=0; y<CELL_SIZE.y; y++) {
    for (x=0; x<CELL_SIZE.x; x++)
      if (ptr[x*4 + ALPHA_OFFSET] < m_alpha_threshold)
        *((Uint32*)(ptr + x*4)) = COLOR_KEY;
    ptr += pitch;
  }

  SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(pixels, CELL_SIZE.x, CELL_SIZE.y, 32, pitch,
                                               0xFF0000, 0xFF00, 0xFF, 0xFF000000);
  m_surface = Surface(SDL_DisplayFormat(surf));
  SDL_FreeSurface(surf);
  MapColorKey();
}

void TileItem_ColorKey16::Darken(int start_x, int end_x, uint8_t* buf)
{
  if (start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    uint16_t *ptr = (uint16_t*)buf;
    ptr += start_x;
    while (end_x--) {
      uint16_t s = ptr[0];
      if (s != color_key)
        ptr[0] = (s>>1)&0x7BEF;
      ptr++;
    }
  }
}

void TileItem_ColorKey16::Empty(int start_x, int end_x, uint8_t* buf)
{
  if (start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    uint16_t *ptr = (uint16_t*)buf;
    uint16_t ckey = color_key;
    ptr += start_x;
    while (end_x--)
      *(ptr++) = ckey;
  }
}

void TileItem_ColorKey16::ScalePreview(uint8_t* out, int x, uint opitch, uint shift)
{
  const Uint16 *idata  = (Uint16*)m_surface.GetPixels();
  uint          ipitch = m_surface.GetPitch();
  Point2i       start  = m_start_check>>shift;
  Point2i       end    = (m_end_check + (1<<shift) -1)>>shift;

  out   += (x<<(2+CELL_BITS-shift)) + start.y*opitch;
  ipitch >>= 1;
  idata += (start.y<<shift)*ipitch;

  for (int j=start.y; j<end.y; j++) {
    for (int i=start.x; i<end.x; i++) {
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

  if (m_need_check_empty)
    CheckEmpty();
}

// === Implemenation of TileItem_ColorKey24 ==============================
TileItem_ColorKey24::TileItem_ColorKey24(void *pixels, int pitch, uint8_t threshold)
  : TileItem_BaseColorKey(24, threshold)
{
  uint8_t *ptr  = (uint8_t*)pixels;
  int      x, y;

  // Set pixels considered as transparent as colorkey
  for (y=0; y<CELL_SIZE.y; y++) {
    for (x=0; x<CELL_SIZE.x; x++)
      if (!ptr[x*4 + ALPHA_OFFSET])
        *((Uint32*)(ptr + x*4)) = COLOR_KEY;
    ptr += pitch;
  }

  SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(pixels, CELL_SIZE.x, CELL_SIZE.y, 32, pitch,
                                               0xFF0000, 0xFF00, 0xFF, 0xFF000000);
  SDL_SetAlpha(surf, 0, 0);
  SDL_PixelFormat fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.BitsPerPixel = 24;
  m_surface = Surface(SDL_ConvertSurface(surf, &fmt, SDL_SWSURFACE));
  SDL_FreeSurface(surf);
  MapColorKey();
}

void TileItem_ColorKey24::Darken(int start_x, int end_x, uint8_t* buf)
{
  if( start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    buf += 3*start_x;
    while(end_x--) {
      if (buf[0]!=0xFF || buf[1] || buf[2]!=0xFF) {
        buf[0] = (buf[0]>>1)&0x7F;
        buf[1] = (buf[1]>>1)&0x7F;
        buf[2] = (buf[2]>>1)&0x7F;
      }
      buf += 3;
    }
  }
}

void TileItem_ColorKey24::Empty(int start_x, int end_x, uint8_t* buf)
{
  if( start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    buf += 3*start_x;
    while (end_x--) {
      *(buf++) = 0xFF;
      *(buf++) = 0x00;
      *(buf++) = 0xFF;
    }
  }
}

void TileItem_ColorKey24::ScalePreview(uint8_t* out, int x, uint opitch, uint shift)
{
  const uint8_t *idata  = m_surface.GetPixels();
  uint           ipitch = m_surface.GetPitch();
  Point2i        start  = m_start_check>>shift;
  Point2i        end    = (m_end_check + (1<<shift) -1)>>shift;

  out   += 4*(x<<(CELL_BITS-shift)) + start.y*opitch;
  idata += (start.y<<shift)*ipitch;

  for (int j=start.y; j<end.y; j++) {
    for (int i=start.x; i<end.x; i++) {
      const uint8_t* ptr = idata + 3*(i<<shift);
      uint count = 0, p0 = 0, p1 = 0, p2 = 0;

      for (uint u=0; u<(1U<<shift); u++) {
        for (uint v=0; v<(1U<<shift); v++) {
          const uint8_t* pix = ptr + 3*v;
          if (pix[0]!=0xFF || pix[1] || pix[2]!=0xFF) {
            p0 += (uint)pix[0];
            p1 += (uint)pix[1];
            p2 += (uint)pix[2];
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

  if (m_need_check_empty)
    CheckEmpty();
}

// === Implemenation of TileItem_SoftwareAlpha ==============================
TileItem_AlphaSoftware::TileItem_AlphaSoftware(uint8_t alpha_threshold)
  : TileItem_NonEmpty(alpha_threshold)
{
  m_surface = Surface(CELL_SIZE, SDL_SWSURFACE|SDL_SRCALPHA, true).DisplayFormatAlpha();
}

void TileItem_AlphaSoftware::ForceEmpty()
{
  TileItem_NonEmpty::ForceEmpty();
  m_surface.Fill(0);
}

TileItem_AlphaSoftware::TileItem_AlphaSoftware(void *pixels, int pitch, uint8_t alpha_threshold)
  : TileItem_NonEmpty(alpha_threshold)
{
  SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(pixels, CELL_SIZE.x, CELL_SIZE.y, 32, pitch,
                                               0xFF0000, 0xFF00, 0xFF, 0xFF000000);
  // Required to have a copy of the area
  m_surface = Surface(SDL_DisplayFormatAlpha(surf));
  SDL_FreeSurface(surf);
}

void TileItem_AlphaSoftware::ScalePreview(uint8_t *odata, int x, uint opitch, uint shift)
{
  const Uint8 *idata  = m_surface.GetPixels();
  uint         ipitch = m_surface.GetPitch();
  Point2i      start  = m_start_check>>shift;
  Point2i      end    = (m_end_check + (1<<shift) -1)>>shift;
  uint         p0, p1, p2, p3;

  //printf("Converted (%i,%i)->(%i,%i) to (%i,%i)->(%i,%i)\n",
  //       m_start_check.x, m_start_check.y, m_end_check.x, m_end_check.y,
  //       start.x, start.y, end.x, end.y);

  odata += (x<<(2+CELL_BITS-shift)) + start.y*opitch;
  idata += (start.y<<shift)*ipitch;

  for (int j=start.y; j<end.y; j++) {
    for (int i=start.x; i<end.x; i++) {
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

  if (m_need_check_empty)
    CheckEmpty();
}

void TileItem_AlphaSoftware::Empty(int start_x, int end_x, uint8_t* buf)
{
  if (start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    buf += start_x * 4;
    memset(buf, 0 , 4 * end_x);
  }
}

void TileItem_AlphaSoftware::Dig(const Point2i &position, const Surface& dig)
{
  m_start_check.SetValues(position.max(Point2i(0, 0)));
  m_end_check.SetValues(m_surface.GetSize().min(position+dig.GetSize()));

  Uint32 *ptr    = (Uint32 *)m_surface.GetPixels();
  int     pitch  = m_surface.GetPitch()>>2;

  ptr += m_start_check.y*pitch;

  for (int py = m_start_check.y ; py < m_end_check.y ; py++) {
    for (int px = m_start_check.x ; px < m_end_check.x ; px++) {
      if (dig.GetPixel(px-position.x, py-position.y) != 0)
        ptr[px] = 0;
    }
    ptr += pitch;
  }
}

void TileItem_AlphaSoftware::Darken(int start_x, int end_x, uint8_t* buf)
{
  if (start_x < CELL_SIZE.x && end_x >= 0) {
    //Clamp the value to empty only the in this tile
    start_x = (start_x < 0) ? 0 : (start_x >= CELL_SIZE.x) ? CELL_SIZE.x - 1 : start_x;
    end_x = (end_x >= CELL_SIZE.x) ? CELL_SIZE.x - start_x : end_x - start_x + 1;

    ASSERT(m_surface.GetSurface()->format->Amask == 0xFF000000);

    uint32_t *ptr = (uint32_t*)buf;
    ptr += start_x;
    while(end_x--) {
      uint32_t s = ptr[0];
      // Mask component MSBs and alpha, then readd alpha
      // We have force the mask
      ptr[0] = ((s>>1)&0x007F7F7F) | (s&0xFF000000);
      ptr++;
    }
  }
}

bool TileItem_AlphaSoftware::CheckEmpty()
{
  const Uint32 *ptr   = (Uint32 *)m_surface.GetPixels();
  int           pitch = m_surface.GetPitch()>>2;
  uint8_t      *buf   = m_empty_bitfield;
  int           sx    = m_start_check.x&0xFFFFFFF0,
                ex    = (m_end_check.x+7)&0xFFFFFFF0;

  m_is_empty = true;
  m_need_check_empty = false;

  buf += (sx + m_start_check.y*CELL_SIZE.x)>>8;
  for (int py=m_start_check.y; py<m_end_check.y; py++) {
    for (int px=sx; px<ex; px+=8, buf++) {
      uint8_t mask_empty = 0;

      for (int i=0; i<8; i++) {
        if ((ptr[px+i]&0xFF000000)>>24 < m_alpha_threshold)
          mask_empty |= 1<<i;
      }

      if (mask_empty != 0xFF)
        m_is_empty = false;
      buf[0] = mask_empty;
    }

    ptr += pitch;
    buf += CELL_SIZE.x>>8;
  }

  // Make sure it is empty
  if (m_is_empty)
    CheckEmptyField();
  return m_is_empty;
}
