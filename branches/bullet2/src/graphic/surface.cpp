/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * Default constructor.
 *
 * Build a null surface with autoFree at true.
 */
Surface::Surface()
{
  surface = NULL;
  autoFree = true;
}

/**
 * Constructor building a surface object using an existing SDL_Surface pointer.
 *
 * @param sdl_surface The existing sdl_surface.
 */
Surface::Surface(SDL_Surface *sdl_surface)
{
  surface = sdl_surface;
  autoFree = true;
}

/**
 * Constructor building a surface object using the NewSurface function.
 *
 * @param size
 * @param flags
 * @param useAlpha
 * @see NewSurface
 */
Surface::Surface(const Point2i &size, Uint32 flags, bool useAlpha)
{
  surface = NULL;
  autoFree = true;
  NewSurface(size, flags, useAlpha);
}

/**
 * Constructor building a surface by reading the image from a file.
 *
 * @param filename_str A string containing the path to the graphic file.
 */
Surface::Surface(const std::string &filename)
{
  surface = NULL;
  autoFree = true;
  if( !ImgLoad(filename) )
    Error( Format("Unable to open image file '%s': %s", filename.c_str(), IMG_GetError() ) );
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
  if( !IsNull() )
    surface->refcount++;
}

/**
 * Destructor of the surface.
 *
 * Will free the memory used by the surface if autoFree is set to true and if the counter of reference reach 0
 */
Surface::~Surface()
{
  AutoFree();
}

bool Surface::IsNull() const
{
  return surface == NULL;
}

/**
 * Return the size of a surface.
 */
Point2i Surface::GetSize() const
{
  return Point2i( GetWidth(), GetHeight() );
}

Uint32 Surface::GetFlags() const
{
  return surface->flags;
}

/// Return the length of a surface scanline in bytes.
Uint16 Surface::GetPitch() const
{
  return surface->pitch;
}

/// Return the number of bytes used to represent each pixel in a surface. Usually one to four.
Uint8 Surface::GetBytesPerPixel() const
{
  return surface->format->BytesPerPixel;
}

/// Return a pointer on the pixels data.
unsigned char *Surface::GetPixels() const
{
  return (unsigned char *) surface->pixels;
}

Surface &Surface::operator=(const Surface & src)
{
  AutoFree();
  surface = src.surface;
  autoFree = true;
  if( !IsNull() )
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
  if( !IsNull() ){
    SDL_FreeSurface( surface );
    surface = NULL;
  }
}

void Surface::AutoFree()
{
  if( autoFree )
    Free();
}

/**
 * Set the auto free status of a surface.
 *
 * In general it should always be true for non-system surface.
 * @param newAutoFree the new autoFree status.
 */
void Surface::SetAutoFree( bool newAutoFree )
{
  autoFree = newAutoFree;
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

  if( autoFree )
    Free();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
  redMask   = 0xff000000;
  greenMask = 0x00ff0000;
  blueMask  = 0x0000ff00;
  alphaMask = 0x000000ff;
#else
  redMask   = 0x000000ff;
  greenMask = 0x0000ff00;
  blueMask  = 0x00ff0000;
  alphaMask = 0xff000000;
#endif

  if( !useAlpha )
    alphaMask = 0;

  surface = SDL_CreateRGBSurface(flags, size.x, size.y, 32,
                                 redMask, greenMask, blueMask, alphaMask );

  if( surface == NULL )
    Error( std::string("Can't create SDL RGBA surface: ") + SDL_GetError() );
}

/**
 * Set the alpha value of a surface.
 *
 */
int Surface::SetAlpha(Uint32 flags, Uint8 alpha)
{
  return SDL_SetAlpha( surface, flags, alpha );
}

/**
 * Lock the surface to permit direct access.
 *
 */
int Surface::Lock()
{
  return SDL_LockSurface( surface );
}

/**
 * Unlock the surface.
 *
 */
void Surface::Unlock()
{
  SDL_UnlockSurface( surface );
}

int Surface::Blit(const Surface& src, SDL_Rect *srcRect, SDL_Rect *dstRect)
{
  return SDL_BlitSurface( src.surface, srcRect, surface, dstRect );
}

/**
 * Blit the whole surface src on the current surface.
 *
 * @param src The source surface.
 */
