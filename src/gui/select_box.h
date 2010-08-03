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

#ifndef SELECT_BOX
#define SELECT_BOX

#include "gui/scroll_box.h"

class SelectBox : public ScrollBox
{
protected:
  // Colors
  Color selected_item_color;
  Color default_item_color;

  // Selection
  std::vector<Widget*> m_items;
  bool always_one_selected;
  int  selected_item;

  int MouseIsOnWhichItem(const Point2i & mousePosition) const;

public:
  SelectBox(const Point2i& size, bool always_one_selected = true);

  // No need for a Draw method: the additional stuff drawn is made by Update
  virtual void Update(const Point2i& mousePosition,
                      const Point2i& lastMousePosition);
  virtual Widget* ClickUp(const Point2i & mousePosition, uint button);

  // to add a widget
  virtual void RemoveSelected();
  virtual void AddWidget(Widget* w) { m_items.push_back(w); ScrollBox::AddWidget(w); }
  virtual void AddWidgetItem(bool select, Widget* w);
  virtual void Empty() { m_items.clear(); ScrollBox::Empty(); }

  // Specific to selection
  Widget* GetSelectedItem() const { return (selected_item==-1) ? NULL : m_items[selected_item]; };
  int Size() { return m_items.size(); }

  void SetSelectedItemColor(const Color& color) { selected_item_color = color; };
  void SetDefaultItemColor(const Color& color) { default_item_color = color; };
  virtual void Select(uint index);
  void Deselect();
  Widget* MouseIsOnWhichWidget(const Point2i & mousePosition) const
  {
    int index = MouseIsOnWhichItem(mousePosition);
    return (index==-1) ? NULL : m_items[selected_item];
  }
};

class ItemBox : public SelectBox
{
  std::vector<const char*> m_values;
public:
  ItemBox(const Point2i& size, bool always = false) : SelectBox(size, always) { };
  void AddWidget(Widget* /*w*/)
  {
    fprintf(stderr, "ItemBox::AddWidget called\n");
    exit(1);
  }
  void AddItem(bool select, Widget* w, const char* value);
  void RemoveSelected();
  void Empty() { m_values.clear(); SelectBox::Empty(); }
  void Select(uint index);

  const char* GetSelectedValue() const;
};

#endif //SELECT_BOX
