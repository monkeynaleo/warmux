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
 *  MERCHANTABILITY or FITNESS FOR A ARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU GeneralPublic License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Handle a SDL Surface
 *****************************************************************************/

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <iostream>
#include <png.h>

#include "graphic/surface.h"
#include "tool/math_tools.h"

/* texturedPolygon import from SDL_gfx v2.0.15 */
#if (SDL_GFXPRIMITIVES_MAJOR == 2) && (SDL_GFXPRIMITIVES_MINOR == 0) && (SDL_GFXPRIMITIVES_MICRO < 14)
#include "graphic/textured_polygon.h"
#endif /* texturedPolygon import from SDL_gfx v2.0.15 */

#include "graphic/fading_effect.h"

#define BUGGY_SDLGFX 1

/**
 * Constructor building a surface by reading the image from a file.
 *
 * @param filename_str A string containing the path to the graphic file.
 */
Surface::Surface(const std::string &filename)
{
  surface = NULL;
  autoFree = true;
  if (!ImgLoad(filename))
    Error(Format("Unable to open image file '%s': %s", filename.c_str(), IMG_GetError()));
}

/**
 * Copy constructor: build a surface from an other surface.
 *
 * The two surfaces share the same graphic data.
 */
Surface::Surface(const Surface &src)
{
  surface = src.surface;
  autoFree = true;
  if (!IsNull())
    surface->refcount++;
}

Surface &Surface::operator=(const Surface & src)
{
  AutoFree();
  surface = src.surface;
  autoFree = true;
  if (!IsNull())
    surface->refcount++;

  return *this;
}

/**
 * Free the memory occupied by the surface.
 *
 * The memory is really freed if the reference counter reach 0.
 */
void Surface::Free()
{
  if (!IsNull()) {
    SDL_FreeSurface(surface);
    surface = NULL;
  }
}

/**
 * Create a new surface.
 *
 * @param size
 * @param flags
 * @param useAlpha
 */
void Surface::NewSurface(const Point2i &size, Uint32 flags, bool useAlpha)
{
  Uint32 alphaMask;
  Uint32 redMask;
  Uint32 greenMask;
  Uint32 blueMask;

  if (autoFree)
    Free();

  const SDL_PixelFormat* fmt = SDL_GetVideoSurface()->format;

  // If no alpha, use default parameters
  if (!useAlpha) {
    alphaMask = 0;
    surface = SDL_CreateRGBSurface(flags, size.x, size.y,
                                   fmt->BitsPerPixel, 0, 0, 0, 0);

    if (!surface)
      Error(std::string("Can't create SDL RGBA surface: ") + SDL_GetError());
 } else {
    // Try using display format mask
    if (fmt->BitsPerPixel >= 24) {
      redMask   = fmt->Rmask;
      greenMask = fmt->Gmask;
      blueMask  = fmt->Bmask;
    } else {
#if SDL_BYTEORDER != SDL_LIL_ENDIAN
      redMask   = 0xff000000;
      greenMask = 0x00ff0000;
      blueMask  = 0x0000ff00;
#else
      redMask   = 0x000000ff;
      greenMask = 0x0000ff00;
      blueMask  = 0x00ff0000;
#endif
    }

    // Use the mask most obvious from the others masks
    if (redMask==0xff || blueMask==0xff || greenMask==0xff)
      alphaMask = 0xff000000;
    else
      alphaMask = 0x000000ff;

    surface = SDL_CreateRGBSurface(flags, size.x, size.y, 32,
                                   redMask, greenMask, blueMask, alphaMask);


    if (!surface)
      Error(std::string("Can't create SDL RGBA surface: ") + SDL_GetError());
  }
}

/**
 * Set the alpha value of a surface.
 *
 */
int Surface::SetAlpha(Uint32 flags, Uint8 alpha)
{
  return SDL_SetAlpha(surface, flags, alpha);
}

/**
 * Lock the surface to permit direct access.
 *
 */
int Surface::Lock()
{
  return SDL_LockSurface(surface);
}

/**
 * Unlock the surface.
 *
 */
void Surface::Unlock()
{
  SDL_UnlockSurface(surface);
}

void Surface::SwapClipRect(Rectanglei& rect)
{
  SDL_Rect newClipRect = GetSDLRect(rect);
  SDL_Rect oldClipRect;

  SDL_GetClipRect(surface, &oldClipRect);
  SDL_SetClipRect(surface, &newClipRect);

  rect.SetPositionX(oldClipRect.x);
  rect.SetPositionY(oldClipRect.y);
  rect.SetSizeX(oldClipRect.w);
  rect.SetSizeY(oldClipRect.h);
}


