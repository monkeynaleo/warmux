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
 * Label in GUI.
 *****************************************************************************/

#include "label.h"

Label::Label (const std::string &label, const Rectanglei &rect, Font& _font,
	      const Color& color, bool _center) 
  : font_color(color)
{
  position = rect.GetPosition();
  size = rect.GetSize();
  size.y = _font.GetHeight();
  font = &_font;
  center = _center;
  txt_label = new Text(label, font_color, &_font);
}

Label::~Label()
{
  delete txt_label;
}

void Label::Draw(const Point2i &mousePosition, Surface& surf)
{
  if (!center)
    txt_label->DrawTopLeft(position);
  else
    txt_label->DrawCenterTop(position.x + size.x/2, position.y);
}

void Label::SetSizePosition(const Rectanglei &rect)
{
  StdSetSizePosition(rect);
}

void Label::SetText(std::string &new_txt)
{
  need_redrawing = true;
  delete txt_label;
  txt_label = new Text(new_txt, font_color, font);
}

std::string& Label::GetText()
{
  return txt_label->GetText();
}

