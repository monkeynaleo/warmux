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

Box::Box(uint x, uint y, uint w, uint h, 
	 bool _visible) :
  Widget(x,y,w,h)
{
  last_widget = NULL;
  visible = _visible;
  w_margin = h_margin = 5;
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

void Box::SetMargin (uint _w_margin, uint _h_margin)
{
  w_margin = _w_margin;
  h_margin = _h_margin;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VBox::VBox(uint x, uint y, uint w, bool _visible) :
  Box(x, y, w, 1, _visible)
{
}

//-----------------------------------------------------------------------------

void VBox::AddWidget(Widget * a_widget)
{
  assert(a_widget != NULL);

  uint _y;

  if (last_widget != NULL) {
    _y = last_widget->GetY()+last_widget->GetH();
  } else {
    _y = y;
  }

  a_widget->SetSizePosition(x+w_margin, 
			    _y+h_margin, 
			    w-2*w_margin, 
			    a_widget->GetH());

  last_widget = a_widget;

  widgets.push_back(a_widget);

  h = a_widget->GetY() + a_widget->GetH() - y + h_margin;
}

//-----------------------------------------------------------------------------

void VBox::SetSizePosition(uint _x, uint _y, uint _w, uint _h)
{
  x = _x;
  y = _y;

  std::list<Widget *>::iterator it;
  for (it = widgets.begin(); 
       it != widgets.end(); 
       ++it){
    assert(it != NULL);
    (*it)->SetSizePosition(x+w_margin,
			   _y+h_margin,
			   (*it)->GetW(),
			   (*it)->GetH());
    _y = (*it)->GetY() + (*it)->GetH();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

HBox::HBox(uint x, uint y, uint h, bool _visible) :
  Box(x, y, 1, h, _visible)
{
}

//-----------------------------------------------------------------------------

void HBox::AddWidget(Widget * a_widget)
{
  assert(a_widget != NULL);

  uint _x;

  if (last_widget != NULL) {
     _x = last_widget->GetX()+last_widget->GetW();
  }
  else {
    _x = x;
  }

  a_widget->SetSizePosition(_x+w_margin, 
			    y+h_margin, 
			    a_widget->GetW(), 
			    h-2*h_margin);

  last_widget = a_widget;

  widgets.push_back(a_widget);

  w = a_widget->GetX() + a_widget->GetW() - x + w_margin;
}

//-----------------------------------------------------------------------------

void HBox::SetSizePosition(uint _x, uint _y, uint _w, uint _h)
{
  //StdSetSizePosition(_x, _y, _w, _h);

  x = _x;
  y = _y;

  std::list<Widget *>::iterator it;
  for (it = widgets.begin(); 
       it != widgets.end(); 
       ++it){
    assert(it != NULL);
    (*it)->SetSizePosition(_x+w_margin,
			y+h_margin,
			(*it)->GetW(),
			(*it)->GetH());
    _x = (*it)->GetX()+ (*it)->GetW();
  }
}

//-----------------------------------------------------------------------------
