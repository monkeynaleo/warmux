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
 * Widget list : store all widgets displayed on one screen
 * It is a fake widget
 *****************************************************************************/
#ifndef WIDGET_LIST_H
#define WIDGET_LIST_H

#include <list>
#include <SDL_keyboard.h>
#include "widget.h"
#include "container.h"

class WidgetList : public Widget, public Container
{
private:
  Point2i lastMousePosition;
  Widget* last_clicked;

protected:
  std::list<Widget*> widget_list;
  virtual void DelFirstWidget(); // usefull only for message_box

public:
  WidgetList();
  WidgetList(const Rectanglei &rect);
  virtual ~WidgetList();

  // methods specialized from Widget to manage the list of widgets
  virtual void SendKey(SDL_keysym key);
  virtual Widget* Clic(const Point2i &mousePosition, uint button);
  virtual void Draw(const Point2i &mousePosition);

  // needed to implements Widget
  virtual void SetSizePosition(const Rectanglei &rect) {};  

  // to add a widget
  virtual void AddWidget(Widget*);

  // redraw bottom layer container
  virtual void Redraw(const Rectanglei& rect);
};

#endif // WIDGET_LIST_H
