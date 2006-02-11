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
#include <SDL.h>
#include <SDL_ttf.h>
#include <map>
#include "../include/base.h"
#include "colors.h"
#include "surface.h"

class GameLoop;

class Font
{
  typedef std::map<std::string, Surface>::value_type 
    txt_sample;
  typedef std::map<std::string, Surface>::iterator 
    txt_iterator;

  std::map<std::string, Surface> surface_text_table;
  TTF_Font *m_font;
  void Write(int x, int y, Surface &surface);

public:
  Font(int size);
  ~Font();
  
  bool Load (const std::string& filename, int size);
  TTF_Font& GetTTF();

  void WriteLeft (int x, int y, const std::string &txt, SDL_Color color);
  void WriteLeftBottom (int x, int y, const std::string &txt, SDL_Color color);
  void WriteRight (int x, int y, const std::string &txt, SDL_Color color);
  void WriteCenterTop (int x, int y, const std::string &txt, SDL_Color color);
  void WriteCenter (int x, int y, const std::string &txt, SDL_Color color);
  
  int GetWidth (const std::string &txt);
  int GetHeight ();
  int GetHeight (const std::string &txt);

  Surface Render(const std::string &txt, SDL_Color color, bool cache=false);
  Surface Font::CreateSurface(const std::string &txt, SDL_Color color);
};

class GameFont : public Font
{
  GameLoop &game_loop;
  void Write(int x, int y, Surface &surface);

public:
  GameFont(GameLoop &game_loop, int size);

};

class Fonts
{
public:
  Fonts(GameLoop &game_loop);
  GameFont huge;
  GameFont large;
  GameFont big;
  GameFont normal;
  GameFont small;
  GameFont tiny;
};

#endif
