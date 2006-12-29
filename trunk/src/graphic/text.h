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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************/

#ifndef TEXT_H
#define TEXT_H

#include <string>
#include "colors.h"
#include "font.h"
#include "surface.h"

class Text
{
  Surface surf;
  Surface background; //shadow or outline or nothing;
  std::string txt;
  Font* font;
  Color color;
  bool shadowed;
  uint bg_offset;
  uint max_width;

  void Render();
  void RenderMultiLines();
public:
  Text(const std::string &new_txt, Color new_color = white_color, 
       Font* new_font = NULL, bool shadowed = true);
  ~Text();

  //Draw method using windows coordinates
  void DrawCenter(int x, int y) const;
  void DrawCenter(const Point2i &position) const;
  void DrawTopLeft(int x, int y) const;
  void DrawTopLeft(const Point2i &position) const;
  void DrawTopRight(int x, int y) const;
  void DrawCenterTop(int x, int y) const;

  //Draw method using map coordinates
  void DrawCenterOnMap(int x, int y) const;
  void DrawTopLeftOnMap(int x, int y) const;
  void DrawCenterTopOnMap(int x, int y) const;

  void Set(const std::string &new_txt);
  const std::string& GetText() const;
  void SetColor( const Color &new_color);
  void SetMaxWidth(uint max_w);
  int GetWidth() const;
  int GetHeight() const;
};

void DrawTmpBoxText(Font &font, 
		    Point2i pos,
		    const std::string &txt, uint space=3,
		    Color boxColor = defaultColorBox,
		    Color rectColor = defaultColorRect);

void DrawTmpBoxTextWithReturns(Font &font,
			       const Point2i &position,
			       const std::string &txt, uint space=3,
			       Color boxColor = defaultColorBox,
			       Color rectColor = defaultColorRect);


#endif
