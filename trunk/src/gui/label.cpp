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
#include "../include/app.h"

Label::Label (const std::string &label, const Rectanglei &rect, Font& _font){
  position = rect.GetPosition();
  size = rect.GetSize();
  size.y = _font.GetHeight();
  font = &_font;
  txt_label = new Text(label, white_color, &_font);
}

Label::~Label(){
  delete txt_label;
}

void Label::Draw(const Point2i &mousePosition){
  txt_label->DrawTopLeft(position);
}

void Label::SetSizePosition(const Rectanglei &rect){
  StdSetSizePosition(rect);
}

void Label::SetText(std::string &new_txt)
{
  delete txt_label;
  txt_label = new Text(new_txt, white_color, font);
}

std::string& Label::GetText()
{
  return txt_label->GetText();
}

