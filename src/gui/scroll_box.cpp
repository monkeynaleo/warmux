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
 * Vertical Scroll Box
 *****************************************************************************/

#include <SDL.h>
#include "graphic/video.h"
#include "gui/button.h"
#include "gui/vertical_box.h"
#include "gui/scroll_box.h"
#include "include/app.h"

#define SCROLL_SPEED  6
#define BORDER        2
#define NO_DRAG      -1

ScrollBox::ScrollBox(const Point2i & _size, bool force_widget_size)
  : WidgetList(_size)
  , m_up(NULL)
  , m_down(NULL)
  , start_drag_y(-1)
  , start_drag_offset(NO_DRAG)
  , offset(0)
{
  // Load buttons
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  m_up = new Button(res, "menu/up");
  m_down = new Button(res, "menu/down");
  GetResourceManager().UnLoadXMLProfile(res);

  Widget::SetBorder(white_color, 1);
  Widget::SetBackgroundColor(defaultListColor1);

  scrollbar_width = m_up->GetSizeX();
  // Let's consider the scrollbar is not displayed for now.
  vbox = new VBox(_size.x - 2*BORDER - scrollbar_width, false, force_widget_size);
  vbox->SetBorder(Point2i(BORDER, BORDER));
  vbox->SetMargin(BORDER);

  WidgetList::AddWidget(vbox);
  WidgetList::AddWidget(m_up);
  WidgetList::AddWidget(m_down);
}

ScrollBox::~ScrollBox()
{
}

Widget * ScrollBox::ClickUp(const Point2i & mousePosition, uint button)
{
  bool was_drag = start_drag_offset != NO_DRAG;
  start_drag_offset = NO_DRAG;

  if (!vbox->GetFirstWidget()) {
    return NULL;
  }

  if (vbox->Contains(mousePosition)) {
    Widget *w = vbox->ClickUp(mousePosition, button);
    if (w) {
      return w;
    }

    // The click was not handled, let's try using it for scrolling
  }

  if (HasScrollBar()) {
    bool is_click   = Mouse::IS_CLICK_BUTTON(button);
    int  max_offset = GetMaxOffset();
    int  new_offset = offset;

    // The event involves the scrollbar or its buttons
    if ((button == SDL_BUTTON_WHEELDOWN && Contains(mousePosition)) ||
        (is_click && m_down->Contains(mousePosition))) {

      // bottom button
      new_offset = offset+SCROLL_SPEED;
      if (new_offset > max_offset)
        new_offset = max_offset;
    } else if ((button == SDL_BUTTON_WHEELUP && Contains(mousePosition)) ||
               (is_click && m_up->Contains(mousePosition))) {

      // top button
      new_offset = offset-SCROLL_SPEED;
      if (new_offset < 0)
        new_offset = 0;
    } else if (is_click) {
      // Was it released after a drag operation?
      if (was_drag) // or start_drag_y != mousePosition.y
        return this;
      Rectanglei scroll_track = GetScrollTrack();
      if (scroll_track.Contains(mousePosition)) {
        // Set this as new scroll thumb position
        new_offset = ((mousePosition.y - scroll_track.GetPositionY()) * (size.y+max_offset))
                   / scroll_track.GetSizeY();
      }
    }

    if (new_offset != offset) {
      offset = new_offset;
      Pack();
      NeedRedrawing();
    }
  }

  return NULL;
}

Widget * ScrollBox::Click(const Point2i & mousePosition, uint button)
{
  if (!Contains(mousePosition)) {
    return NULL;
  }

  if (HasScrollBar()) {
    // If the position is inside the scrollthumb, it really is a drag operation
    if (GetScrollThumb().Contains(mousePosition) && Mouse::IS_CLICK_BUTTON(button)) {
      start_drag_y = mousePosition.y;
      if (!offset) {
        // Not yet set, derive from mouse position
        Rectanglei scroll_track = GetScrollTrack();
        offset = ((mousePosition.y - scroll_track.GetPositionY()) * (size.y+GetMaxOffset()))
               / scroll_track.GetSizeY();
      }
      start_drag_offset = offset;
    } else {
      start_drag_offset = NO_DRAG;
    }
  }

  return WidgetList::Click(mousePosition, button);
}

void ScrollBox::__Update(const Point2i & mousePosition,
                         const Point2i & /*lastMousePosition*/)
{
  //printf("__Update: size=%ix%i max=%i\n", size.x, size.y, GetMaxOffset());

  // update position of items because of scrolling with scroll bar
  if (HasScrollBar() && start_drag_offset>0) {
    Point2i track_pos  = GetScrollTrackPos();
    int     height     = GetTrackHeight();
    int     max_offset = GetMaxOffset();
    int     new_offset = start_drag_offset +
             ((mousePosition.y - start_drag_y) * (size.y+max_offset))/height;
    if (new_offset < 0)
      new_offset = 0;
    if (new_offset > max_offset)
      new_offset = max_offset;

    if (new_offset != offset) {
      offset = new_offset;
      Pack();
    }
  }
}

