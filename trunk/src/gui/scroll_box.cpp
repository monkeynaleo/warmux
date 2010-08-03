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

//#include "graphic/text.h"
#include "graphic/video.h"
#include "gui/button.h"
#include "gui/vertical_box.h"
#include "gui/scroll_box.h"
#include "include/app.h"

#define SCROLL_SPEED  6
#define BORDER        2

ScrollBox::ScrollBox(const Point2i & _size)
  : WidgetList(_size)
  , m_up(NULL)
  , m_down(NULL)
  , start_scroll_y(-1)
  , start_offset(0)
  , moving(false)
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
  vbox = new VBox(_size.x - 2*BORDER - scrollbar_width, false, true);
  vbox->SetBorder(Point2i(BORDER, BORDER));
  vbox->SetMargin(BORDER);

  WidgetList::AddWidget(m_up);
  WidgetList::AddWidget(m_down);
  WidgetList::AddWidget(vbox);
}

ScrollBox::~ScrollBox()
{
}

Widget * ScrollBox::ClickUp(const Point2i & mousePosition, uint button)
{
  start_offset = 0;
  moving = false;

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
    bool is_click = Mouse::IS_CLICK_BUTTON(button);

    // The event involves the scrollbar or its buttons
    if ((button == SDL_BUTTON_WHEELDOWN && Contains(mousePosition)) ||
        (is_click && m_down->Contains(mousePosition))) {

      // bottom button
      offset += SCROLL_SPEED;
      if (offset > GetMaxOffset())
        offset = GetMaxOffset();

      Pack();
      return m_down;
    } else if ((button == SDL_BUTTON_WHEELUP && Contains(mousePosition)) ||
               (is_click && m_up->Contains(mousePosition))) {

      // top button
      offset -= SCROLL_SPEED;
      if (offset < 0)
        offset = 0;

      Pack();
      return m_up;
    } else if (is_click) {
      if (start_scroll_y == mousePosition.y)
        return this;
      Rectanglei scroll_track = GetScrollTrack();
      if (scroll_track.Contains(mousePosition)) {
        // Set this as new scroll thumb position
        offset = ((mousePosition.y - scroll_track.GetPositionY()) * GetMaxOffset())
               / scroll_track.GetSizeY();
        Pack();
        return this;
      }
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
    if (GetScrollThumb().Contains(mousePosition) && Mouse::IS_CLICK_BUTTON(button)) {
      start_scroll_y = mousePosition.y;
      if (!offset) {
        // Not yet set, derive from mouse position
        Rectanglei scroll_track = GetScrollTrack();
        offset = ((mousePosition.y - scroll_track.GetPositionY()) * GetMaxOffset())
               / scroll_track.GetSizeY();
      }
      start_offset = offset;
    }

    if (!moving && (m_down->Contains(mousePosition) || m_up->Contains(mousePosition))) {
      moving = true;
    }
  }

  return WidgetList::Click(mousePosition, button);
}

void ScrollBox::__Update(const Point2i & mousePosition,
                         const Point2i & /*lastMousePosition*/)
{
  if (!Contains(mousePosition)) {
    start_offset = 0;
    moving = false;
    return;
  }

  //printf("__Update: size=%ix%i max=%i\n", size.x, size.y, GetMaxOffset());
  if (HasScrollBar()) {
    if (moving) {
      // Does not work because called only once, waiting for
      // new events before reentering => we should try to
      // generate activity (fake event?) ?
      if (m_down->Contains(mousePosition)) {
        // bottom button
        offset += SCROLL_SPEED;
        if (offset > GetMaxOffset())
          offset = GetMaxOffset();
        Pack();
        return;
      } else if (m_up->Contains(mousePosition)) {
        // top button
        offset -= SCROLL_SPEED;
        if (offset < 0)
          offset = 0;
        Pack();
        return;
      }
      moving = false;
      return;
    }

    // update position of items because of scrolling with scroll bar
    if (start_offset) {
      Point2i track_pos  = GetScrollTrackPos();
      int     height     = GetTrackHeight();
      int     max_offset = GetMaxOffset();
      offset = start_offset +
               ((mousePosition.y - start_scroll_y) * max_offset)/height;
      if (offset < 0)
        offset = 0;
      if (offset > max_offset)
        offset = max_offset;
      Pack();
      return;
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
  return size.y - 2*(m_up->GetSizeY()+BORDER+1);
}

void ScrollBox::Empty()
{
  // We want to leave around the buttons and the box
  vbox->Empty();
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
    bool over = start_offset || thumb.Contains(mousePosition);
    GetMainWindow().BoxColor(thumb, over ? white_color : gray_color);
  }
}

void ScrollBox::Pack()
{
  // Make a first guess about the box properties
  vbox->SetSizeX(size.x -2*BORDER - scrollbar_width);

  //printf("Pack: size=%ix%i max=%i\n", size.x, size.y, GetMaxOffset());

  // No that we known better, account for the scrollbar
  m_up->SetPosition(GetScrollTrack().GetPositionX(), position.y + BORDER);
  m_down->SetPosition(position + size - m_down->GetSize() - BORDER);
  if (HasScrollBar()) {
    vbox->SetSizeX(size.x -2*BORDER - scrollbar_width);
    vbox->SetPosition(position.x + BORDER, position.y + BORDER - offset);
  } else {
    vbox->SetPosition(position.x + BORDER, position.y + BORDER);
  }

  WidgetList::Pack();
}
