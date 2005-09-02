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

#include "font.h"
//-----------------------------------------------------------------------------

Font::Font()
{ 
  m_font = NULL; 
}

//-----------------------------------------------------------------------------

Font::~Font()
{
  if (m_font != NULL) {
    TTF_CloseFont(m_font);
    m_font = NULL;
  }
}

//-----------------------------------------------------------------------------

void Font::Load (const std::string& font_name, int size) 
{
  assert (m_font == NULL);
  m_font = TTF_OpenFont(font_name, size);
}
//-----------------------------------------------------------------------------

void Font::WriteLeft (int x, int y, const std::string &txt, 
		      SDL_Color color)
{ 
  SDL_surface text_surface = Render(txt, color);
  SDL_rect dst_rect;
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.h = dst_rect.w = 0;

  SDL_BlitSurface(text_surface,NULL,screen, dst_rect);
}

// //-----------------------------------------------------------------------------

void Font::WriteLeftBottom (int x, int y, const std::string &txt,
			    SDL_Color color)
{ 
  SDL_surface text_surface = Render(txt, color);
  SDL_rect dst_rect;
  dst_rect.x = x;
  dst_rect.y = y - GetHeight();
  dst_rect.h = dst_rect.w = 0;

  SDL_BlitSurface(text_surface,NULL,screen, dst_rect);
}

// //-----------------------------------------------------------------------------

void Font::WriteRight (int x, int y, const std::string &txt,
		       SDL_Color color)
{ 
  SDL_surface text_surface = Render(txt, color);
  SDL_rect dst_rect;
  dst_rect.x = x - GetWidth(txt);
  dst_rect.y = y;
  dst_rect.h = dst_rect.w = 0;

  SDL_BlitSurface(text_surface, NULL, screen, dst_rect);
}

// //-----------------------------------------------------------------------------

void Font::WriteCenter (int txt_x, int txt_y, const std::string &txt,
			SDL_Color color)
{ 
  SDL_surface text_surface = Render(txt, color);
  SDL_rect dst_rect;
  dst_rect.x = x - GetWidth(txt)/2;
  dst_rect.y = y - GetHeight()/2;
  dst_rect.h = dst_rect.w = 0;

  SDL_BlitSurface(text_surface, NULL, screen, dst_rect);
}

// //-----------------------------------------------------------------------------

void Font::WriteCenterTop (int txt_x, int txt_y, const std::string &txt,
			   SDL_Color color)
{ 
  SDL_surface text_surface = Render(txt, color);
  SDL_rect dst_rect;
  dst_rect.x = x - GetWidth(txt)/2;
  dst_rect.y = y;
  dst_rect.h = dst_rect.w = 0;

  SDL_BlitSurface(text_surface, NULL, screen, dst_rect);
}

//-----------------------------------------------------------------------------

SDL_Surface * Font::Render(const std::string &txt, SDL_Color color)
{
  return TTF_RenderUTF8_Blended(m_font, txt, color);
}

//-----------------------------------------------------------------------------

int Font::GetWidth (const std::string &txt)
{ 
  int width=-1;
  TTF_SizeUTF8(m_font, txt, &width, NULL);
  return width;
}

//-----------------------------------------------------------------------------

uint Font::GetHeight ()
{ 
  return TTF_FontHeight(m_font);
}

//-----------------------------------------------------------------------------
