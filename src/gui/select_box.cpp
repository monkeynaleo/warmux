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

SelectBox::SelectBox(const Point2i& size, bool always, bool force, bool alt)
  : ScrollBox(size, force, alt)
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
  ScrollBox::Update(mousePosition, lastMousePosition);

  Surface& surf = GetMainWindow();
  Rectanglei clip(position, Point2i(size.x - scrollbar_width, size.y));
  SwapWindowClip(clip);

  if (selected_item != -1) {
    Widget *sel = m_items[selected_item];
    surf.BoxColor(*sel, selected_item_color);
  }

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
  // Let's make sure we call SelectBox method and not a child method,
  // as we are not sure of the consequences
  SelectBox::AddWidget(w);
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

//--------------------------------------------------------------------------

void ItemBox::AddItem(bool select, Widget* w, const void* value)
{
  // First put the value, because it is accessed by Select
  m_values.push_back(value);
  SelectBox::AddWidgetItem(select, w);
}

void ItemBox::Select(uint index)
{
  ASSERT(index < m_items.size());
  selected_item = index;
  NeedRedrawing();
}

void ItemBox::RemoveSelected()
{
  if (selected_item != -1) {
    SelectBox::RemoveSelected();
    m_values.erase(m_values.begin() + selected_item);
  }
}

const void* ItemBox::GetSelectedValue() const
{
  return (selected_item==-1) ? "" : m_values[selected_item];
}

void ItemBox::AddLabelItem(bool selected,
                           const std::string & label,
                           const void* value,
                           Font::font_size_t fsize,
                           Font::font_style_t fstyle,
                           const Color & color)
{
  AddItem(selected,
          new Label(label, 200, fsize, fstyle,
                    color, Text::ALIGN_TOP_LEFT, true),
          value);
}