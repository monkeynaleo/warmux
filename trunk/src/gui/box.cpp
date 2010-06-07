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
 * Vertical or Horizontal Box
 *****************************************************************************/

#include "gui/box.h"
#include "graphic/surface.h"
#include "graphic/colors.h"
#include "graphic/video.h"

Box::Box(void):
  margin(0),
  border()
{
}

Box::Box(const Point2i & size, 
         bool _draw_border) : 
  WidgetList(size),
  margin(5)
{
  border.SetValues(5, 5);

  if (_draw_border) {
    Widget::SetBorder(defaultOptionColorRect, 2);
    Widget::SetBackgroundColor(defaultOptionColorBox);
  }
}

Box::Box(Profile * profile,
         const xmlNode * boxNode) :
  WidgetList(profile, boxNode),
  margin(5)
{
}

Box::~Box()
{
}

void Box::ParseXMLBoxParameters()
{
  ParseXMLBorder();
  ParseXMLBackground();
}

void Box::Update(const Point2i &mousePosition,
		 const Point2i &/*lastMousePosition*/)
{
  if (need_redrawing) {
    Widget::RedrawBackground(*this);
    Draw(mousePosition);
  }

  WidgetList::Update(mousePosition);
  need_redrawing = false;
}

// --------------------------------------------------

HBox::HBox(uint height, bool _draw_border, bool _force_widget_size) :
  Box(Point2i(100, height), _draw_border),
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

