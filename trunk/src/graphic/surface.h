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

namespace Wormux{

	class Surface
	{
		private:
			SDL_Surface* surface;
			bool autoFree;

		public:

		private:

		public:
			Surface();
			Surface(SDL_Surface *sdl_surface);
			Surface(int width, int height, Uint32 flags, bool useAlpha);
			Surface(const char *filename);
			Surface(const Surface &src);
			~Surface();
			Surface &operator=(Surface & src);
			
			void Free();
			void AutoFree();
			void SetAutoFree(bool newAutoFree);
			
			void SetSurface(SDL_Surface *newSurface, bool freePrevious = true);
			void Surface::NewSurface(int width, int height, Uint32 flags, bool useAlpha);
			
			SDL_Surface *GetSurface();
			int GetWidth();
			int GetHeight();
			int SetAlpha(Uint32 flags, Uint8 alpha);
			Uint32 GetFlags();
			int Lock();
			void Unlock();
			int Blit(SDL_Surface *src, SDL_Rect *srcRect, SDL_Rect *dstRect);
			int Blit(Surface src, SDL_Rect *srcRect, SDL_Rect *dstRect);
			int SetColorKey(Uint32 flag, Uint32 key);
			void SetClipRect(SDL_Rect *rect);
			void Flip();
			int BoxRGBA(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
			int RectangleRGBA(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
			int FillRect( SDL_Rect *dstrect, Uint32 color);
			int FillRect( SDL_Rect *r, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
			int ImgLoad( const char *filename );
	};

}
#endif
