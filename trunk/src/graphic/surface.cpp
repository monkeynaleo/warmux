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
#include <png.h>
#include "../tool/debug.h"
#include "../tool/error.h"
#include "../tool/i18n.h"
#include "../include/base.h"

/**
 * Default constructor.
 *
 * Build a null surface with autoFree at true.
 */
Surface::Surface(){
	surface = NULL;
	autoFree = true;
}

/**
 * Constructor building a surface object using an existing SDL_Surface pointer.
 *
 * @param sdl_surface The existing sdl_surface.
 */
Surface::Surface(SDL_Surface *sdl_surface){
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
Surface::Surface(const Point2i &size, Uint32 flags, bool useAlpha){
	surface = NULL;
	autoFree = true;
	NewSurface(size, flags, useAlpha);
}

/**
 * Constructor building a surface by reading the image from a file.
 *
 * @param filename_str A string containing the path to the graphic file.
 */
Surface::Surface(const std::string &filename){
	surface = NULL;
	autoFree = true;
	if( !ImgLoad(filename) )
		Error( Format("Unable to open image file : %s", filename.c_str() ) );
}

/**
 * Copy constructor: build a surface from an other surface.
 *
 * The two surfaces share the same graphic data.
 */
Surface::Surface(const Surface &src){
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
Surface::~Surface(){
	AutoFree();
}

Surface &Surface::operator=(const Surface & src){
	AutoFree();
	surface = src.surface;
	if( !IsNull() )
		surface->refcount++;

	return *this;
}

/**
 * Free the memory occupied by the surface.
 *
 * The memory is really freed if the reference counter reach 0.
 */
void Surface::Free(){
	if( !IsNull() ){
		SDL_FreeSurface( surface );
		surface = NULL;
	}
}

void Surface::AutoFree(){
	if( autoFree )
		Free();
}

/**
 * Set the auto free status of a surface.
 *
 * In general it should always be true for non-system surface.
 * @param newAutoFree the new autoFree status.
 */
void Surface::SetAutoFree( bool newAutoFree ){
	autoFree = newAutoFree;
}

/**
 * Return the pointer of the SDL_Surface.
 *
 * Should be used carefully.
 */
SDL_Surface *Surface::GetSurface(){
	return surface;
}

/**
 * Change the surface pointer.
 *
 * @param newSurface The new surface to use.
 * @param freePrevius Indicate if the old surface should be freed.
 */
void Surface::SetSurface(SDL_Surface *newSurface, bool freePrevious){
	if( freePrevious )
		Free();

	surface = newSurface;
}

/**
 * Create a new surface.
 *
 * @param size
 * @param flags
 * @param useAlpha
 */
void Surface::NewSurface(const Point2i &size, Uint32 flags, bool useAlpha){
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

	surface = SDL_CreateRGBSurface(flags, size.x, size.y, 32,
			redMask, greenMask, blueMask, alphaMask );

	if( surface == NULL )
		Error( std::string("Can't create SDL RGBA surface: ") + SDL_GetError() );
}

/**
 * Set the alpha value of a surface.
 *
 */
int Surface::SetAlpha(Uint32 flags, Uint8 alpha){
	return SDL_SetAlpha( surface, flags, alpha );
}

/**
 * Lock the surface to permit direct access.
 *
 */
int Surface::Lock(){
	return SDL_LockSurface( surface );
}

/**
 * Unlock the surface.
 *
 */
void Surface::Unlock(){
	SDL_UnlockSurface( surface );
}

int Surface::Blit(const Surface& src, SDL_Rect *srcRect, SDL_Rect *dstRect){
	return SDL_BlitSurface( src.surface, srcRect, surface, dstRect );
}

/**
 * Blit the whole surface src on the current surface.
 *
 * @param src The source surface.
 */
int Surface::Blit(const Surface& src){
	return Blit(src, NULL, NULL);
}

/**
 * Blit a surface (src) on the current surface at a certain position (dst)
 *
 * @src The source surface.
 * @dst A point defining the destination coordinate on the current surface.
 */
int Surface::Blit(const Surface& src, const Point2i &dst){
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
int Surface::Blit(const Surface& src, const Rectanglei &srcRect, const Point2i &dstPoint){
	SDL_Rect sdlSrcRect = GetSDLRect( srcRect );
	SDL_Rect sdlDstRect = GetSDLRect( dstPoint );

	return Blit(src, &sdlSrcRect, &sdlDstRect);
}

/**
 *
 * @param flag
 * @param key
 */
int Surface::SetColorKey(Uint32 flag, Uint32 key){
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
int Surface::SetColorKey(Uint32 flag, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
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
Uint32 Surface::MapColor(Color color) const
{
  return MapRGBA(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

/**
 * @param rect
 */
void Surface::SetClipRect(const Rectanglei &rect){
	SDL_Rect sdlRect = GetSDLRect( rect );
	SDL_SetClipRect( surface, &sdlRect );
}

void Surface::Flip(){
	SDL_Flip( surface );
}

int Surface::BoxColor(const Rectanglei &rect, const Color &color){
	if( rect.IsSizeZero() )
		return 0;

	Point2i ptBR = rect.GetBottomRightPoint();

	return boxRGBA( surface, rect.GetPositionX(), rect.GetPositionY(), ptBR.GetX(), ptBR.GetY(), color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::RectangleColor(const Rectanglei &rect, const Color &color, const uint &border_size)
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

int Surface::VlineColor(const uint &x1, const uint &y1, const uint &y2, const Color &color){
  return vlineRGBA( surface, x1, y1, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::LineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color){
  return lineRGBA( surface, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::AALineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color){
  return aalineRGBA( surface, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::CircleColor(const uint &x, const uint &y, const uint &rad, const Color &color){
  return circleRGBA( surface, x, y, rad, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int Surface::AAPolygonColor(const Sint16 * vx, const Sint16 * vy, const int n, const Color & color){
  return aapolygonRGBA(surface, vx, vy, n, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

int Surface::FilledPolygon(const Sint16 * vx, const Sint16 * vy, const int n, const Color & color){
  return filledPolygonRGBA(surface, vx, vy, n, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
}

/* texturedPolygon import from SDL_gfx v2.0.15 */
#if (SDL_GFXPRIMITIVES_MAJOR == 2) && (SDL_GFXPRIMITIVES_MINOR == 0) && (SDL_GFXPRIMITIVES_MICRO < 14)
#include <SDL_gfxPrimitives.h>
static int gfxPrimitivesPolyAllocated = 0;
static int *gfxPrimitivesPolyInts = NULL;
int gfxPrimitivesCompareInt(const void *a, const void *b)
{
    return (*(const int *) a) - (*(const int *) b);
}

int _texturedHLine(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y,SDL_Surface *texture,int texture_dx,int texture_dy)
{
    Sint16 left, right, top, bottom;
    Sint16 w;
    Sint16 xtmp;
    int result = 0;
    int texture_x_walker;    
    int texture_y_start;    
    SDL_Rect source_rect,dst_rect;
    int pixels_written,write_width;

    /*
    * Get clipping boundary 
    */
    left = dst->clip_rect.x;
    right = dst->clip_rect.x + dst->clip_rect.w - 1;
    top = dst->clip_rect.y;
    bottom = dst->clip_rect.y + dst->clip_rect.h - 1;

    /*
    * Check visibility of hline 
    */
    if ((x1<left) && (x2<left)) {
    return(0);
    }
    if ((x1>right) && (x2>right)) {
    return(0);
    }
    if ((y<top) || (y>bottom)) {
    return (0);
    }

    /*
    * Swap x1, x2 if required 
    */
    if (x1 > x2) {
            xtmp = x1;
            x1 = x2;
            x2 = xtmp;
    }

    /*
    * Clip x 
    */
    if (x1 < left) {
            x1 = left;
    }
    if (x2 > right) {
            x2 = right;
    }

    /*
    * Calculate width 
    */
    w = x2 - x1;

    /*
    * Sanity check on width 
    */
    if (w < 0) {
            return (0);
    }

    /*
    * Determint where in the texture we start drawing
    **/
    texture_x_walker =   (x1 - texture_dx)  % texture->w;
    if (texture_x_walker < 0){
      texture_x_walker = texture->w +texture_x_walker ;
    }

    texture_y_start = (y + texture_dy) % texture->h;
    if (texture_y_start < 0){
      texture_y_start = texture->h + texture_y_start;
    }

    //setup the source rectangle  we are only drawing one horizontal line
    source_rect.y = texture_y_start;
    source_rect.x =texture_x_walker;
    source_rect.h =1;
    //we will draw to the current y
    dst_rect.y = y;

    //if there are enough pixels left in the current row of the texture
    //draw it all at once
    if (w <= texture->w -texture_x_walker){
      source_rect.w = w;
      source_rect.x = texture_x_walker;
      dst_rect.x= x1;
      result |= !SDL_BlitSurface  (texture,&source_rect , dst, &dst_rect) ;
    } else {//we need to draw multiple times
      //draw the first segment
      pixels_written = texture->w  -texture_x_walker;
      source_rect.w = pixels_written;
      source_rect.x = texture_x_walker;
      dst_rect.x= x1;
      result |= !SDL_BlitSurface  (texture,&source_rect , dst, &dst_rect);
      write_width = texture->w;

      //now draw the rest
      //set the source x to 0
      source_rect.x = 0;
      while(pixels_written < w){
        if (write_width >= w - pixels_written){
          write_width=  w- pixels_written;
        }
        source_rect.w = write_width;
        dst_rect.x = x1 + pixels_written;
        result |= !SDL_BlitSurface  (texture,&source_rect , dst, &dst_rect) ;
        pixels_written += write_width;
      }
  }
  return result;
}

/**
 * Draws a polygon filled with the given texture. this operation use SDL_BlitSurface. It supports
 * alpha drawing.
 * to get the best performance of this operation you need to make sure the texture and the dst surface have the same format
 * see  http://docs.mandragor.org/files/Common_libs_documentation/SDL/SDL_Documentation_project_en/sdlblitsurface.html
 *
 * dest the destination surface, 
 * vx array of x vector components
 * vy array of x vector components
 * n the amount of vectors in the vx and vy array
 * texture the sdl surface to use to fill the polygon
 * texture_dx the offset of the texture relative to the screeen. if you move the polygon 10 pixels 
 * to the left and want the texture to apear the same you need to increase the texture_dx value
 * texture_dy see texture_dx
 **/
int texturedPolygon(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, SDL_Surface * texture,int texture_dx,int texture_dy)
{
    int result;
    int i;
    int y, xa, xb;
    int minx,maxx,miny, maxy;
    int x1, y1;
    int x2, y2;
    int ind1, ind2;
    int ints;
    /*
     * Sanity check 
     */
    if (n < 3) {
            return -1;
    }
    /*
     * Allocate temp array, only grow array 
     */
    if (!gfxPrimitivesPolyAllocated) {
            gfxPrimitivesPolyInts = (int *) malloc(sizeof(int) * n);
            gfxPrimitivesPolyAllocated = n;
    } else {
            if (gfxPrimitivesPolyAllocated < n) {
                gfxPrimitivesPolyInts = (int *) realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
                gfxPrimitivesPolyAllocated = n;
            }
    }
    miny = vy[0];
    maxy = vy[0];
    minx = vx[0];
    maxx = vx[0];
    for (i = 1; (i < n); i++) {
        if (vy[i] < miny) {
            miny = vy[i];
        } else if (vy[i] > maxy) {
            maxy = vy[i];
        }
        if (vx[i] < minx) {
            minx = vx[i];
        } else if (vx[i] > maxx) {
            maxx = vx[i];
        }
    }
    if (maxx <0 || minx > dst->w){
      return 0;
    }
    if (maxy <0 || miny > dst->h){
      return 0;
    }
    /*
     * Draw, scanning y 
     */
    result = 0;
    for (y = miny; (y <= maxy); y++) {
            ints = 0;
            for (i = 0; (i < n); i++) {
                if (!i) {
                        ind1 = n - 1;
                        ind2 = 0;
                } else {
                        ind1 = i - 1;
                        ind2 = i;
                }
                y1 = vy[ind1];
                y2 = vy[ind2];
                if (y1 < y2) {
                        x1 = vx[ind1];
                        x2 = vx[ind2];
                } else if (y1 > y2) {
                        y2 = vy[ind1];
                        y1 = vy[ind2];
                        x2 = vx[ind1];
                        x1 = vx[ind2];
                } else {
                        continue;
                }
                if ( ((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2)) ) {
                        gfxPrimitivesPolyInts[ints++] = ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
                }
            }
            qsort(gfxPrimitivesPolyInts, ints, sizeof(int), gfxPrimitivesCompareInt);
            for (i = 0; (i < ints); i += 2) {
                xa = gfxPrimitivesPolyInts[i] + 1;
                xa = (xa >> 16) + ((xa & 32768) >> 15);
                xb = gfxPrimitivesPolyInts[i+1] - 1;
                xb = (xb >> 16) + ((xb & 32768) >> 15);
                result |= _texturedHLine(dst, xa, xb, y, texture,texture_dx,texture_dy);
            }
    }
    return (result);
}
#endif /* texturedPolygon import from SDL_gfx v2.0.15 */

int Surface::TexturedPolygon(const Sint16 * vx, const Sint16 * vy, const int n, const Surface *texture, const int texture_dx, const int texture_dy){
  return texturedPolygon(surface, vx, vy, n, texture->surface, texture_dx, texture_dy);
}

/**
 *
 * @param color
 */
int Surface::Fill(Uint32 color) const {
    return SDL_FillRect( surface, NULL, color);
}

int Surface::Fill(const Color &color) const{
	return Fill( MapColor(color) );
}

/**
 *
 * @param dstRect
 * @param color
 */
int Surface::FillRect(const Rectanglei &dstRect, Uint32 color) const{
	SDL_Rect sdlDstRect = GetSDLRect( dstRect );

	return SDL_FillRect( surface, &sdlDstRect, color);
}

/**
 *
 * @param dstRect
 * @param color
 */
int Surface::FillRect(const Rectanglei &dstRect, const Color &color) const{

	return FillRect( dstRect, MapColor(color) );
}

/**
 *
 * @param filename
 */
int Surface::ImgLoad(std::string filename){
  AutoFree();
  surface = IMG_Load( filename.c_str() );

  return !IsNull();
}

/**
 *
 * @param filename
 */
int Surface::ImgSave(std::string filename){
  FILE *f             = NULL;
  png_structp png_ptr = NULL;
  png_infop info_ptr  = NULL;
  Uint8 *ptr          = (Uint8 *)(surface->pixels);
  int height          = surface->h;
  // Same dirty hack as in tileitemp.cpp : there is no obvious reason it should work, but with this it works (TM)
  // WE NEED TO KNOW WHY !!!
  // It's seems that this hack is not needed if we work with little image. But when we want to save a map, we need this ...
  ptr++;
  // Creating a png ...
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png_ptr == NULL) // Structure and ...
    goto error_while_creating_png;
  info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == NULL) // Information.
    goto error_while_creating_png;

  // Opening a new file
  f = fopen(filename.c_str(), "wb");
  if(f == NULL)
    goto error_while_creating_png;
  png_init_io(png_ptr, f); // Associate png struture with a file
  png_set_IHDR(png_ptr, info_ptr, surface->w, surface->h, 8,
               PNG_COLOR_TYPE_RGB_ALPHA,      PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,  PNG_FILTER_TYPE_DEFAULT);
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  // Creating the png file
  png_write_info(png_ptr, info_ptr);
  png_set_bgr(png_ptr); // Set byte order to blue green red.
  while(height > 0) {
    png_write_row(png_ptr, ptr);
    ptr += surface->pitch;
    height--;
  }
  png_write_flush(png_ptr);
  png_write_end(png_ptr, info_ptr);
  fclose(f);
  return 0;
 error_while_creating_png:
  if (png_ptr) png_destroy_write_struct(&png_ptr, NULL);
  if (f) fclose(f);
  return 1;
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
Surface Surface::RotoZoom(double angle, double zoomx, double zoomy, int smooth){
	Surface newSurf;

	newSurf.SetSurface( rotozoomSurfaceXY(surface, angle * ratio_deg_to_rad , zoomx, zoomy, smooth) );

	if( newSurf.IsNull() )
		Error( "Unable to make a rotozoom on the surface !" );

	return newSurf;
}

/**
 *
 */
Surface Surface::DisplayFormatAlpha(){
	Surface newSurf;

	newSurf.SetSurface( SDL_DisplayFormatAlpha( surface ) );

	if( newSurf.IsNull() )
		Error( "Unable to convert the surface to a surface compatible with the display format with alpha." );

	return newSurf;
}

/**
 *
 */
Surface Surface::DisplayFormat(){
	Surface newSurf;

	newSurf.SetSurface( SDL_DisplayFormat( surface ) );

	if( newSurf.IsNull() )
		Error( "Unable to convert the surface to a surface compatible with the display format." );

	return newSurf;
}


/**
 * GetPixel.
 *
 * From the SDL wiki.
 * @param x
 * @param y
 */
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
        return 0;   // To make gcc happy
    }
}

/**
 *
 * @param x
 * @param y
 * @param pixel
 */
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
