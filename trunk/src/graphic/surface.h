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
 * Handle a SDL_Surface.
 *****************************************************************************/

#ifndef SURFACE_H
#define SURFACE_H
#include <SDL.h>
#include <string>
#include "../include/base.h"

class Surface
{
  SDL_Surface* surface;
  bool autoFree;

public:
  explicit Surface();
  explicit Surface(SDL_Surface *sdl_surface);
  explicit Surface(int width, int height, Uint32 flags, bool useAlpha = true);
  explicit Surface(const std::string &filename);
  Surface(const Surface &src);
  ~Surface();
  Surface &operator=(const Surface &src);

  void Free();
  void AutoFree();
  void SetAutoFree(bool newAutoFree);

  void SetSurface(SDL_Surface *newSurface, bool freePrevious = true);
  void Surface::NewSurface(int width, int height, Uint32 flags, bool useAlpha = true);

  SDL_Surface *GetSurface();
  int SetAlpha(Uint32 flags, Uint8 alpha);
  int Lock();
  void Unlock();
  int Blit(SDL_Surface *src, SDL_Rect *srcRect, SDL_Rect *dstRect);
  int Blit(const Surface& src, SDL_Rect *srcRect, SDL_Rect *dstRect);
  int SetColorKey(Uint32 flag, Uint32 key);
  int SetColorKey(Uint32 flag, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  void GetRGBA(Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a);
  Uint32 MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  void SetClipRect(SDL_Rect *rect);
  void Flip();
  int BoxRGBA(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  int RectangleRGBA(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  int FillRect( SDL_Rect *dstrect, Uint32 color);
  int FillRect( SDL_Rect *r, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  int ImgLoad( const char *filename );
  Surface RotoZoom(double angle, double zoomx, double zoomy, int smooth);
  Surface DisplayFormatAlpha();
  Surface DisplayFormat();
  Uint32 GetPixel(int x, int y);
  void PutPixel(int x, int y, Uint32 pixel);

  inline bool IsNull() const{
    return surface == NULL;
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
