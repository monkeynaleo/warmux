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

#ifndef SCROLL_BOX
#define SCROLL_BOX

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

  // Scroll information
  bool         scrolling;
  bool         moving;
  int          offset;
  int          scrollbar_width;

  virtual void __Update(const Point2i & mousePosition,
                        const Point2i & lastMousePosition);
  Rectanglei GetScrollThumb() const;
  Rectanglei GetScrollTrack() const;
  Point2i    GetScrollTrackPos() const;
  int GetMaxOffset() const;
  int GetTrackHeight() const;

public:
  ScrollBox(const Point2i & size);
  ~ScrollBox();

  virtual void Update(const Point2i &mousePosition,
                      const Point2i &lastMousePosition);
  virtual void Pack();

  // to add a widget
  virtual void AddWidget(Widget* widget);
  virtual void RemoveWidget(Widget* w);
  virtual void Empty();

  virtual void Draw(const Point2i & mousePosition) const;
  virtual Widget* Click(const Point2i & mousePosition, uint button);
  virtual Widget* ClickUp(const Point2i & mousePosition, uint button);
};

#endif  //SCROLL_BOX
