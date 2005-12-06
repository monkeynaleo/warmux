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
 * Vertical Box
 *****************************************************************************/

#include "vbox.h"
#include <iostream>
//-----------------------------------------------------------------------------

Vbox::Vbox(uint _x, uint _y, uint _w, uint _h) :
  Widget(_x,_y,_w,_h)
{
}

//-----------------------------------------------------------------------------

Vbox::~Vbox()
{
  
  std::list<Widget *>::iterator it;
  for (it = widgets.begin(); 
       it != widgets.end(); 
       ++it){
    widgets.erase(it);
  }
}

//-----------------------------------------------------------------------------

void Vbox::Draw (uint mouse_x, uint mouse_y)
{
  std::list<Widget *>::iterator it;
  for (it = widgets.begin(); 
       it != widgets.end(); 
       ++it){
    (*it)->Draw(mouse_x, mouse_y);
  }
}

//-----------------------------------------------------------------------------

// bool Vbox::Clic (uint mouse_x, uint mouse_y)
// {
//   bool r=false;

//   std::list<Widget *>::iterator it;
//   for (it = widgets.begin(); 
//        it != widgets.end(); 
//        ++it){
//     std::cout << "Do we crash ?" << std::endl;
//     r = (*it)->Clic(mouse_x, mouse_y);
//     std::cout << "No :-)" << std::endl;
//     if (r) return true;
//   }

//   return false;
// }

//-----------------------------------------------------------------------------

void Vbox::AddWidget(Widget * a_widget)
{
  if (!widgets.empty()) {
    Widget * prev = (*(--widgets.end()));
    a_widget->SetSizePosition(x, prev->GetY()+prev->GetH(), w, a_widget->GetH());
  }
  else 
    a_widget->SetSizePosition(x, y, w, a_widget->GetH());

  widgets.push_back(a_widget);

}

//-----------------------------------------------------------------------------
