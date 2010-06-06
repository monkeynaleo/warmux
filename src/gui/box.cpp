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
 * Vertical or Horizontal Box
 *****************************************************************************/

#include "gui/box.h"
#include "graphic/surface.h"
#include "graphic/colors.h"
#include "graphic/video.h"

Box::Box(void):
  margin(0),
  border()
{
}

Box::Box(const Point2i & size, 
         bool _draw_border) : 
  WidgetList(size),
  margin(5)
{
  border.SetValues(5, 5);

  if (_draw_border) {
    Widget::SetBorder(defaultOptionColorRect, 2);
    Widget::SetBackgroundColor(defaultOptionColorBox);
  }
}

Box::Box(Profile * profile,
         const xmlNode * boxNode) :
  WidgetList(profile, boxNode),
  margin(5)
{
}

Box::~Box()
{
}

void Box::ParseXMLBoxParameters()
{
  ParseXMLBorder();
  ParseXMLBackground();
}

void Box::Update(const Point2i &mousePosition,
		 const Point2i &/*lastMousePosition*/)
{
  if (need_redrawing) {
    Widget::RedrawBackground(*this);
    Draw(mousePosition);
  }

  WidgetList::Update(mousePosition);
  need_redrawing = false;
}

// --------------------------------------------------

VBox::VBox(uint width, bool _draw_border, bool _force_widget_size) :
  Box(Point2i(width, 100), _draw_border),
  force_widget_size(_force_widget_size)
{
}

void VBox::Pack()
{
  uint _y = position.y;
  uint max_size_x = 0;

  std::list<Widget *>::iterator it;
  for (it = widget_list.begin();
       it != widget_list.end();
       ++it) {

    if (it == widget_list.begin())
      _y += border.y - margin;

    (*it)->SetPosition(position.x + border.x,
		       _y + margin);

    if (force_widget_size) {
      (*it)->SetSize(size.x - 2*border.x,
		     (*it)->GetSizeY());
    } else {
      max_size_x = std::max(max_size_x, uint((*it)->GetSizeX()));
    }

    (*it)->Pack();

    _y = (*it)->GetPositionY() + (*it)->GetSizeY();
  }

  size.y = _y - position.y + border.y;

  if (!force_widget_size) {
    size.x = max_size_x + 2*border.x;
  }
}

// --------------------------------------------------

HBox::HBox(uint height, bool _draw_border, bool _force_widget_size) :
  Box(Point2i(100, height), _draw_border),
  force_widget_size(_force_widget_size)
{
}

void HBox::Pack()
{
  uint _x = position.x;
  uint max_size_y = 0;

  std::list<Widget *>::iterator it;
  for (it = widget_list.begin();
       it != widget_list.end();
       ++it) {

    if (it == widget_list.begin())
      _x += border.x - margin;

    (*it)->SetPosition(_x + margin,
		       position.y + border.y);

    if (force_widget_size) {
      (*it)->SetSize((*it)->GetSizeX(),
		     size.y - 2*border.y);
    } else {
      max_size_y = std::max(max_size_y, uint((*it)->GetSizeY()));
    }

    (*it)->Pack();

    _x = (*it)->GetPositionX()+ (*it)->GetSizeX();
  }
  size.x = _x - position.x + border.x;

  if (!force_widget_size) {
    size.y = max_size_y + 2*border.y;
  }
}

// --------------------------------------------------

GridBox::GridBox(uint _lines,
                 uint _columns, 
                 uint margin, 
                 bool _draw_border) :
  Box(Point2i(-1, -1), _draw_border),
  autoResize(true),
  fixedMargin(margin),
  lines(_lines),
  columns(_columns),
  grid(NULL)
{
  InitGrid();
}

GridBox::GridBox(Profile * profile,
                 const xmlNode * gridBoxNode) :
  Box(profile, gridBoxNode),
  autoResize(false),
  fixedMargin(0),
  lines(10),
  columns(10),
  grid(NULL)
{
}

GridBox::~GridBox(void)
{
  for (uint i = 0; i < lines; ++i) {
    delete [] grid[i];
  }
  delete [] grid;
}

