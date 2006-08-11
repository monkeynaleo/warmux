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
 * It is a fake widget.
 *****************************************************************************/
#include "../include/app.h"
#include "../graphic/colors.h"
#include "widget_list.h"
#include "widget.h"
#include <iostream>

WidgetList::WidgetList()
{
  last_clicked = NULL;
}

WidgetList::WidgetList(const Rectanglei &rect) : Widget(rect)
{
  last_clicked = NULL;
}

WidgetList::~WidgetList()
{
  for(std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
    delete *w;

  widget_list.clear();
}

void WidgetList::DelFirstWidget()
{
  delete widget_list.front();
  widget_list.pop_front();
}

void WidgetList::AddWidget(Widget* w)
{
  assert(w!=NULL);
  widget_list.push_back(w);
  w->SetContainer(this);
}

void WidgetList::Draw(const Point2i &mousePosition)
{
  for(std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    // Then redraw the widget
    (*w)->Update(mousePosition, lastMousePosition);
  }

  lastMousePosition = mousePosition;
}


void WidgetList::Redraw(const Rectanglei& rect)
{
  // Redraw bottom layer
  if (ct != NULL) {
    ct->Redraw(rect);
  }
}

void WidgetList::SendKey(SDL_keysym key)
{
  if(last_clicked != NULL)
    last_clicked -> SendKey(key);
}

Widget* WidgetList::Clic(const Point2i &mousePosition, uint button)
{
  for(std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    if((*w)->Contains(mousePosition))
    {
      Widget* child = (*w)->Clic(mousePosition,button);
      if(child != NULL)
      {
        if(last_clicked != NULL) last_clicked->have_focus = false;
        last_clicked = child;
        last_clicked->have_focus = true;
        return child;
      }
    }
  }
  return NULL;
}

