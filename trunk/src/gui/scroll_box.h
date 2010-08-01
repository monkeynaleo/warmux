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

#ifndef VERTICAL_SCROLL_BOX
#define VERTICAL_SCROLL_BOX

#include "widget_list.h"

class VBox;
class Button;

class ScrollBox : public WidgetList
{
protected:
  // Internal box
  VBox *vbox;

  // Buttons
  Button * m_up;
  Button * m_down;

  // Colors
  Color selected_item_color;
  Color default_item_color;

  // Scroll information
  bool         always_one_selected;
  bool         scrolling;
  bool         moving;
  int          offset;
  Widget      *highlit;

  virtual void __Update(const Point2i & mousePosition,
                        const Point2i & lastMousePosition);
  Rectanglei GetScrollThumb() const;
  Rectanglei GetScrollTrack() const;
  Point2i    GetScrollTrackPos() const;
  int GetMaxOffset() const;
  int GetTrackHeight() const;

public:
  void SetSelectedItemColor(const Color& color) { selected_item_color = color; };
  void SetDefaultItemColor(const Color& color) { default_item_color = color; };

  ScrollBox(const Point2i & size, bool always_one_selected = true);
  ScrollBox(Profile * profile, const xmlNode * baseListBoxNode);
  virtual ~ScrollBox();

  //virtual bool LoadXMLConfiguration(void);

  // to add a widget
  virtual void AddWidget(Widget* widget);
  virtual void RemoveWidget(Widget* w);

  virtual void Draw(const Point2i & mousePosition) const;
  virtual Widget* Click(const Point2i & mousePosition, uint button);
  virtual Widget* ClickUp(const Point2i & mousePosition, uint button);
  void Update(const Point2i & mousePosition,
              const Point2i & lastMousePosition);

  virtual void SetFocusOn(Widget* widget, bool force_mouse_position = false);
};

#endif  //VERTICAL_SCROLL_BOX