int Surface::Blit(const Surface& src)
{
  return Blit(src, NULL, NULL);
}

/**
 * Blit a surface (src) on the current surface at a certain position (dst)
 *
 * @src The source surface.
 * @dst A point defining the destination coordinate on the current surface.
 */
int Surface::Blit(const Surface& src, const Point2i &dst)
{
  SDL_Rect dstRect = GetSDLRect( dst );;

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
  SDL_Rect sdlSrcRect = GetSDLRect( srcRect );
  SDL_Rect sdlDstRect = GetSDLRect( dstPoint );

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
void Surface::MergeSurface( Surface &spr, const Point2i &pos)
{
  Uint32 spr_pix, cur_pix;
  Uint8 r, g, b, a, p_r, p_g, p_b, p_a;
  float f_a, f_ca, f_pa;
  SDL_PixelFormat * current_fmt = surface->format;
  SDL_PixelFormat * spr_fmt = spr.surface->format;
  int current_offset, spr_offset;
  Point2i offset;

  spr.Lock();
  Lock();

  // for each pixel lines of a source image
  for (offset.x = (pos.x > 0 ? 0 : -pos.x); offset.x < spr.GetWidth() && pos.x + offset.x < GetWidth(); offset.x++) {
    for (offset.y = (pos.y > 0 ? 0 : -pos.y); offset.y < spr.GetHeight() && pos.y + offset.y < GetHeight(); offset.y++) {

      // Computing offset on both sprite
      current_offset = (pos.y + offset.y) * surface->w + pos.x + offset.x;
      spr_offset = offset.y * spr.surface->w + offset.x;

      // Retrieving a pixel of sprite to merge
      spr_pix = ((Uint32*)spr.surface->pixels)[spr_offset];
      cur_pix = ((Uint32*)surface->pixels)[current_offset];

      // Retreiving each chanel of the pixel using pixel format
      r = (Uint8)(((spr_pix & spr_fmt->Rmask) >> spr_fmt->Rshift) << spr_fmt->Rloss);
      g = (Uint8)(((spr_pix & spr_fmt->Gmask) >> spr_fmt->Gshift) << spr_fmt->Gloss);
      b = (Uint8)(((spr_pix & spr_fmt->Bmask) >> spr_fmt->Bshift) << spr_fmt->Bloss);
      a = (Uint8)(((spr_pix & spr_fmt->Amask) >> spr_fmt->Ashift) << spr_fmt->Aloss);

      // Retreiving previous alpha value
      p_a = (Uint8)(((cur_pix & current_fmt->Amask) >> current_fmt->Ashift) << current_fmt->Aloss);

      if (a == SDL_ALPHA_OPAQUE || (p_a == 0 && a >0)) // new pixel with no alpha or nothing on previous pixel
        ((Uint32 *)(surface->pixels))[current_offset] = SDL_MapRGBA(current_fmt, r, g, b, a);
      else if (a > 0) { // alpha is lower => merge color with previous value
        f_a = (float)a / 255.0;
        f_ca = 1.0 - f_a;
        f_pa = (float)p_a / 255.0;

        p_r = (Uint8)(((cur_pix & current_fmt->Rmask) >> current_fmt->Rshift) << current_fmt->Rloss);
        p_g = (Uint8)(((cur_pix & current_fmt->Gmask) >> current_fmt->Gshift) << current_fmt->Gloss);
        p_b = (Uint8)(((cur_pix & current_fmt->Bmask) >> current_fmt->Bshift) << current_fmt->Bloss);

        r = (Uint8)((float)p_r * f_ca * f_pa + (float)r * f_a);
        g = (Uint8)((float)p_g * f_ca * f_pa + (float)g * f_a);
        b = (Uint8)((float)p_b * f_ca * f_pa + (float)b * f_a);

        a = (a > p_a ? a : p_a);
        ((Uint32 *)(surface->pixels))[current_offset] = SDL_MapRGBA(current_fmt, r, g, b, a);
      }
    }
  }
  Unlock();
  spr.Unlock();
}

/**
 *
 * @param flag
 * @param key
 */
int Surface::SetColorKey(Uint32 flag, Uint32 key)
{
  return SDL_SetColorKey( surface, flag, key );
}

/**
 *
 *
 * @param flag
 * @param r
 * @param g
 * @param b
 * @param a
 */
int Surface::SetColorKey(Uint32 flag, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  return SetColorKey( flag, MapRGBA(r, g, b, a) );
}

/**
 * @param color
 * @param r
 * @param g
 * @param b
 * @param a
 */
void Surface::GetRGBA(Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) const
{
  SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);
}