bool GridBox::LoadXMLConfiguration(void)
{
  if (NULL == profile || NULL == widgetNode) {
    return false;
  }

  ParseXMLPosition();
  ParseXMLSize();
  ParseXMLBoxParameters();

  XmlReader * xmlFile = profile->GetXMLDocument();

  xmlFile->ReadUintAttr(widgetNode, "lines", lines);
  xmlFile->ReadUintAttr(widgetNode, "columns", columns);
  xmlFile->ReadUintAttr(widgetNode, "margin", fixedMargin);
 
  InitGrid();

  return true;
}

void GridBox::InitGrid(void) 
{
  grid = new Widget **[lines];

  for (uint i = 0; i < lines; ++i) {
    grid[i] = new Widget*[columns];
    for (uint j = 0; j < columns; ++j) {
      grid[i][j] = NULL;
    }
  }
}

void GridBox::PlaceWidget(Widget * widget, 
                          uint line, 
                          uint column)
{
  if (line >= lines || column >= columns) {
    return;
  }
  grid[line][column] = widget;
}

void GridBox::AddWidget(Widget * widget)
{
  ASSERT(widget != NULL);

  for (uint i = 0; i < lines; ++i) {
    for (uint j = 0; j < columns; ++j) {
      if (NULL == grid[i][j]) {
        grid[i][j] = widget;
	WidgetList::AddWidget(widget);
        return;
      }
    }
  }
}

void GridBox::AddWidget(Widget * widget, 
                        uint line, 
                        uint column)
{
  ASSERT(widget != NULL);
  grid[line][column] = widget;
}

int GridBox::GetMaxHeightByLine(uint line)
{
  int height = 0;
  Widget * widget = NULL;

  for (uint i = 0; i < columns; ++i) {
    widget = grid[line][i];
    if (NULL == widget) {
      continue;
    }
    widget->Pack();
    if (widget->GetSizeY() > height) {
      height = widget->GetSizeY();
    }
  }
  return height + fixedMargin;
}

int GridBox::GetMaxWidthByColumn(uint column)
{
  int width = 0;
  Widget * widget = NULL;

  for (uint i = 0; i < lines; ++i) {
    widget = grid[i][column];
    if (NULL == widget) {
      continue;
    }
    widget->Pack();
    if (widget->GetSizeX() > width) {
      width = widget->GetSizeX();
    }
  }
  return width + fixedMargin;
}

void GridBox::Pack()
{
  if (0 == widget_list.size()) {
    return;
  }
  int heightMax;
  int widthMax;
  uint x       = position.x;
  uint y       = position.y;
  uint marginX = 0;
  uint marginY = 0;

  int totalWidth = 0;
  for (uint columnIt = 0; columnIt < columns; ++columnIt) { 
    totalWidth += GetMaxWidthByColumn(columnIt);
  }
  if (totalWidth < GetSize().x) {
    marginX = (GetSize().x - totalWidth) / (columns + 1);
  }
  
  int totalHeight = 0;
  for (uint lineIt = 0; lineIt < lines; ++lineIt) {
    totalHeight += GetMaxHeightByLine(lineIt);
  }
  if (totalHeight < GetSize().y) {
    marginY = (GetSize().y - totalHeight) / (lines + 1);
  }

  uint gridWidth  = 0;
  uint gridHeight = 0;

  for (uint lineIt = 0; lineIt < lines; ++lineIt) {
    heightMax = GetMaxHeightByLine(lineIt);

    gridHeight += heightMax;
    gridWidth = 0;

    for (uint columnIt = 0; columnIt < columns; ++columnIt) {
      widthMax = GetMaxWidthByColumn(columnIt);
      gridWidth += widthMax;

      if (NULL != grid[lineIt][columnIt]) {
        Widget * widget = grid[lineIt][columnIt];
        widget->Pack();

        widget->SetPosition(x + marginX + ((widthMax + marginX) * columnIt) + ((widthMax - widget->GetSizeX()) / 2), 
                            y + marginY + ((heightMax + marginY) * lineIt) + ((heightMax - widget->GetSizeY()) / 2));
        widget->Pack();
      }
    }
  }
  if (true == autoResize) {
    SetSize(gridWidth, gridHeight);
  }
}

