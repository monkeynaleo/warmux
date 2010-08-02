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
 * Vertical Scroll Box with selection
 *****************************************************************************/

#include "graphic/video.h"
#include "gui/select_box.h"
#include "gui/vertical_box.h"
#include "include/app.h"

SelectBox::SelectBox(const Point2i& size, bool always)
  : ScrollBox(size)
  , selected_item_color(defaultListColor2)
  , default_item_color(defaultListColor3)
  , always_one_selected(always)
  , selected_item(-1)
{
  vbox->SetMargin(0);
}

void SelectBox::Update(const Point2i& mousePosition,
                       const Point2i& lastMousePosition)
{
  Surface& surf = GetMainWindow();

  Rectanglei clip = *this;
  SwapWindowClip(clip);

  if (selected_item != -1) {
    Widget *sel = m_items[selected_item];
    surf.BoxColor(*sel, selected_item_color);
  }

  ScrollBox::Update(mousePosition, lastMousePosition);

  int item = MouseIsOnWhichItem(mousePosition);
  if (item!=-1 && item!=selected_item) {
    surf.BoxColor(*(m_items[item]), default_item_color);
  }

  SwapWindowClip(clip);
}

void SelectBox::Draw(const Point2i &mousePosition) const
{
  Surface& surf = GetMainWindow();

  Rectanglei clip = *this;
  SwapWindowClip(clip);

  if (selected_item != -1) {
    Widget *sel = m_items[selected_item];
    surf.BoxColor(*sel, selected_item_color);
  }

  ScrollBox::Draw(mousePosition);

  int item = MouseIsOnWhichItem(mousePosition);
  if (item!=-1 && item!=selected_item) {
    surf.BoxColor(*(m_items[item]), default_item_color);
  }

  SwapWindowClip(clip);
}

Widget * SelectBox::ClickUp(const Point2i & mousePosition, uint button)
{
  if (Mouse::IS_CLICK_BUTTON(button)) {
    int item = MouseIsOnWhichItem(mousePosition);

    if (item!=-1) {
      if (item==selected_item && !always_one_selected) {
        Deselect();
      } else {
        Select(item);
      }

      return this;
    }
  }

  return ScrollBox::ClickUp(mousePosition, button);
}

void SelectBox::AddWidgetItem(bool select, Widget* w)
{
  AddWidget(w);
  if (select)
    Select(m_items.size()-1);
}

void SelectBox::Select(uint index)
{
  ASSERT(index < m_items.size());
  selected_item = index;
  NeedRedrawing();
}

void SelectBox::Deselect()
{
  ASSERT(always_one_selected == false);
  selected_item = -1;
  NeedRedrawing();
}

void SelectBox::RemoveSelected()
{
  ASSERT (always_one_selected == false);

  if (selected_item != -1) {
    m_items.erase(m_items.begin() + selected_item);
    selected_item =- 1;
  }
  NeedRedrawing();
}

int SelectBox::MouseIsOnWhichItem(const Point2i & mousePosition) const
{
  if (!Contains(mousePosition)) {
    return -1;
  }

  for (uint i=0; i<m_items.size(); i++) {
    if (m_items[i]->Contains(mousePosition))
      return i;
  }
  return -1;
}
