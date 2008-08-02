/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
#include <SDL_keyboard.h>
#include "gui/widget_list.h"
#include "gui/widget.h"
#include "interface/mouse.h"
#include <iostream>

WidgetList::WidgetList()
{
  last_clicked = NULL;
  selected_widget = NULL;
}

WidgetList::WidgetList(const Point2i &size) : Widget(size)
{
  last_clicked = NULL;
  selected_widget = NULL;
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
  ASSERT(w!=NULL);
  widget_list.push_back(w);
  w->SetContainer(this);
}

void WidgetList::RemoveWidget(Widget* w)
{
  ASSERT(w!=NULL);
  widget_list.remove(w);
  w->SetContainer(NULL);
  delete w;
}

void WidgetList::Update(const Point2i &mousePosition)
{
  // if (selected_widget != NULL && !selected_widget->Contains(mousePosition)) {
  //   selected_widget->SetFocus(false);
  //   selected_widget = NULL;
  // }

  for (std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    // Then redraw the widget
    (*w)->Update(mousePosition, lastMousePosition);
    // if (lastMousePosition != mousePosition && (*w)->Contains(mousePosition)) {
    //   selected_widget = (*w);
    //   selected_widget->SetHighlighted(true);
    // }

    // if ((*w) != selected_widget && !(*w)->Contains(mousePosition)) {
    //   (*w)->SetHighlighted(false);
    // }
  }

  lastMousePosition = mousePosition;
}

Widget* WidgetList::GetFirstWidget() const
{
  Widget *first = NULL;

  MSG_DEBUG("widgetlist", "%p::GetFirstWidget()", this);

  for (std::list<Widget*>::const_iterator it = widget_list.begin();
       it != widget_list.end();
       it++) {
    if ((*it)->IsWidgetList()) {
      first = static_cast<WidgetList*>(*it)->GetFirstWidget();
      if (first != NULL)
	return first;
    } else {
      return (*it);
    }
  }

  return NULL;
}

Widget* WidgetList::GetLastWidget() const
{
  Widget *last = NULL;

  for (std::list<Widget*>::const_reverse_iterator it = widget_list.rbegin();
       it != widget_list.rend();
       it++) {
    if ((*it)->IsWidgetList()) {
      last = static_cast<WidgetList*>(*it)->GetLastWidget();
      if (last != NULL)
	return last;
    } else {
      return (*it);
    }
  }

  return NULL;
}

Widget* WidgetList::GetNextWidget(const Widget *w, bool loop) const
{
  Widget *r = NULL;

  ASSERT(!w || !w->IsWidgetList());

  MSG_DEBUG("widgetlist", "%p::GetNextWidget(%s:%p)", this, typeid(w).name(), w);

  if (widget_list.size() == 0) {
    return NULL;
  }

  if (w == NULL) {
    r = GetFirstWidget();
    MSG_DEBUG("widgetlist", "%p::GetNextWidget(%s:%p) ==> %s%p", this, typeid(w).name(), w, typeid(r).name(), r);
    return r;
  }

  std::list<Widget*>::const_iterator it;
  for (it = widget_list.begin(); it != widget_list.end(); it++) {

    MSG_DEBUG("widgetlist", "iterate on %s:%p", typeid(*it).name(), (*it));

    if (w == (*it)) {
      MSG_DEBUG("widgetlist", "we have found %s:%p", typeid(*it).name(), (*it));

      it++;
      if (it != widget_list.end())
	r = (*it);
      else if (loop)
	r = GetFirstWidget();
      else
	r = (Widget*)w;
      break;
    }

    if ((*it)->IsWidgetList()) {
      r = static_cast<WidgetList*>(*it)->GetNextWidget(w, false);

      if (r && r == w && it != widget_list.end()) {
	MSG_DEBUG("widgetlist", "r == w %s:%p", typeid(r).name(), (r));
	it++;
	if (it != widget_list.end()) {
	  r = (*it);
	  MSG_DEBUG("widgetlist", "r ==>  %s:%p", typeid(r).name(), (r));
	  if (r->IsWidgetList()) {
	    r = static_cast<WidgetList*>(r)->GetFirstWidget();
	  }
	} else if (loop) {
	  r = GetFirstWidget();
	}
      }
      if (r)
	break;
    }
  }

  ASSERT(!r || !r->IsWidgetList());

  MSG_DEBUG("widgetlist", "%p::GetNextWidget(%s:%p) ==> %s%p", this, typeid(w).name(), w, typeid(r).name(), r);

  return r;
}

