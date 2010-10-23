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
 * Draw a graph of a series of points according to various parameters 
 *****************************************************************************/

#ifndef GRAPH_CANVAS_H
#define GRAPH_CANVAS_H

#include <vector>
#include "gui/widget.h"

class GraphCanvas : public Widget
{
public:
  typedef std::pair<float, float> Value;
  typedef struct
  {
    std::vector<Value> list;
    const Surface      *item;
    Color              color;
    float              max_value;
  } Result;
private:
  Surface              xaxis, yaxis;
  std::vector<Result>  results;
  uint                 thickness;

public:
  GraphCanvas(const Point2i& size,
              const std::string& xname, const std::string& yname,
              std::vector<Result>& res, uint thick=2);
  virtual ~GraphCanvas() {};
  virtual void Draw(const Point2i&) const;

  virtual void DrawGraph(uint i,
                         int x, int y,
                         float xscale,
                         float yscale,
                         float xmax) const;
  virtual void DrawGraph(int x, int y, int w, int h) const;

  virtual void Pack() {};
};


#endif // GRAPH_CANVAS_H
