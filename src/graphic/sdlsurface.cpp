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
 *  MERCHANTABILITY or FITNESS FOR A ARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU GeneralPublic License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * SdlSurface
 *****************************************************************************/

#include "sdlsurface.h"
#include "../include/app.h"
#include "../tool/error.h"

SDL_Surface* CreateRGBSurface (int width, int height, Uint32 flags){

  SDL_Surface* surface = SDL_CreateRGBSurface(flags, width, height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN          
          0xff000000,  // red mask
          0x00ff0000,  // green mask
          0x0000ff00,  // blue mask
#else
          0x000000ff,  // red mask
          0x0000ff00,  // green mask
          0x00ff0000,  // blue mask
#endif  
          0 // don't use alpha
   );          
  if ( surface == NULL )
      Error(std::string("Can't create SDL RGBA surface: ") + SDL_GetError());	
  return surface;
}    

SDL_Surface* CreateRGBASurface (int width, int height, Uint32 flags){
	
  SDL_Surface* surface = SDL_CreateRGBSurface(flags, width, height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN          
          0xff000000,  // red mask
          0x00ff0000,  // green mask
          0x0000ff00,  // blue mask
          0x000000ff // alpha mask
#else
          0x000000ff,  // red mask
          0x0000ff00,  // green mask
          0x00ff0000,  // blue mask
          0xff000000 // alpha mask
#endif  
   );          
  if ( surface == NULL )
      Error(std::string("Can't create SDL RGBA surface: ") + SDL_GetError());	
  return surface;
}    