int Surface::Blit(const Surface& src, SDL_Rect *srcRect, SDL_Rect *dstRect)
{
  return SDL_BlitSurface(src.surface, srcRect, surface, dstRect);
}

/**
 * Blit a surface (src) on the current surface at a certain position (dst)
 *
 * @src The source surface.
 * @dst A point defining the destination coordinate on the current surface.
 */
int Surface::Blit(const Surface& src, const Point2i &dst)
{
  SDL_Rect dstRect = GetSDLRect(dst);

  return Blit(src, NULL, &dstRect);
}

/**
 * Blit a part (srcRect) of surface (src) at a certaint position (dst) of the current surface/
 *
 * @param src
 * @param srcRect
 * @param dstPoint
 */
int Surface::Blit(const Surface& src, const Rectanglei &srcRect, const Point2i &dstPoint)
{
  SDL_Rect sdlSrcRect = GetSDLRect(srcRect);
  SDL_Rect sdlDstRect = GetSDLRect(dstPoint);

  return Blit(src, &sdlSrcRect, &sdlDstRect);
}

/**
 * Merge a sprite (spr) with current Surface at a given position.
 *
 * No more buggy but slow ! :) Don't use it for quick blit. Needed by the ground generator.
 *
 * @param spr
 * @param position
 */
void Surface::MergeSurface(Surface &spr, const Point2i &pos)
{
  SDL_PixelFormat* cur_fmt = surface->format;
  SDL_PixelFormat * spr_fmt = spr.surface->format;

  spr.Lock();
  Lock();

  // for each pixel lines of a source image
  if (cur_fmt->BytesPerPixel == spr_fmt->BytesPerPixel && cur_fmt->BytesPerPixel == 4) {
    int     cur_pitch = (surface->pitch>>2);
    Uint32* cur_ptr   = (Uint32*)surface->pixels;
    int     spr_pitch = (spr.surface->pitch>>2);
    Uint32* spr_ptr   = (Uint32*)spr.surface->pixels;
    // shift necessary to move the RGB triplet into the LSBs
    Uint32  spr_pix, cur_pix, a, p_a;
    Point2i offset;

    offset.y = (pos.y > 0) ? 0 : -pos.y;

    cur_ptr += pos.x + (pos.y + offset.y) * cur_pitch;
    spr_ptr += offset.y * spr_pitch;

    // Same masks: use more optimized version
    if (cur_fmt->Amask == spr_fmt->Amask) {
      Uint32  ashift    = cur_fmt->Ashift;
      Uint32  amask     = cur_fmt->Amask;
      // shift necessary to move the RGB triplet into the LSBs
      Uint32  shift     = (ashift) ? 0 : 8;

      for (; offset.y < spr.GetHeight() && pos.y + offset.y < GetHeight(); offset.y++) {
        for (offset.x = (pos.x > 0 ? 0 : -pos.x); offset.x < spr.GetWidth() && pos.x + offset.x < GetWidth(); offset.x++) {
          // Retrieving a pixel of sprite to merge
          spr_pix = spr_ptr[offset.x];
          cur_pix = cur_ptr[offset.x];

          a   = (spr_pix&amask)>>ashift;
          p_a = (cur_pix&amask)>>ashift;

          if (a == SDL_ALPHA_OPAQUE || (!p_a && a)) // new pixel with no alpha or nothing on previous pixel
            cur_ptr[offset.x] = spr_pix;
          else if (a) { // alpha is lower => merge color with previous value
            uint f_a  = a + 1;
            uint f_ca = 256 - f_a;

            // A will be discarded either by this shift or the bitmasks used
            cur_pix >>= shift;
            spr_pix >>= shift;
            // Only do 2 components at a time, and avoid one component overflowing
            // to bleed into other components
            Uint32 tmp = ((cur_pix&0xFF00FF)*f_ca + (spr_pix&0xFF00FF)*f_a)>>8;
            tmp &= 0xFF00FF;

            tmp |= (((cur_pix&0xFF00)*f_ca + (spr_pix&0xFF00)*f_a)>>8) & 0xFF00;

            a = (a > p_a) ? a : p_a;
            cur_ptr[offset.x] = (tmp<<shift) | (a<<ashift);
          }
        }

        spr_ptr += spr_pitch;
        cur_ptr += cur_pitch;
      }
    } else {
      // Troublesome masks: use generic version
      for (; offset.y < spr.GetHeight() && pos.y + offset.y < GetHeight(); offset.y++) {
        for (offset.x = (pos.x > 0 ? 0 : -pos.x); offset.x < spr.GetWidth() && pos.x + offset.x < GetWidth(); offset.x++) {
          // Retrieving a pixel of sprite to merge
          spr_pix = spr_ptr[offset.x];
          cur_pix = cur_ptr[offset.x];

          a   = (spr_pix&spr_fmt->Amask)>>spr_fmt->Ashift;
          p_a = (cur_pix&cur_fmt->Amask)>>cur_fmt->Ashift;

          if (a == SDL_ALPHA_OPAQUE || (!p_a && a)) {
            // new pixel with no alpha or nothing on previous pixel
            cur_ptr[offset.x] = spr_pix;
          } else if (a) {
            // alpha is lower => merge color with previous value
            uint f_a  = a + 1;
            uint f_ca = 256 - f_a;

            Uint32 r = (((cur_pix&cur_fmt->Rmask)>>cur_fmt->Rshift)*f_ca +
                        ((spr_pix&spr_fmt->Rmask)>>spr_fmt->Rshift)*f_a)>>8;
            Uint32 g = (((cur_pix&cur_fmt->Gmask)>>cur_fmt->Gshift)*f_ca +
                        ((spr_pix&spr_fmt->Gmask)>>spr_fmt->Gshift)*f_a)>>8;
            Uint32 b = (((cur_pix&cur_fmt->Bmask)>>cur_fmt->Bshift)*f_ca +
                        ((spr_pix&spr_fmt->Bmask)>>spr_fmt->Bshift)*f_a)>>8;

            a = (a > p_a) ? a : p_a;
            cur_ptr[offset.x] = (r<<cur_fmt->Rshift)|(g<<cur_fmt->Gshift)|
                                (b<<cur_fmt->Bshift)|(a<<cur_fmt->Ashift);
          }
        }

        spr_ptr += spr_pitch;
        cur_ptr += cur_pitch;
      }
    }
  } else {
    fprintf(stderr, "Not handling: spr=(bpp=%u,rmask=%X) vs surf=(bpp=%u,rmask=%X)\n",
            spr_fmt->BytesPerPixel, spr_fmt->Rmask, cur_fmt->BytesPerPixel, cur_fmt->Rmask);
    Blit(spr, pos);
  }

  Unlock();
  spr.Unlock();
}

