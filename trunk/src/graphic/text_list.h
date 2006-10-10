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
 * List of Text Clases. 
 * TextBox is not valid while playing game
 *****************************************************************************/

#ifndef TEXT_LIST_H
#define TEXT_LIST_H

#define HEIGHT 15
#define XPOS 25
#define YPOS 130
#define MAXLINES 10 //Fidel's advise

#include "text.h"
#include <list>

class TextList{
 public:
  std::list<Text*> list;
  typedef std::list<Text*>::iterator iterator;
  
 public:
  // Constructors
  TextList();
  ~TextList();

  // Manage items in list
  void AddText(const std::string &txt);
  void Clear();
  void Draw();
  void DrawLine(Text* newline);

};

#endif
