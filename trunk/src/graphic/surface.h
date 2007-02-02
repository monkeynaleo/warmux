/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
 * Handle a SDL_Surface.
 *****************************************************************************/

#ifndef SURFACE_H
#define SURFACE_H

#include <SDL.h>
#include <string>
#include <list>
#include "color.h"
#include "../include/base.h"
#include "../tool/point.h"
#include "../tool/rectangle.h"
#include "polygon.h"

class Polygon;

class Surface
{
private:
  SDL_Surface* surface;
  bool autoFree;
  int Blit(const Surface& src, SDL_Rect *srcRect, SDL_Rect *dstRect);
  SDL_Rect GetSDLRect(const Rectanglei &r) const;
  SDL_Rect GetSDLRect(const Point2i &r) const;
  
public:
  explicit Surface();
  explicit Surface(SDL_Surface *sdl_surface);
  explicit Surface(const Point2i &size, Uint32 flags, bool useAlpha = true);
  explicit Surface(const std::string &filename);
  Surface(const Surface &src);
  ~Surface();

  Surface &operator=(const Surface &src);
  
  void Free();
  void AutoFree();
  void SetAutoFree(bool newAutoFree);
  
  void SetSurface(SDL_Surface *newSurface, bool freePrevious = true);
  void NewSurface(const Point2i &size, Uint32 flags, bool useAlpha = true);
  
  SDL_Surface *GetSurface();
  
  int SetAlpha(Uint32 flags, Uint8 alpha);
  
  int Lock();
  void Unlock();

  int Blit(const Surface& src); 
  int Blit(const Surface& src, const Point2i& dst);
  int Blit(const Surface& src, const Rectanglei& srcRect, const Point2i &dstPoint);
  
  int SetColorKey(Uint32 flag, Uint32 key);
  int SetColorKey(Uint32 flag, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  
  void GetRGBA(Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) const;
  Uint32 MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const;
  Color GetColor(Uint32 color) const;
  Uint32 MapColor(Color color) const;

  void SetClipRect(const Rectanglei &rect);
  void Flip();

  int BoxColor(const Rectanglei &rect, const Color &color);
  int RectangleColor(const Rectanglei &rect, const Color &color, const uint &border_size = 1);
  int VlineColor(const uint &x1, const uint &y1, const uint &y2, const Color &color);
  int LineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color);
  int AALineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color);
  int CircleColor(const uint &x, const uint &y, const uint &rad, const Color &color);
  int AAPolygonColor(const Sint16 * vx, const Sint16 * vy, const int n, const Color & color);
  int AAPolygonColor(std::list<Point2i> polygon, const Color & color);
  int FilledPolygon(const Sint16 * vx, const Sint16 * vy, const int n, const Color & color);
  int FilledPolygon(std::list<Point2i> polygon, const Color & color);
  int TexturedPolygon(const Sint16 * vx, const Sint16 * vy, const int n, const Surface *texture, const int texture_dx, const int texture_dy);
  int TexturedPolygon(std::list<Point2i> polygon, const Surface *texture);
  int DrawPolygon(const Polygon &polygon);

  int Fill(Uint32 color) const;
  int Fill(const Color &color) const;
  int FillRect(const Rectanglei &dstRect, Uint32 color) const;
  int FillRect(const Rectanglei &dstRect, const Color &color) const;

  int ImgLoad(std::string filename);
  int ImgSave(std::string filename);
  Surface RotoZoom(double angle, double zoomx, double zoomy, int smooth);
  Surface DisplayFormatAlpha();
  Surface DisplayFormat();
  Uint32 GetPixel(int x, int y);
  void PutPixel(int x, int y, Uint32 pixel);
  
  inline bool IsNull() const{
    return surface == NULL;
  }
  
  /** 
   * Return the size of a surface.
   */
  inline Point2i GetSize() const{
    return Point2i( GetWidth(), GetHeight() );
  }
  
  /// Return the width of a surface.
  inline int GetWidth() const{
    return surface->w;
  }
  
  /// Return the height of a surface.
  inline int GetHeight() const{
    return surface->h;
  }
  
  inline Uint32 GetFlags() const{
    return surface->flags;
  }
  
  /// Return the length of a surface scanline in bytes.
  inline Uint16 GetPitch() const{
    return surface->pitch;
  }
  
  /// Return the number of bytes used to represent each pixel in a surface. Usually one to four.
  inline Uint8 GetBytesPerPixel() const{
    return surface->format->BytesPerPixel;
  }
  
  /// Return a pointer on the pixels data.
  inline unsigned char *GetPixels() const{
    return (unsigned char *) surface->pixels;
  }

};

#endif
