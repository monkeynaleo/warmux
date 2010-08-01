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

#define BORDER           2
#define SCROLLBAR_WIDTH 12

ScrollBox::ScrollBox(const Point2i & _size, bool always)
  : WidgetList(_size)
  , m_up(NULL)
  , m_down(NULL)
  , selected_item_color(defaultListColor2)
  , default_item_color(defaultListColor3)
  , always_one_selected(always)
  , scrolling(false)
  , offset(0)
  , highlit(NULL)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  m_up = new Button(res, "menu/up");
  m_down = new Button(res, "menu/down");
  GetResourceManager().UnLoadXMLProfile(res);

  Widget::SetBorder(white_color, 1);
  Widget::SetBackgroundColor(defaultListColor1);

  vbox = new VBox(_size.x-SCROLLBAR_WIDTH-2*(BORDER+5), false, false);
  vbox->SetBorder(Point2i(5, 5));

  track_size = Point2i(SCROLLBAR_WIDTH, GetSizeY() - 2*(m_up->GetSizeY()+BORDER));

  WidgetList::AddWidget(vbox);
  WidgetList::AddWidget(m_up);
  WidgetList::AddWidget(m_down);
}

ScrollBox::~ScrollBox()
{
}

Widget * ScrollBox::ClickUp(const Point2i & mousePosition, uint button)
{
  scrolling = false;

  if (!vbox->GetFirstWidget()) {
    return NULL;
  }

  if (vbox->Contains(mousePosition)) {
    Widget *w = vbox->ClickUp(mousePosition, button);
    if (w) {
      if (w == highlit && !always_one_selected) {
        highlit = NULL;
      } else {
        highlit = w;
      }

      NeedRedrawing();
      return w;
    }

    // The click was not handled, let's try using it for scrolling
  }

  bool is_click = Mouse::IS_CLICK_BUTTON(button);
  // buttons for listbox with more items than visible (first or last item not visible)
  if ((button == SDL_BUTTON_WHEELDOWN && Contains(mousePosition)) ||
      (is_click && m_down->Contains(mousePosition))) {

    // bottom button
    offset += 6;
    if (offset > GetMaxOffset())
      offset = GetMaxOffset();

    NeedRedrawing();
    return m_down;
  } else if ((button == SDL_BUTTON_WHEELUP && Contains(mousePosition)) ||
             (is_click && m_up->Contains(mousePosition))) {

    // top button
    offset -= 6;
    if (offset < 0)
      offset = 0;

    NeedRedrawing();
    return m_up;
  } else {
    Rectanglei scroll_track = GetScrollTrack();
    if (scroll_track.Contains(mousePosition)) {
      // Set this as new scroll thumb position
      offset = ((mousePosition.y - scroll_track.GetPositionY()) * GetMaxOffset())
             / track_size.GetY();
      NeedRedrawing();
      return this;
    }
  }

  return NULL;
}

Widget * ScrollBox::Click(const Point2i & mousePosition, uint button)
{
  if (!Contains(mousePosition)) {
    return NULL;
  }

  if (GetScrollThumb().Contains(mousePosition) && Mouse::IS_CLICK_BUTTON(button)) {
    scrolling = true;
  }

  return WidgetList::Click(mousePosition, button);
}

void ScrollBox::__Update(const Point2i & mousePosition,
                         const Point2i & lastMousePosition)
{
  (void)lastMousePosition;
  if (!Contains(mousePosition)) {
    scrolling = false;
  }

  // update position of items because of scrolling with scroll bar
  if (scrolling) {
    Rectanglei scroll_track = GetScrollTrack();
    if (scroll_track.Contains(mousePosition)) {
      offset = ((mousePosition.y - scroll_track.GetPositionY()) * GetMaxOffset())
             / track_size.GetY();
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

void ScrollBox::SetFocusOn(Widget* widget, bool force_mouse_position)
{
  if (widget!=this && widget!=m_up && widget!=m_down)
    highlit = widget;

  WidgetList::SetFocusOn(widget, force_mouse_position);
}

void ScrollBox::Draw(const Point2i &mousePosition) const
{
  m_up->SetPosition(GetScrollTrack().GetPositionX(),
                    GetPositionY() + BORDER);
  m_down->SetPosition(GetPosition() + GetSize() - m_down->GetSize() - BORDER);

  if (GetMaxOffset() > 0) {
    vbox->SetPosition(GetPositionX() + BORDER, GetPositionY() + BORDER - offset);
  } else {
    vbox->SetPosition(GetPosition() + BORDER);
  }

  vbox->Pack();
  WidgetList::Draw(mousePosition);

  if (GetMaxOffset() > 0) {
    GetMainWindow().BoxColor(GetScrollTrack(), dark_gray_color);

    Rectanglei thumb = GetScrollThumb();
    GetMainWindow().BoxColor(thumb,
                             (scrolling || thumb.Contains(mousePosition)) ? white_color : gray_color);
  }
}

Point2i ScrollBox::GetScrollTrackPos() const
{
  return Point2i(GetPositionX() + GetSizeX() - BORDER - SCROLLBAR_WIDTH,
                 GetPositionY() + BORDER + m_up->GetSizeY());
}

Rectanglei ScrollBox::GetScrollTrack() const
{
  return Rectanglei(GetScrollTrackPos(), track_size);
}

Rectanglei ScrollBox::GetScrollThumb() const
{
  // Height: (part of the vbox that is displayed / vbox size) * scrollbar height
  Rectanglei scroll_track = GetScrollTrack();
  uint tmp_h = ((GetSizeY() - 2*BORDER) * scroll_track.GetSizeY())
             / ((GetSizeY() - 2*BORDER) + GetMaxOffset());
  // Start position: from the offset
  uint tmp_y = scroll_track.GetPositionY()
             + (offset * scroll_track.GetSizeY()) / (GetSizeY() + GetMaxOffset());
  if (tmp_h < 6)
    tmp_h = 6;
  return Rectanglei(scroll_track.GetPositionX(), tmp_y,
                    SCROLLBAR_WIDTH, tmp_h);
}

int ScrollBox::GetMaxOffset() const
{
  return vbox->GetSizeY() - GetSizeY();
}