void WidgetList::SetFocusOnNextWidget()
{
  // No widget => exit
  if (widget_list.size() == 0) {
    selected_widget = NULL;
    return;
  }

  // Previous selection ?
  if (selected_widget != NULL) {
    selected_widget->SetFocus(false);
  }

  MSG_DEBUG("widgetlist", "before %s:%p", typeid(selected_widget).name(), selected_widget);

  selected_widget = GetNextWidget(selected_widget, true);
  if (selected_widget != NULL) {
    selected_widget->SetFocus(true);
    Mouse::GetInstance()->SetPosition(selected_widget->GetPosition() +
				      selected_widget->GetSize()/2);
  }
  MSG_DEBUG("widgetlist", "after %s:%p", typeid(selected_widget).name(), selected_widget);

}

Widget* WidgetList::GetPreviousWidget(const Widget *w, bool loop) const
{
  Widget *r = NULL;

  if (widget_list.size() == 0) {
    return NULL;
  }

  if (w == NULL) {
    r = GetLastWidget();
    return r;
  }

  for (std::list<Widget*>::const_reverse_iterator it = widget_list.rbegin();
       it != widget_list.rend();
       it++) {
    if (w == (*it)) {
      it++;
      if (it != widget_list.rend())
	r = (*it);
      else if (loop)
	r = (*widget_list.rbegin());
      else
	r = NULL;
      break;
    }
  }

  return r;
}

void WidgetList::SetFocusOnPreviousWidget()
{
  // No widget => exit
  if (widget_list.size() == 0) {
    selected_widget = NULL;
    return;
  }

  // Previous selection ?
  if (selected_widget != NULL) {
    selected_widget->SetFocus(false);
  }

  selected_widget = GetPreviousWidget(selected_widget, true);
  if (selected_widget != NULL) {
    selected_widget->SetFocus(true);
    Mouse::GetInstance()->SetPosition(selected_widget->GetPosition() +
				      selected_widget->GetSize()/2);
  }
}

bool WidgetList::SendKey(SDL_keysym key)
{
  if (last_clicked != NULL)
    return last_clicked->SendKey(key);

  return false;
}

void WidgetList::Draw(const Point2i &mousePosition) const
{
  for (std::list<Widget*>::const_iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    (*w)->Draw(mousePosition);
  }
}

Widget* WidgetList::ClickUp(const Point2i &mousePosition, uint button)
{
  for(std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    if((*w)->Contains(mousePosition))
    {
      Widget* child = (*w)->ClickUp(mousePosition,button);
      if(child != NULL)
      {
        SetMouseFocusOn(child);
        return child;
      }
    }
  }
  return NULL;
}

Widget* WidgetList::Click(const Point2i &mousePosition, uint button)
{
  for(std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    if((*w)->Contains(mousePosition))
    {
      (*w)->Click(mousePosition,button);
    }
  }
  return NULL;
}

void WidgetList::NeedRedrawing()
{
  need_redrawing = true;

  for(std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    (*w)->NeedRedrawing();
  }
}

void WidgetList::SetMouseFocusOn(Widget* w)
{
  if(last_clicked != NULL) {
    last_clicked->SetFocus(false);
  }

  if (w != NULL) {
    last_clicked = w ;
    last_clicked->SetFocus(true);
  }
}

void WidgetList::Pack()
{
  for(std::list<Widget*>::iterator w=widget_list.begin();
      w != widget_list.end();
      w++)
  {
    (*w)->Pack();
  }
}
