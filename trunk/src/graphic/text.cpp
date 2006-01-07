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
 ******************************************************************************/

#include <SDL/SDL_video.h>
#include <SDL_gfxPrimitives.h>
#include <string>
#include "text.h"
#include "colors.h"
#include "font.h"
#include "video.h"
#include "../include/app.h"
#include "../tool/error.h"
#include "../interface/interface.h"
#include "../map/map.h"
//-----------------------------------------------------------------------------

Text::Text(const std::string &new_txt, SDL_Color new_color, Font* new_font)
{
  assert(new_font!=NULL);
  txt = "";
  color = new_color;
  font = new_font;
  surf = NULL;
  Set(new_txt);
}

//-----------------------------------------------------------------------------
Text::~Text()
{
  assert(surf!=NULL);
  SDL_FreeSurface(surf);
}

//-----------------------------------------------------------------------------
void Text::Set(const std::string &new_txt)
{
  if(txt == new_txt)
    return;

  txt = new_txt;

  if (surf != NULL)
  {
    SDL_FreeSurface(surf);
  }

  //Doesn't work.. don't know why...
/*  SDL_Surface* tmp = font->Render(new_txt,color);
  surf = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, tmp->w, tmp->h, 32,
                                0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
  SDL_Rect dst_rect = {0,0,tmp->w,tmp->h};
  SDL_BlitSurface(tmp, NULL, surf, &dst_rect);*/

  surf = TTF_RenderUTF8_Blended(font->m_font, txt.c_str(),color);
}

//-----------------------------------------------------------------------------
void Text::DrawCenter (int x, int y)
{ 
  SDL_Rect dst_rect;
  dst_rect.x = x - surf->w / 2;
  dst_rect.y = y - surf->h / 2;
  dst_rect.w = surf->w;
  dst_rect.h = surf->h;

  SDL_BlitSurface(surf,NULL,app.sdlwindow, &dst_rect);
  world.ToRedrawOnScreen(Rectanglei(dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h));
}

//-----------------------------------------------------------------------------
void Text::DrawTopLeft (int x, int y)
{ 
  SDL_Rect dst_rect;
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.w = surf->w;
  dst_rect.h = surf->h;

  SDL_BlitSurface(surf,NULL,app.sdlwindow, &dst_rect);
  world.ToRedrawOnScreen(Rectanglei(dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h));
}

//-----------------------------------------------------------------------------
void Text::DrawTopRight (int x, int y)
{ 
  SDL_Rect dst_rect;
  dst_rect.x = x - surf->w;
  dst_rect.y = y;
  dst_rect.w = surf->w;
  dst_rect.h = surf->h;

  SDL_BlitSurface(surf,NULL,app.sdlwindow, &dst_rect);
  world.ToRedrawOnScreen(Rectanglei(dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h));
}

//-----------------------------------------------------------------------------
void Text::DrawCenterTop (int x, int y)
{ 
  SDL_Rect dst_rect;
  dst_rect.x = x - surf->w / 2;
  dst_rect.y = y;
  dst_rect.w = surf->w;
  dst_rect.h = surf->h;

  SDL_BlitSurface(surf,NULL,app.sdlwindow, &dst_rect);
  world.ToRedrawOnScreen(Rectanglei(dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h));
}

//-----------------------------------------------------------------------------
void Text::DrawCenterOnMap (int x, int y)
{
  AbsoluteDraw(surf, x - surf->w / 2, y - surf->h / 2);
}

//-----------------------------------------------------------------------------
void Text::DrawTopLeftOnMap (int x, int y)
{
  AbsoluteDraw(surf, x, y);
}

//-----------------------------------------------------------------------------
void Text::DrawCenterTopOnMap (int x, int y)
{
  AbsoluteDraw(surf, x - surf->w / 2, y);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// 
void DrawTmpBoxText(Font &font, 
		    int _x, int _y, 
		    const std::string &txt, uint space)
{
  int x,y,w,h;
  w = font.GetWidth(txt)+space*2;
  x = _x - w / 2;

  h = font.GetHeight(txt)+space*2;
  y = _y - h / 2;
  _y -= font.GetHeight(txt)/2;

  boxRGBA(app.sdlwindow, x, y, x+w, y+h,
	  80,80,159,206);

  rectangleRGBA(app.sdlwindow, x, y, x+w, y+h,
		49, 32, 122, 255);  

  world.ToRedrawOnScreen(Rectanglei(x, y, w, h));
  font.WriteCenterTop (_x, _y, txt, white_color);
}
