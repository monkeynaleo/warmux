/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Widget
 *****************************************************************************/

#include "gui/widget.h"

#include "graphic/colors.h"
#include "graphic/surface.h"
#include "gui/container.h"
#include "tool/debug.h"

Widget::Widget():
  Rectanglei(),
  has_focus(false),
  visible(true),
  is_highlighted(false),
  border_color(white_color),
  border_size(0),
  background_color(transparent_color),
  highlight_bg_color(transparent_color),
  ct(NULL),
  need_redrawing(true)
{
}

Widget::Widget(const Rectanglei &rect):
  Rectanglei(rect),
  has_focus(false),
  visible(true),  
  border_color(white_color),
  border_size(0),
  background_color(transparent_color),
  highlight_bg_color(transparent_color),
  ct(NULL),
  need_redrawing(true)
{
}

void Widget::StdSetSizePosition(const Rectanglei &rect)
{
  position = rect.GetPosition();
  size = rect.GetSize();
}

// From Container: it redraws the border and the background
void Widget::RedrawBackground(const Rectanglei& rect,
			      Surface& surf)
{
  if (ct != NULL) 
    ct->RedrawBackground(rect, surf);

  if (!visible)
    return;

  if (IsHighlighted() && highlight_bg_color != transparent_color) {
    surf.BoxColor(*this, highlight_bg_color);
  } else if (background_color != transparent_color) {
    surf.BoxColor(rect, background_color);
  }
  
  if (border_size != 0 && border_color != transparent_color
      && rect == *this)
    surf.RectangleColor(*this, border_color, border_size);

  if (IsDEBUGGING("widget.border"))
    surf.RectangleColor(*this, c_red, border_size);
}

void Widget::Update(const Point2i &mousePosition,
                    const Point2i &lastMousePosition,
                    Surface& surf)
{
  if (need_redrawing ||
      (Contains(mousePosition) && mousePosition != lastMousePosition) ||
      (Contains(lastMousePosition) && !Contains(mousePosition))) {

    // Redraw the border and the background
    RedrawBackground(*this, surf);

    __Update(mousePosition, lastMousePosition, surf);

    if (visible) {
      Draw(mousePosition, surf);
    }
  }
  need_redrawing = false;
}

void Widget::SetFocus(bool focus)
{
  has_focus = focus;
  is_highlighted = focus;
  NeedRedrawing();
}

Widget* Widget::Click(const Point2i &mousePosition, uint /* button */)
{
  if (Contains(mousePosition)) {
    NeedRedrawing();
    return this;
  }
  return NULL;
}

Widget* Widget::ClickUp(const Point2i &mousePosition, uint /* button */)
{
  if (Contains(mousePosition)) {
    NeedRedrawing();
    return this;
  }
  return NULL;
}

void Widget::SetVisible(bool _visible)
{
  if (visible != _visible) {
    visible = _visible;
    NeedRedrawing();
  }
}

void Widget::SetBorder(const Color &_border_color, uint _border_size)
{
  if (border_color != _border_color ||
      border_size != _border_size) {
    border_color = _border_color;
    border_size = _border_size;
    NeedRedrawing();
  }
}

void Widget::SetBackgroundColor(const Color &bg_color)
{
  if (background_color != bg_color) {
    background_color = bg_color;
    NeedRedrawing();
  }
}

bool Widget::IsHighlighted() const 
{ 
  return (is_highlighted || HasFocus()); 
}
  
void Widget::SetHighlighted(bool focus)
{
  is_highlighted = focus;
  NeedRedrawing();
}

void Widget::SetHighlightBgColor(const Color &_highlight_bg_color)
{
  if (highlight_bg_color != _highlight_bg_color) {
    highlight_bg_color = _highlight_bg_color;
    NeedRedrawing();
  }
}
