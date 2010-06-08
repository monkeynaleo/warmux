/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 * Horizontal Box
 *****************************************************************************/

#include "gui/horizontal_box.h"

HBox::HBox(uint height, 
           bool draw_border, 
           bool _force_widget_size) :
  Box(Point2i(100, height), draw_border),
  force_widget_size(_force_widget_size)
{
}

void HBox::Pack()
{
  uint _x = position.x;
  uint max_size_y = 0;

  std::list<Widget *>::iterator it;
  for (it = widget_list.begin();
       it != widget_list.end();
       ++it) {

    if (it == widget_list.begin())
      _x += border.x - margin;

    (*it)->SetPosition(_x + margin,
                       position.y + border.y);

    if (force_widget_size) {
      (*it)->SetSize((*it)->GetSizeX(),
                     size.y - 2*border.y);
    } else {
      max_size_y = std::max(max_size_y, uint((*it)->GetSizeY()));
    }

    (*it)->Pack();

    _x = (*it)->GetPositionX()+ (*it)->GetSizeX();
  }
  size.x = _x - position.x + border.x;

  if (!force_widget_size) {
    size.y = max_size_y + 2*border.y;
  }
}