int Surface::SetColorKey(Uint32 flag, Uint32 key)
{
  return SDL_SetColorKey(surface, flag, key);
}

void Surface::GetRGBA(Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) const
{
  SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);
}

Uint32 Surface::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const
{
  return SDL_MapRGBA(surface->format, r, g, b, a);
}

Color Surface::GetColor(Uint32 color) const
{
  Uint8 r, g, b, a;
  GetRGBA(color, r, g, b, a);
  return Color(r, g, b, a);
}

Uint32 Surface::MapColor(const Color& color) const
{
  return MapRGBA(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

void Surface::Flip()
{
  SDL_Flip(surface);
}

  int Surface::BoxColor(const Rectanglei &rect, const Color &color)
{
  if (rect.IsSizeZero())
    return 0;

  Point2i ptBR = rect.GetBottomRightPoint();

  return boxRGBA(surface, rect.GetPositionX(), rect.GetPositionY(), ptBR.GetX(), ptBR.GetY(), color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::RectangleColor(const Rectanglei &rect, const Color &color,
                            const uint &border_size)
{
  if (rect.IsSizeZero())
    return 0;

  Point2i ptBR = rect.GetBottomRightPoint();

  if (border_size == 1)
    return rectangleRGBA(surface, rect.GetPositionX(), rect.GetPositionY(), ptBR.GetX(), ptBR.GetY(), color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());

  // top border
  boxRGBA (surface,
           rect.GetPositionX(), rect.GetPositionY(), ptBR.GetX(), rect.GetPositionY()+border_size,
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());

  // bottom border
  boxRGBA (surface,
           rect.GetPositionX(), ptBR.GetY() - border_size, ptBR.GetX(), ptBR.GetY(),
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());

  // left border
  boxRGBA (surface,
           rect.GetPositionX(), rect.GetPositionY() + border_size, rect.GetPositionX()+border_size, ptBR.GetY()-border_size,
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());

  // right border
  boxRGBA (surface,
           ptBR.GetX() - border_size, rect.GetPositionY() + border_size, ptBR.GetX(), ptBR.GetY()-border_size,
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());

  return 1;
}

int Surface::VlineColor(const uint &x, const uint &y1, const uint &y2, const Color &color)
{
  return vlineRGBA(surface, x, y1, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::HlineColor(const uint &x1, const uint &x2, const uint &y, const Color &color)
{
  return hlineRGBA(surface, x1, x2, y, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::LineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color)
{
  return lineRGBA(surface, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::AALineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color)
{
  return aalineRGBA(surface, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::AAFadingLineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color1, const Color &color2)
{
  return aafadingLineColor(surface, x1, y1, x2, y2,color1.GetColor(), color2.GetColor());
}

int Surface::CircleColor(const uint &x, const uint &y, const uint &rad, const Color &color)
{
  return circleRGBA(surface, x, y, rad, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::FilledCircleColor(const uint &x, const uint &y, const uint &rad, const Color &color)
{
  return filledCircleRGBA(surface, x, y, rad, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::PieColor(const uint &x, const uint &y, const uint &rad, const int &start, const int &end, const Color &color)
{
  return pieRGBA(surface, x, y, rad, start, end, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::FilledPieColor(const uint &x, const uint &y, const uint &rad, const int &start, const int &end, const Color &color)
{
  return filledPieRGBA(surface, x, y, rad, start, end, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::AAPolygonColor(const Sint16 * vx, const Sint16 * vy, const int n, const Color & color) {
  return aapolygonRGBA(surface, vx, vy, n, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::AAPolygonColor(std::list<Point2i> polygon, const Color & color)
{
  Sint16 * vx, * vy;
  vx = new Sint16[polygon.size()];
  vy = new Sint16[polygon.size()];
  int i = 0;
  for (std::list<Point2i>::iterator point = polygon.begin(); point != polygon.end(); point++, i++) {
    vx[i] = point->x;
    vy[i] = point->y;
  }
  int result = aapolygonRGBA(surface, vx, vy, polygon.size(), color.GetRed(),
                             color.GetGreen(), color.GetBlue(), color.GetAlpha());
  delete[] vx;
  delete[] vy;
  return result;
}

int Surface::FilledPolygon(const Sint16 * vx, const Sint16 * vy, const int n, const Color & color)
{
  return filledPolygonRGBA(surface, vx, vy, n, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::FilledPolygon(std::list<Point2i> polygon, const Color & color)
{
  Sint16 * vx, * vy;
  vx = new Sint16[polygon.size()];
  vy = new Sint16[polygon.size()];
  int i = 0;
  for (std::list<Point2i>::iterator point = polygon.begin(); point != polygon.end(); point++, i++) {
    vx[i] = point->x;
    vy[i] = point->y;
  }
  int result = filledPolygonRGBA(surface, vx, vy, polygon.size(), color.GetRed(),
                                 color.GetGreen(), color.GetBlue(), color.GetAlpha());
  delete[] vx;
  delete[] vy;
  return result;
}

int Surface::TexturedPolygon(const Sint16 * vx, const Sint16 * vy, const int n, const Surface *texture, const int texture_dx, const int texture_dy)
{
  return texturedPolygon(surface, vx, vy, n, texture->surface, texture_dx, texture_dy);
}

int Surface::TexturedPolygon(std::list<Point2i> polygon, const Surface * texture)
{
  Sint16 * vx, * vy;
  vx = new Sint16[polygon.size()];
  vy = new Sint16[polygon.size()];
  int i = 0;
  for (std::list<Point2i>::iterator point = polygon.begin(); point != polygon.end(); point++, i++) {
    vx[i] = point->x;
    vy[i] = point->y;
  }
  int result = texturedPolygon(surface, vx, vy, polygon.size(), texture->surface, 0, 0);
  delete[] vx;
  delete[] vy;
  return result;
}

/**
 *
 * @param color
 */
int Surface::Fill(Uint32 color) const
{
  return SDL_FillRect(surface, NULL, color);
}

int Surface::Fill(const Color &color) const
{
  return Fill(MapColor(color));
}

/**
 *
 * @param dstRect
 * @param color
 */
int Surface::FillRect(const Rectanglei &dstRect, Uint32 color) const
{
  SDL_Rect sdlDstRect = GetSDLRect(dstRect);

  return SDL_FillRect(surface, &sdlDstRect, color);
}

/**
 *
 * @param dstRect
 * @param color
 */
int Surface::FillRect(const Rectanglei &dstRect, const Color &color) const
{
  return FillRect(dstRect, MapColor(color));
}

/**
 *
 * @param filename
 */
int Surface::ImgLoad(const std::string& filename)
{
  AutoFree();
  surface = IMG_Load(filename.c_str());

  return !IsNull();
}

/**
 *
 * @param filename
 */
#ifdef WIN32
#  define alloca _alloca
#endif
bool Surface::ImgSave(const std::string& filename)
{
  FILE            *f        = NULL;
  png_structp      png_ptr  = NULL;
  png_infop        info_ptr = NULL;
  SDL_PixelFormat *spr_fmt  = surface->format;
  bool             ret      = false;
  Uint8           *tmp_line = NULL;

  // Creating a png ...
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) // Structure and ...
    return 1;
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) // Information.
    goto end;

  // Opening a new file
  f = fopen(filename.c_str(), "wb");
  if (f == NULL)
    goto end;

  png_init_io(png_ptr, f); // Associate png struture with a file
  png_set_IHDR(png_ptr, info_ptr, surface->w, surface->h, 8,
               PNG_COLOR_TYPE_RGB_ALPHA,      PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,  PNG_FILTER_TYPE_DEFAULT);
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  // Creating the png file
  png_write_info(png_ptr, info_ptr);

  tmp_line = SDL_stack_alloc(Uint8, surface->w * spr_fmt->BytesPerPixel); // alloca
  Lock();
  for (int y = 0; y < surface->h; y++) {
    for (int x = 0; x < surface->w; x++) {
      Uint8   r, g, b, a;
      // Retrieving a pixel of sprite to merge
      Uint32  spr_pix = ((Uint32*)surface->pixels)[y * surface->w  + x];

      // Retreiving each chanel of the pixel using pixel format
      SDL_GetRGBA(spr_pix, surface->format, &r, &g, &b, &a);
      tmp_line[x * spr_fmt->BytesPerPixel + 0] = r;
      tmp_line[x * spr_fmt->BytesPerPixel + 1] = g;
      tmp_line[x * spr_fmt->BytesPerPixel + 2] = b;
      tmp_line[x * spr_fmt->BytesPerPixel + 3] = a;
    }
    png_write_row(png_ptr, (Uint8 *)tmp_line);
  }
  Unlock();
  SDL_stack_free(tmp_line);
  png_write_flush(png_ptr);
  png_write_end(png_ptr, info_ptr);
  ret = true;

end:
  if (info_ptr) png_destroy_info_struct(png_ptr, &info_ptr);
  if (png_ptr) png_destroy_write_struct(&png_ptr, NULL);
  if (f) fclose(f);
  return ret;
}

/**
 *
 * @param angle in radian
 * @param zoomx
 * @param zoomy
 * @param smooth
* Warning rotozoomSurfaceXY uses degrees so the rotation of image use degrees here,
* but when accessing thanks to GetSurfaceForAngle the index is using radian
* (because we juste need an index in array, not an angle) */
static const Double ratio_deg_to_rad = 180 / PI;
Surface Surface::RotoZoom(Double angle, Double zoomx, Double zoomy, int smooth)
{
  SDL_Surface *surf;

#ifdef BUGGY_SDLGFX
  /* From SDLGFX website,
   * 'zoomx' and 'zoomy' are scaling factors that
   * can also be negative. In this case the corresponding axis is flipped.
   * Note: Flipping currently only works with antialiasing turned off
   */
  if (zoomx < ZERO || zoomy < ZERO)
    smooth = SMOOTHING_OFF;
#endif

  if (EqualsZero(angle)) {
    if (zoomx!=Double(1) || zoomy!=Double(1))
      surf = zoomSurface(surface, zoomx.toDouble(), zoomy.toDouble(), smooth);
    else {
      return *this;
    }
  } else if (zoomx == zoomy && zoomx > ZERO) {
    surf = rotozoomSurface(surface, (angle * ratio_deg_to_rad).toDouble() , zoomx.toDouble(), smooth);
  } else {
    surf = rotozoomSurfaceXY(surface, (angle * ratio_deg_to_rad).toDouble() , zoomx.toDouble(), zoomy.toDouble(), smooth);
  }

  if (!surf)
    Error("Unable to make a rotozoom on the surface !");

  return Surface(surf);
}

/**
 *
 */
Surface Surface::DisplayFormatAlpha()
{
  switch (surface->format->BitsPerPixel)
  {
  case 24:
    return DisplayFormat();
  case 8: // paletted
    {
#if 0
      Surface nsurf = DisplayFormat();
      SDL_Color key = surface->format->palette->colors[surface->format->colorkey];

      nsurf.SetColorKey(SDL_SRCCOLORKEY,
                        SDL_MapRGB(SDL_GetVideoSurface()->format, key.r, key.g, key.b));
      printf("Using colorkey (%u,%u,%u)\n", key.r, key.g, key.b);

      return nsurf;
#else
      return *this;
#endif
    }
  default:
    {
      SDL_Surface *surf = SDL_DisplayFormatAlpha(surface);

      if (!surf)
        Error("Unable to convert the surface to a surface compatible with the display format with alpha.");

      return Surface(surf);
    }
  }
}

/**
 *
 */
Surface Surface::DisplayFormat()
{
  SDL_Surface *surf = SDL_DisplayFormat(surface);

  if (!surf)
    Error("Unable to convert the surface to a surface compatible with the display format.");

  return Surface(surf);
}


/**
 * GetPixel.
 *
 * From the SDL wiki.
 * @param x
 * @param y
 */
Uint32 Surface::GetPixel(int x, int y) const
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
  case 1:
    return *p;

  case 2:
    return *(Uint16 *)p;

  case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return p[0] << 16 | p[1] << 8 | p[2];
#else
    return p[0] | p[1] << 8 | p[2] << 16;
#endif
  case 4:
    return *(Uint32 *)p;

  default:
    Error("Unknow bpp!");
    return 0;   // To make gcc happy
  }
}

/**
 *
 * @param x
 * @param y
 * @param pixel
 */
void Surface::PutPixel(int x, int y, Uint32 pixel) const
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
  case 1:
    *p = pixel;
    break;

  case 2:
    *(Uint16 *)p = pixel;
    break;

  case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    p[0] = (pixel >> 16) & 0xff;
    p[1] = (pixel >> 8) & 0xff;
    p[2] = pixel & 0xff;
#else
    p[0] = pixel & 0xff;
    p[1] = (pixel >> 8) & 0xff;
    p[2] = (pixel >> 16) & 0xff;
#endif
    break;

  case 4:
    *(Uint32 *)p = pixel;
    break;
  }
}

SDL_Rect Surface::GetSDLRect(const Rectanglei &r)
{
  SDL_Rect sdlRect;

  sdlRect.x = r.GetPositionX();
  sdlRect.y = r.GetPositionY();
  sdlRect.w = r.GetSizeX();
  sdlRect.h = r.GetSizeY();

  return sdlRect;
}

SDL_Rect Surface::GetSDLRect(const Point2i &pt)
{
  SDL_Rect sdlRect;

  sdlRect.x = pt.GetX();
  sdlRect.y = pt.GetY();
  sdlRect.w = 0;
  sdlRect.h = 0;

  return sdlRect;
}

/**
 * Set an alpha . This's alpha is set to red value off mask
 *
 * @param mask
 * @param position
 */
void Surface::MergeAlphaSurface(const Surface &mask, const Point2i &pos)
{
  int dest_x = pos.x;
  int dest_y = pos.y;
  int size_x = mask.GetWidth();
  int size_y = mask.GetHeight();
  int mask_x = 0;
  int mask_y = 0;

  if (dest_x<0) {
    dest_x = 0;
    mask_x = -pos.x;
  }
  if (dest_y<0) {
    dest_y = 0;
    mask_y = -pos.y;
  }

  if (size_x+dest_x > GetWidth()) {
    size_x = GetWidth()- dest_x;
  }

  if (size_y+dest_y > GetHeight()) {
    size_y = GetHeight() - dest_y;
  }

  Uint32 temp_pix;
  Uint32 mask_color, this_color;
  Uint8  mask_r, mask_g, mask_b, mask_a;
  Uint8  this_r, this_g, this_b, this_a;

  for (int i=0; i<size_x; i++) {
    for (int j=0; j<size_y; j++) {
      //Copy red pixel of mask in alpha pixel of this
      mask_color  = mask.GetPixel(i + mask_x, j + mask_y);
      this_color = GetPixel(i + dest_x, j + dest_y) ;

      GetRGBA(this_color, this_r, this_g, this_b, this_a);
      mask.GetRGBA(mask_color,mask_r,mask_g,mask_b,mask_a);

      if (mask_r < this_a) {
        temp_pix = MapRGBA(this_r, this_g, this_b, mask_r);
        PutPixel(i+ dest_x, j + dest_y, temp_pix);
      }

    }
  }
}