/**
 * @param r
 * @param g
 * @param b
 * @param a
 */
Uint32 Surface::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const
{
  return SDL_MapRGBA(surface->format, r, g, b, a);
}

/**
 *
 * @param color
 */
Color Surface::GetColor(Uint32 color) const
{
  Uint8 r, g, b, a;
  GetRGBA(color, r, g, b, a);
  return Color(r, g, b, a);
}

/**
 *
 * @param color
 */
Uint32 Surface::MapColor(const Color& color) const
{
  return MapRGBA(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

/**
 * @param rect
 */
void Surface::SetClipRect(const Rectanglei &rect)
{
  SDL_Rect sdlRect = GetSDLRect( rect );
  SDL_SetClipRect( surface, &sdlRect );
}

void Surface::Flip()
{
  SDL_Flip( surface );
}

  int Surface::BoxColor(const Rectanglei &rect, const Color &color)
{
  if( rect.IsSizeZero() )
    return 0;

  Point2i ptBR = rect.GetBottomRightPoint();

  return boxRGBA( surface, rect.GetPositionX(), rect.GetPositionY(), ptBR.GetX(), ptBR.GetY(), color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::RectangleColor(const Rectanglei &rect, const Color &color,
                            const uint &border_size)
{
  if( rect.IsSizeZero() )
    return 0;

  Point2i ptBR = rect.GetBottomRightPoint();

  if (border_size == 1)
    return rectangleRGBA( surface, rect.GetPositionX(), rect.GetPositionY(), ptBR.GetX(), ptBR.GetY(), color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

  // top border
  boxRGBA (surface,
           rect.GetPositionX(), rect.GetPositionY(), ptBR.GetX(), rect.GetPositionY()+border_size,
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

  // bottom border
  boxRGBA (surface,
           rect.GetPositionX(), ptBR.GetY() - border_size, ptBR.GetX(), ptBR.GetY(),
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

  // left border
  boxRGBA (surface,
           rect.GetPositionX(), rect.GetPositionY() + border_size, rect.GetPositionX()+border_size, ptBR.GetY()-border_size,
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

  // right border
  boxRGBA (surface,
           ptBR.GetX() - border_size, rect.GetPositionY() + border_size, ptBR.GetX(), ptBR.GetY()-border_size,
           color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

  return 1;
}

int Surface::VlineColor(const uint &x1, const uint &y1, const uint &y2, const Color &color)
{
  return vlineRGBA( surface, x1, y1, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::LineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color)
{
  return lineRGBA( surface, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::AALineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color)
{
  return aalineRGBA( surface, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::AAFadingLineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color1, const Color &color2)
{
  return aafadingLineColor(surface, x1, y1, x2, y2,color1.GetColor(), color2.GetColor());
}

int Surface::CircleColor(const uint &x, const uint &y, const uint &rad, const Color &color)
{
  return circleRGBA( surface, x, y, rad, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::FilledCircleColor(const uint &x, const uint &y, const uint &rad, const Color &color)
{
  return filledCircleRGBA( surface, x, y, rad, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::PieColor(const uint &x, const uint &y, const uint &rad, const int &start, const int &end, const Color &color)
{
  return pieRGBA(surface, x, y, rad, start, end, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::FilledPieColor(const uint &x, const uint &y, const uint &rad, const int &start, const int &end, const Color &color)
{
  return filledPieRGBA(surface, x, y, rad, start, end, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::AAPolygonColor(const Sint16 * vx, const Sint16 * vy, const int n, const Color & color){
  return aapolygonRGBA(surface, vx, vy, n, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::AAPolygonColor(std::list<Point2i> polygon, const Color & color)
{
  Sint16 * vx, * vy;
  vx = new Sint16[polygon.size()];
  vy = new Sint16[polygon.size()];
  int i = 0;
  for(std::list<Point2i>::iterator point = polygon.begin(); point != polygon.end(); point++, i++) {
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
  for(std::list<Point2i>::iterator point = polygon.begin(); point != polygon.end(); point++, i++) {
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
  for(std::list<Point2i>::iterator point = polygon.begin(); point != polygon.end(); point++, i++) {
    vx[i] = point->x;
    vy[i] = point->y;
  }
  int result = texturedPolygon(surface, vx, vy, polygon.size(), texture->surface, 0, 0);
  delete[] vx;
  delete[] vy;
  return result;
}

int Surface::PointColor(const Point2d &point,const Color &color)
{
  LineColor(point.x-1,point.x+1 ,point.y ,point.y ,color);
  return LineColor(point.x,point.x ,point.y+1 ,point.y-1 ,color);
}

/**
 *
 * @param color
 */
int Surface::Fill(Uint32 color) const
{
  return SDL_FillRect( surface, NULL, color);
}

int Surface::Fill(const Color &color) const
{
  return Fill( MapColor(color) );
}

/**
 *
 * @param dstRect
 * @param color
 */
int Surface::FillRect(const Rectanglei &dstRect, Uint32 color) const
{
  SDL_Rect sdlDstRect = GetSDLRect( dstRect );

  return SDL_FillRect( surface, &sdlDstRect, color);
}

/**
 *
 * @param dstRect
 * @param color
 */
int Surface::FillRect(const Rectanglei &dstRect, const Color &color) const
{
  return FillRect( dstRect, MapColor(color) );
}

/**
 *
 * @param filename
 */
int Surface::ImgLoad(const std::string& filename)
{
  AutoFree();
  surface = IMG_Load( filename.c_str() );

  return !IsNull();
}

/**
 *
 * @param filename
 */
#ifdef WIN32
#  define alloca _alloca
#endif
int Surface::ImgSave(const std::string& filename)
{
  FILE            *f        = NULL;
  png_structp     png_ptr   = NULL;
  png_infop       info_ptr  = NULL;
  SDL_PixelFormat * spr_fmt = surface->format;
  int             ret       = 1;
  Uint8           *tmp_line = NULL;

  // Creating a png ...
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png_ptr == NULL) // Structure and ...
    return 1;
  info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == NULL) // Information.
    goto end;

  // Opening a new file
  f = fopen(filename.c_str(), "wb");
  if(f == NULL)
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
  for(int y = 0; y < surface->h; y++) {
    for(int x = 0; x < surface->w; x++) {
      Uint8   r, g, b, a;
      // Retrieving a pixel of sprite to merge
      Uint32  spr_pix = ((Uint32*)surface->pixels)[y * surface->w  + x];

      // Retreiving each chanel of the pixel using pixel format
      r = (Uint8)(((spr_pix & spr_fmt->Rmask) >> spr_fmt->Rshift) << spr_fmt->Rloss);
      g = (Uint8)(((spr_pix & spr_fmt->Gmask) >> spr_fmt->Gshift) << spr_fmt->Gloss);
      b = (Uint8)(((spr_pix & spr_fmt->Bmask) >> spr_fmt->Bshift) << spr_fmt->Bloss);
      a = (Uint8)(((spr_pix & spr_fmt->Amask) >> spr_fmt->Ashift) << spr_fmt->Aloss);
      tmp_line[x * spr_fmt->BytesPerPixel] = r;
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
  ret = 1;

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
static const double ratio_deg_to_rad = 180 / M_PI;
Surface Surface::RotoZoom(double angle, double zoomx, double zoomy, int smooth)
{
  SDL_Surface *surf;

#ifdef BUGGY_SDLGFX
  /* From SDLGFX website,
   * 'zoomx' and 'zoomy' are scaling factors that
   * can also be negative. In this case the corresponding axis is flipped.
   * Note: Flipping currently only works with antialiasing turned off
   */
  if (zoomx < 0.0 || zoomy < 0.0)
    smooth = SMOOTHING_OFF;
#endif

  if (EqualsZero(angle))
    surf = zoomSurface(surface, zoomx, zoomy, smooth);
  else if (zoomx == zoomy && zoomx > 0.0)
    surf = rotozoomSurface(surface, angle * ratio_deg_to_rad , zoomx, smooth);
  else
    surf = rotozoomSurfaceXY(surface, angle * ratio_deg_to_rad , zoomx, zoomy, smooth);

  if(!surf)
    Error( "Unable to make a rotozoom on the surface !" );

  return Surface(surf);
}

/**
 *
 */
Surface Surface::DisplayFormatAlpha()
{
  SDL_Surface *surf = SDL_DisplayFormatAlpha(surface);

  if( !surf )
    Error( "Unable to convert the surface to a surface compatible with the display format with alpha." );

  return Surface(surf);
}

/**
 *
 */
Surface Surface::DisplayFormat()
{
  SDL_Surface *surf = SDL_DisplayFormat(surface);

  if( !surf )
    Error( "Unable to convert the surface to a surface compatible with the display format." );

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

SDL_Rect Surface::GetSDLRect(const Rectanglei &r) const
{
  SDL_Rect sdlRect;

  sdlRect.x = r.GetPositionX();
  sdlRect.y = r.GetPositionY();
  sdlRect.w = r.GetSizeX();
  sdlRect.h = r.GetSizeY();

  return sdlRect;
}

SDL_Rect Surface::GetSDLRect(const Point2i &pt) const
{
  SDL_Rect sdlRect;

  sdlRect.x = pt.GetX();
  sdlRect.y = pt.GetY();
  sdlRect.w = 0;
  sdlRect.h = 0;

  return sdlRect;
}

Uint32 Surface::ComputeCRC()
{
  Uint32 crc = 0;
  Uint32 current_pix;
  SDL_PixelFormat * current_fmt = surface->format;
  Uint8 r, g, b, a;

  Point2i offset;
  int current_offset;

  Lock();
  // for each pixel of the image
  for (offset.x = 0; offset.x < GetWidth(); offset.x++) {
    for (offset.y = 0; offset.y < GetHeight(); offset.y++) {

      current_offset = offset.y * surface->w + offset.x;

      // Retrieving a pixel of sprite to merge
      current_pix = ((Uint32*)surface->pixels)[current_offset];

      // Retreiving each chanel of the pixel using pixel format
      r = (Uint8)(((current_pix & current_fmt->Rmask) >> current_fmt->Rshift) << current_fmt->Rloss);
      g = (Uint8)(((current_pix & current_fmt->Gmask) >> current_fmt->Gshift) << current_fmt->Gloss);
      b = (Uint8)(((current_pix & current_fmt->Bmask) >> current_fmt->Bshift) << current_fmt->Bloss);
      a = (Uint8)(((current_pix & current_fmt->Amask) >> current_fmt->Ashift) << current_fmt->Aloss);

      // Computing CRC - each time, we had at most 255*4, the biggest storable value
      // on a Uint32 is 4294967296
      // avoid integer overflow with a stupid modulo
      crc += (r + g + b + a); // each time, we had at most 255*4
      crc = crc % 429496000;
    }
  }

  Unlock();
  return crc;
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

  if(dest_x<0){
    dest_x = 0;
    mask_x = -pos.x;
	}
  if(dest_y<0){
    dest_y = 0;
    mask_y = -pos.y;
  }

  if((size_x + dest_x) > GetWidth()){
    size_x = GetWidth()- dest_x;
  }

  if((size_y + dest_y) > GetHeight()){
    size_y = GetHeight() - dest_y;
  }
  Uint32 temp_pix;

  Uint32 mask_color, this_color;
  Uint8 mask_r, mask_g, mask_b, mask_a;
  Uint8 this_r, this_g, this_b, this_a;

  for(int i = 0; i<size_x;i++){
    for(int j = 0;j<size_y;j++){
      //Copy red pixel of mask in alpha pixel of this
      mask_color  = mask.GetPixel(i + mask_x, j + mask_y);
      this_color = GetPixel(i + dest_x, j + dest_y) ;

      GetRGBA(this_color, this_r, this_g, this_b, this_a);
      mask.GetRGBA(mask_color,mask_r,mask_g,mask_b,mask_a);

      if(mask_r < this_a){
        temp_pix = MapRGBA(this_r, this_g, this_b, mask_r);
        PutPixel(i+ dest_x, j + dest_y, temp_pix);
      }

    }
  }
}
