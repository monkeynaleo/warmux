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
 
#ifndef FONT_H
#define FONT_H
//-----------------------------------------------------------------------------
#include <SDL>
#include <SDL_ttf>

#include "../include/base.h"
//-----------------------------------------------------------------------------

class Font
{
public:
  TTF_Font *m_font;

public:
  Font();
  ~Font();
  void Load (const std::string& resource_id);
  void WriteLeft (int x, int y, const std::string &txt, SDL_Color color);
  void WriteLeftBottom (int x, int y, const std::string &txt, SDL_Color color);
  void WriteRight (int x, int y, const std::string &txt, SDL_Color color);
  void WriteCenterTop (int x, int y, const std::string &txt, SDL_Color color);
  void WriteCenter (int x, int y, const std::string &txt, SDL_Color color);
  uint GetWidth (const std::string &txt);
  uint GetHeight (const std::string &txt);
  SDL_Surface * Font::Render(const std::string &txt, SDL_Color color);
 };

//-----------------------------------------------------------------------------
#endif
