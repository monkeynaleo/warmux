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
 * Handle a SDL Surface
 *****************************************************************************/

#include "surface.h"
#include <SDL.h>
#include <SDL_endian.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include "../tool/debug.h"
#include "../tool/error.h"
#include "../tool/i18n.h"
#include "../include/base.h"

Surface::Surface(){
	surface = NULL;
	autoFree = true;
}

Surface::Surface(SDL_Surface *sdl_surface){
	surface = sdl_surface;
	autoFree = true;
}

Surface::Surface(int width, int height, Uint32 flags, bool useAlpha){
	surface = NULL;
	autoFree = true;
	NewSurface( width, height, flags, useAlpha );
}

Surface::Surface(const std::string &filename_str){
    const char *filename = filename_str.c_str();
	surface = NULL;
	autoFree = true;
	if( !ImgLoad(filename) )
		Error( Format("Unable to open image file : %s", filename) );
}

Surface::Surface(const Surface &src){
	surface = src.surface;
	autoFree = true;
	if( surface != NULL)
		surface->refcount++;
}

Surface::~Surface(){
	AutoFree();
}

Surface &Surface::operator=(const Surface & src){
	AutoFree();
	surface = src.surface;
	if( surface != NULL )
		surface->refcount++;

	return *this;
}

void Surface::Free(){
	if( surface != NULL ){
		SDL_FreeSurface( surface );
		surface = NULL;
	}
}

void Surface::AutoFree(){
	if( autoFree )
		Free();
}

void Surface::SetAutoFree( bool newAutoFree ){
	autoFree = newAutoFree;
}

SDL_Surface *Surface::GetSurface(){
	return surface;
}

	void Surface::SetSurface(SDL_Surface *newSurface, bool freePrevious){
		if( freePrevious )
			Free();

		surface = newSurface;
	}

void Surface::NewSurface(int width, int height, Uint32 flags, bool useAlpha){
	Uint32 alphaMask;
	Uint32 redMask;
	Uint32 greenMask;
	Uint32 blueMask;

	if( autoFree )
		Free();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	redMask = 0xff000000;
	greenMask = 0x00ff0000;
	blueMask = 0x0000ff00;
	alphaMask =	0x000000ff;
#else
	redMask = 0x000000ff;
	greenMask = 0x0000ff00;
	blueMask = 0x00ff0000;
	alphaMask = 0xff000000;
#endif

	if( !useAlpha )
		alphaMask = 0;

	surface = SDL_CreateRGBSurface( flags, width, height, 32,
			redMask, greenMask, blueMask, alphaMask );

	if( surface == NULL )
		Error( std::string("Can't create SDL RGBA surface: ") + SDL_GetError() );
}

int Surface::SetAlpha(Uint32 flags, Uint8 alpha){
	return SDL_SetAlpha( surface, flags, alpha );
}

int Surface::Lock(){
	return SDL_LockSurface( surface );
}

void Surface::Unlock(){
	SDL_UnlockSurface( surface );
}

int Surface::Blit(SDL_Surface *src, SDL_Rect *srcRect, SDL_Rect *dstRect){
	return SDL_BlitSurface( src, srcRect, surface, dstRect );
}

int Surface::Blit(const Surface& src, SDL_Rect *srcRect, SDL_Rect *dstRect){
	return SDL_BlitSurface( src.surface, srcRect, surface, dstRect );
}

int Surface::SetColorKey(Uint32 flag, Uint32 key){
	return SDL_SetColorKey( surface, flag, key );
}

int Surface::SetColorKey(Uint32 flag, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
	return SetColorKey( flag, MapRGBA(r, g, b, a) );
}

void Surface::GetRGBA(Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a){
	SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);
}

Uint32 Surface::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a){
    return SDL_MapRGBA(surface->format, r, g, b, a);
}

void Surface::SetClipRect(SDL_Rect *rect){
	SDL_SetClipRect( surface, rect );
}

void Surface::Flip(){
	SDL_Flip( surface );
}

int Surface::BoxRGBA(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
	return boxRGBA( surface, x1, y1, x2, y2, r, g, b, a );
}

int Surface::RectangleRGBA(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
	return rectangleRGBA( surface, x1, y1, x2, y2, r, g, b, a );
}

int Surface::FillRect(SDL_Rect *dstRect, Uint32 color){
	return SDL_FillRect( surface, dstRect, color);
}

int Surface::FillRect( SDL_Rect *dstRect, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
	return FillRect( dstRect, MapRGBA(r, g, b, a) );
}

int Surface::ImgLoad( const char *filename ){
	AutoFree();
	surface = IMG_Load( filename );

	return surface != NULL;
}

Surface Surface::RotoZoom(double angle, double zoomx, double zoomy, int smooth){
	Surface newSurf;

	newSurf.SetSurface( rotozoomSurfaceXY(surface, angle, zoomx, zoomy, smooth) );

	if( newSurf.IsNull() )
		Error( "Unable to make a rotozoom on the surface !" );

	return newSurf;	
}

Surface Surface::DisplayFormatAlpha(){
	Surface newSurf;

	newSurf.SetSurface( SDL_DisplayFormatAlpha( surface ) );

	if( newSurf.IsNull() )
		Error( "Unable to convert the surface to a surface compatible with the display format with alpha." );

	return newSurf;
}

Surface Surface::DisplayFormat(){
	Surface newSurf;

	newSurf.SetSurface( SDL_DisplayFormat( surface ) );

	if( newSurf.IsNull() )
		Error( "Unable to convert the surface to a surface compatible with the display format." );

	return newSurf;
}

// From SDL's wiki
Uint32 Surface::GetPixel(int x, int y){
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
		Error("Unknow bpp!");
//        return 0;
    }
}

void Surface::PutPixel(int x, int y, Uint32 pixel){
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
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

