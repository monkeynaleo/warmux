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
 * Vertical or Horizontal Box
 *****************************************************************************/

#include "gui/box.h"
#include "graphic/surface.h"
#include "graphic/colors.h"

Box::Box(const Rectanglei &rect, bool _visible) : WidgetList( rect )
{
  margin = 5;
  border.SetValues(5, 5);

  if (_visible) {
    Widget::SetBorder(defaultOptionColorRect, 2);
    Widget::SetBackgroundColor(defaultOptionColorBox);
  }
}

Box::~Box()
{
}

void Box::Update(const Point2i &mousePosition,
		   const Point2i &/*lastMousePosition*/,
		   Surface& surf)
{
  if (need_redrawing) {
    Widget::RedrawBackground(*this, surf);
    Draw(mousePosition, surf);
  }

  WidgetList::Update(mousePosition, surf);
  need_redrawing = false;
}

// --------------------------------------------------

VBox::VBox(int width, bool _visible) :
  Box(Rectanglei(-1, -1, width, -1), _visible)
{
}

void VBox::DelFirstWidget()
{
  int w_height = widget_list.front()->GetSizeY();
  WidgetList::DelFirstWidget();
  //Make all remaining widget go up:
  for( std::list<Widget*>::iterator it = widget_list.begin();
       it != widget_list.end();
       ++it )
  {
    (*it)->SetPositionY((*it)->GetPositionY() - w_height - margin);
  }
  size.y -= w_height + margin;
}

void VBox::AddWidget(Widget * a_widget)
{
  ASSERT(a_widget != NULL);

  uint _y;

  if(!widget_list.empty())
    _y = widget_list.back()->GetPositionY() + widget_list.back()->GetSizeY();
  else
    _y = position.y + border.y - margin;

  a_widget->SetSizePosition(Rectanglei(position.x + border.x,
                                       _y + margin,
                                       size.x - 2 * border.x,
                                       a_widget->GetSizeY() ));

  size.y = a_widget->GetPositionY() + a_widget->GetSizeY() - position.y + border.y;
  WidgetList::AddWidget(a_widget);
}

void VBox::SetSizePosition(const Rectanglei &rect)
{
  position = rect.GetPosition();
  int _y = rect.GetPositionY();
  std::list<Widget *>::iterator it;
  for( it = widget_list.begin();
       it != widget_list.end();
       ++it ){

    if( it == widget_list.begin() )
      _y += border.y - margin;

    (*it)->SetSizePosition( Rectanglei(position.x + border.x,
                                       _y + margin,
                                       (*it)->GetSizeX(),
                                       (*it)->GetSizeY() ));
    _y = (*it)->GetPositionY() + (*it)->GetSizeY();
  }
}

// --------------------------------------------------

HBox::HBox(int height, bool _visible) :
  Box(Rectanglei(-1, -1, -1, height), _visible)
{
}

void HBox::AddWidget(Widget * a_widget)
{
  ASSERT(a_widget != NULL);

  uint _x;

  if (!widget_list.empty())
    _x = widget_list.back()->GetPositionX() + widget_list.back()->GetSizeX();
  else
    _x = position.x + border.x - margin;

  a_widget->SetSizePosition( Rectanglei(_x + margin,
                                        position.y + border.y,
                                        a_widget->GetSizeX(),
                                        size.y - 2 * border.y) );

  size.x = a_widget->GetPositionX() + a_widget->GetSizeX() - position.x + border.x;

  WidgetList::AddWidget(a_widget);
}

void HBox::SetSizePosition(const Rectanglei &rect)
{
  position = rect.GetPosition();
  int _x = rect.GetPositionX();

  std::list<Widget *>::iterator it;
  for( it = widget_list.begin();
       it != widget_list.end();
       ++it ){

    if( it == widget_list.begin() )
      _x += border.x - margin;

    (*it)->SetSizePosition( Rectanglei(_x + margin,
                                       position.y + border.y,
                                       (*it)->GetSizeX(),
                                       (*it)->GetSizeY()) );
    _x = (*it)->GetPositionX()+ (*it)->GetSizeX();
  }
}

// --------------------------------------------------

GridBox::GridBox(uint _max_line_width, const Point2i& size_of_widget, bool _visible) :
  Box(Rectanglei(-1, -1, -1, -1), _visible)
{
  max_line_width = _max_line_width;
  widget_size = size_of_widget;
  last_line = 0;
  last_column = 0;
}

void GridBox::PlaceWidget(Widget * a_widget, uint _line, uint _column)
{
  uint _x, _y;

  _x = position.x + border.x + _column * (widget_size.GetX() + margin);
  _y = position.y + border.y + _line * (widget_size.GetY() + margin);

  a_widget->SetSizePosition( Rectanglei(_x, _y,
                                        a_widget->GetSizeX(),
                                        a_widget->GetSizeY()) );
}

uint GridBox::NbWidgetsPerLine(const uint nb_total_widgets)
{
  uint max_nb_widgets_per_line = nb_total_widgets;

  while (max_line_width - 2 * border.x <
	 max_nb_widgets_per_line * (widget_size.GetX()+margin) - margin) {
    max_nb_widgets_per_line--;
  }

  if (max_nb_widgets_per_line < 1) {
    max_nb_widgets_per_line = 1;
  }

  uint nb_lines = nb_total_widgets / max_nb_widgets_per_line;
  if (nb_total_widgets % max_nb_widgets_per_line != 0) {
    nb_lines++;
  }

  uint nb_widgets_per_line = nb_total_widgets / nb_lines;
  if (nb_total_widgets % nb_lines != 0) {
    nb_widgets_per_line++;
  }

  return nb_widgets_per_line;
}

void GridBox::AddWidget(Widget * a_widget)
{
  ASSERT(a_widget != NULL);

  WidgetList::AddWidget(a_widget);
  size.x = max_line_width;
  SetXY(position.x, position.y);
}

void GridBox::SetSizePosition(const Rectanglei &rect)
{
  position = rect.GetPosition();
  if (rect.GetSizeX() > 0)
    max_line_width = rect.GetSizeX();

  uint nb_widgets_per_line = NbWidgetsPerLine(widget_list.size());

  std::list<Widget *>::iterator it;
  uint line = 0, column = 0;
  for( it = widget_list.begin();
       it != widget_list.end();
       ++it ){

    last_line = line;
    PlaceWidget((*it), line, column);

    if (column + 1 == nb_widgets_per_line) {
      column = 0;
      line++;
    } else {
      column++;
    }

  }

  size.x = 2*border.x + nb_widgets_per_line * (widget_size.GetX() + margin) - margin;
  size.y = 2*border.y + (last_line+1) * (widget_size.GetY() + margin) - margin;
}
