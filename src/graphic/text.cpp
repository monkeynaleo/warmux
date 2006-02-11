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

#include <SDL_video.h>
#include <SDL_gfxPrimitives.h>
#include <string>
#include <iostream> //cerr
#include "text.h"
#include "colors.h"
#include "font.h"
#include "video.h"
#include "../include/app.h"
#include "../tool/error.h"
#include "../interface/interface.h"
#include "../map/map.h"
#include "../include/global.h"

Text::Text(const std::string &new_txt, SDL_Color new_color,
           Font* new_font, bool shadowed){
				
  if( new_font == NULL )
    new_font = &global().small_font();
	
  txt = new_txt;
  color = new_color;
  font = new_font;
  this->shadowed = shadowed;

  if( shadowed ){
    int width = font->GetWidth("x");
    bg_offset = (unsigned int)width/8; // shadow offset = 0.125ex
    if (bg_offset < 1) bg_offset = 1;
  }
  else
    bg_offset = 0;

  Render();
}

Text::~Text(){
}

void Text::Render(){
  surf = font->CreateSurface(txt, color);
  if( shadowed )
    background = font->CreateSurface(txt, black_color);
}

void Text::Set(const std::string &new_txt){
				
  if(txt == new_txt)
    return;

  txt = new_txt;
	
  Render();
}

void Text::SetColor(SDL_Color new_color){
  if(color.r == new_color.r
  && color.g == new_color.g
  && color.b == new_color.b)
    return;

  color = new_color;
	
  Render();
}

void Text::DrawCenter (int x, int y){ 
  DrawTopLeft(x - surf.GetWidth() / 2, y - surf.GetHeight() / 2);
}

void Text::DrawTopRight (int x, int y){ 
  DrawTopLeft( x - surf.GetWidth(), y);
}

void Text::DrawCenterTop (int x, int y){ 
  DrawTopLeft( x - surf.GetWidth()/2, y);
}

void Text::DrawTopLeft (int x, int y){ 
  SDL_Rect dst_rect;
  
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.w = surf.GetWidth();
  dst_rect.h = surf.GetHeight();

  if(shadowed)
  {
    SDL_Rect shad_rect;
    
    shad_rect.x = dst_rect.x + bg_offset;
    shad_rect.y = dst_rect.y + bg_offset;
    shad_rect.w = background.GetWidth();
    shad_rect.h = background.GetHeight();
    
    app.video.window.Blit(background, NULL, &shad_rect);
    app.video.window.Blit(surf, NULL, &dst_rect);
		
    world.ToRedrawOnScreen(Rectanglei(dst_rect.x, dst_rect.y,
                                      shad_rect.w + bg_offset, shad_rect.h + bg_offset));
  }
  else
  {
    app.video.window.Blit(surf, NULL, &dst_rect);
    world.ToRedrawOnScreen(Rectanglei(dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h));
  }		
}

void Text::DrawCenterOnMap (int x, int y){
  DrawTopLeftOnMap(x - surf.GetWidth()/2, y - surf.GetHeight()/2 );
}

void Text::DrawCenterTopOnMap (int x, int y){
  DrawTopLeftOnMap(x - surf.GetWidth()/2, y);
}

void Text::DrawTopLeftOnMap (int x, int y){
  if(shadowed)
    AbsoluteDraw(background, bg_offset + x, bg_offset + y);
  AbsoluteDraw(surf, x, y);
}

void DrawTmpBoxText(Font &font, int _x, int _y, 
		    const std::string &txt, uint space,
                    int boxR, int boxG, int boxB, int boxA,
                    int rectR, int rectG, int rectB, int rectA)
{
  int x, y, w, h;
  
  w = font.GetWidth(txt) + space*2;
  x = _x - w / 2;

  h = font.GetHeight(txt) + space*2;
  y = _y - h / 2;
  _y -= font.GetHeight(txt)/2;

  app.video.window.BoxRGBA(x, y, x + w, y + h,
                           boxR, boxG, boxB, boxA);

  app.video.window.RectangleRGBA(x, y, x + w, y + h,
                                 rectR, rectG, rectB, rectA);  

  world.ToRedrawOnScreen(Rectanglei(x, y, w, h));
  font.WriteCenterTop (_x, _y, txt, white_color);
}

void DrawTmpBoxTextWithReturns(Font &font, int _x, int _y, 
                               const std::string &txt, uint space,
                               int boxR, int boxG, int boxB, int boxA,
                               int rectR, int rectG, int rectB, int rectA)

{
  size_t pos          = 0;
  size_t last_pos     = 0;
  size_t max_width    = 0;
  size_t total_height = 0;
  int    x, y;
  char  *lines        = strdup(txt.c_str());

  std::vector< size_t > offsets;

  // Store offsets
  offsets.push_back(0);
  while (lines[pos] != '\0')
  {
    if (lines[pos] == '\n')
    {
      lines[pos] = 0;
      if (!lines[pos+1]) break;

      offsets.push_back(pos+1);
      int w = font.GetWidth(lines+last_pos) + space*2;
      if ((int)max_width < w) max_width = w;
      total_height += font.GetHeight(lines+last_pos);
#if DEBUG
      if (last_pos)
      {
        std::cerr << "(" << pos << "," << pos-last_pos
                  << ") >>> " << lines+last_pos << " <<<\n";
      }
#endif
      last_pos = pos+1;
    }
    pos++;
  }
  if (max_width == 0) {
    max_width = font.GetWidth(lines) + space*2;
  }

  // Initial position
  total_height += 5*space;
  x = _x - max_width / 2;
  y = _y - total_height / 2;
  app.video.window.BoxRGBA(x, y, x + max_width, y + total_height,
                           boxR, boxG, boxB, boxA);

  app.video.window.RectangleRGBA(x, y, x + max_width, y + total_height,
                                 rectR, rectG, rectB, rectA);

  world.ToRedrawOnScreen(Rectanglei(x, y, max_width, total_height));

  for( std::vector<size_t>::iterator it=offsets.begin();
       it != offsets.end();
       ++it)
  {
    font.WriteLeft(x+space, y+space, lines+(*it), white_color);
    y += font.GetHeight(lines+(*it));
  }
  offsets.clear();
  free(lines);
}
