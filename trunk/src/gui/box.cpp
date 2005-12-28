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
 * Vertical or Horizontal Box
 *****************************************************************************/

#include <SDL_gfxPrimitives.h>
#include "box.h"
#include "../include/app.h"

#include <iostream>

//-----------------------------------------------------------------------------

Box::Box(uint _x, uint _y, uint _w, 
	 bool _horizontal, 
	 bool _visible) :
  Widget(_x,_y,_w, 1)
{
  last_widget = NULL;
  horizontal = _horizontal;
  visible = _visible;

  if (horizontal) h = _w;
}

//-----------------------------------------------------------------------------

Box::~Box()
{
  
  std::list<Widget *>::iterator it;
  for (it = widgets.begin(); 
       it != widgets.end(); 
       ++it){
    assert(it != NULL);
    widgets.erase(it);
  }
}

//-----------------------------------------------------------------------------

void Box::Draw (uint mouse_x, uint mouse_y)
{
  if (visible) {
    boxRGBA(app.sdlwindow, x, y, x+w, y+h,
	    80,80,159,206);

    rectangleRGBA(app.sdlwindow, x, y, x+w, y+h,
		  49, 32, 122, 255);  
  }

  std::list<Widget *>::iterator it;
  for (it = widgets.begin(); 
       it != widgets.end(); 
       ++it){
    (*it)->Draw(mouse_x, mouse_y);
  }
}

//-----------------------------------------------------------------------------
void Box::SetSizePosition(uint _x, uint _y, uint _w, uint _h)
{
  StdSetSizePosition(_x, _y, _w, _h);
}

//-----------------------------------------------------------------------------

bool Box::Clic (uint mouse_x, uint mouse_y)
{
  bool r=false;

  std::list<Widget *>::iterator it;
  for (it = widgets.begin(); 
       it != widgets.end(); 
       ++it){
    assert(it != NULL);
    r = (*it)->Clic(mouse_x, mouse_y);
    if (r) return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

void Box::AddWidget(Widget * a_widget)
{
  assert(a_widget != NULL);
  if (last_widget != NULL) {

    if (horizontal) {
      uint _w = a_widget->GetW();
      uint _x = last_widget->GetX()+last_widget->GetW();
      a_widget->SetSizePosition(_x+5, y+5, _w, h-10);
    } else {
      uint _h = a_widget->GetH();
      uint _y = last_widget->GetY()+last_widget->GetH();
      a_widget->SetSizePosition(x+5, _y+5, w-10, _h);
    }
  }
  else {

    if (horizontal) {
      a_widget->SetSizePosition(x+5, y+5, a_widget->GetW(), h-10);
    } else {
      a_widget->SetSizePosition(x+5, y+5, w-10, a_widget->GetH());
    }
  }
  last_widget = a_widget;

  widgets.push_back(a_widget);

  if (horizontal) {
    w = a_widget->GetX() + a_widget->GetW() - x + 5;
  } else {
    h = a_widget->GetY() + a_widget->GetH() - y + 5;
  }

}

//-----------------------------------------------------------------------------