void ScrollBox::AddWidget(Widget* widget)
{
  vbox->AddWidget(widget);
}

void ScrollBox::RemoveWidget(Widget* w)
{
  vbox->RemoveWidget(w);
}

void ScrollBox::RemoveFirstWidget()
{
  Widget *w = vbox->GetFirstWidget();
  if (w) {
    RemoveWidget(w);
  }
}

size_t ScrollBox::WidgetCount() const
{
  return vbox->WidgetCount();
}

Point2i ScrollBox::GetScrollTrackPos() const
{
  return Point2i(position.x + size.x - BORDER - scrollbar_width,
                 position.y + BORDER + m_up->GetSizeY());
}

Rectanglei ScrollBox::GetScrollTrack() const
{
  return Rectanglei(GetScrollTrackPos(),
                    Point2i(scrollbar_width, GetTrackHeight()));
}

Rectanglei ScrollBox::GetScrollThumb() const
{
  // Height: (part of the vbox that is displayed / vbox size) * scrollbar height
  Rectanglei scroll_track = GetScrollTrack();
  uint tmp_h = ((size.y - 2*BORDER) * scroll_track.GetSizeY())
             / ((size.y - 2*BORDER) + GetMaxOffset());
  // Start position: from the offset
  uint tmp_y = scroll_track.GetPositionY()
             + (offset * scroll_track.GetSizeY()) / (size.y + GetMaxOffset());
  if (tmp_h < 6)
    tmp_h = 6;
  return Rectanglei(scroll_track.GetPositionX(), tmp_y,
                    scrollbar_width, tmp_h);
}

int ScrollBox::GetMaxOffset() const
{
  return vbox->GetSizeY() - size.y;
}

int ScrollBox::GetTrackHeight() const
{
  return size.y - 2*(m_up->GetSizeY()+BORDER);
}

void ScrollBox::Empty()
{
  // We want to leave around the buttons and the box
  if (vbox)
    vbox->Empty();
}

void ScrollBox::Clear()
{
  // We want to leave around the buttons and the box
  if (vbox)
    vbox->Clear();
}

void ScrollBox::Update(const Point2i &mousePosition,
                       const Point2i &lastMousePosition)
{
  bool redraw = need_redrawing;
  Widget::Update(mousePosition, lastMousePosition);
  need_redrawing = redraw;

  bool has_scrollbar = HasScrollBar();
  m_up->SetVisible(has_scrollbar);
  m_down->SetVisible(has_scrollbar);

  //printf("Update: size=%ix%i max=%i\n", size.x, size.y, GetMaxOffset());
  WidgetList::Update(mousePosition, lastMousePosition);

  if (has_scrollbar) {
    GetMainWindow().BoxColor(GetScrollTrack(), dark_gray_color);

    Rectanglei thumb = GetScrollThumb();
    bool over = start_drag_offset>0 || thumb.Contains(mousePosition);
    GetMainWindow().BoxColor(thumb, over ? white_color : gray_color);
  }
}

void ScrollBox::Pack()
{
  // Make a first guess about the box properties
  vbox->SetSizeX(size.x -2*BORDER - scrollbar_width);
  vbox->Pack();

  //printf("Pack: size=%ix%i max=%i\n", size.x, size.y, GetMaxOffset());

  // No that we known better, account for the scrollbar
  if (HasScrollBar()) {
    vbox->SetPosition(position.x + BORDER, position.y + BORDER - offset);
  } else {
    vbox->SetPosition(position.x + BORDER, position.y + BORDER);
  }
  m_up->SetPosition(position.x + size.x - m_up->GetSizeX() - BORDER,
                    position.y + BORDER);
  m_down->SetPosition(position + size - m_down->GetSize() - BORDER);

  WidgetList::Pack();
}

bool ScrollBox::SendKey(const SDL_keysym & key)
{
  if (!WidgetList::SendKey(key)) {
    int new_offset = offset;
    switch(key.sym)
    {
    case SDLK_PAGEUP:
      new_offset -= size.y;
      break;
    case SDLK_PAGEDOWN:
      new_offset += size.y;
      break;
    default:
      return false;
    }

    if (new_offset < 0)
      new_offset = 0;
    if (new_offset > GetMaxOffset())
      new_offset = GetMaxOffset();

    if (new_offset != offset) {
      offset = new_offset;
      Pack();
      //NeedRedrawing();
    }
  }
  return true;
}
