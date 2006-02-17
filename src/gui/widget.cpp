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
 * Widget
 *****************************************************************************/

#include "widget.h"
#include "../tool/point.h"

Widget::Widget(){

}

Widget::Widget(const Rectanglei &rect){
	position = rect.GetPosition();
	size = rect.GetSize();
}

Widget::~Widget(){
}

bool Widget::MouseIsOver (uint mouse_x, uint mouse_y){
  Point2i mouse(mouse_x, mouse_y);
  return Contains( mouse );

}

bool Widget::Clic (uint mouse_x, uint mouse_y, uint button){
  return false;
}

void Widget::StdSetSizePosition(const Rectanglei &rect){
	position = rect.GetPosition();
	size = rect.GetSize();
}

