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

#include <WORMUX_debug.h>
#include "graphic/font.h"
#include "graphic/video.h"
#include "gui/graph_canvas.h"

#define DEF_BORDER      8

GraphCanvas::GraphCanvas(const Point2i& size,
                         const std::string& xname, const std::string& yname,
                         std::vector<Result>& res, uint thick)
  : Widget(size)
  , results(res)
  , thickness(thick)
{
  Font* font = Font::GetInstance(Font::FONT_MEDIUM, Font::FONT_BOLD);
  xaxis = font->CreateSurface(xname, black_color);
  yaxis = font->CreateSurface(yname, black_color).RotoZoom(M_PI/2, 1.0, 1.0, false);
}

void GraphCanvas::Draw(const Point2i& /*mousePosition*/) const
{
  DrawGraph(position.x+DEF_BORDER, position.y+DEF_BORDER,
            size.x-2*DEF_BORDER, size.y-2*DEF_BORDER);
}

void GraphCanvas::DrawGraph(uint i, float xmax,
                            int x, float xscale,
                            int y, float yscale) const
{
  const Result &res = results[i];
  const Color& color = res.color;

  if (!res.list.size()) {
    MSG_DEBUG("menu", "   No point !?!");
    return;
  }

  int sx = x+int(res.list[0].first*xscale)+thickness,
      sy = y-int(res.list[0].second*yscale);
  Surface &surface = GetMainWindow();
  MSG_DEBUG("menu", "   First point: (%.3f,%.3f) -> (%i,%i)",
            res.list[0].first, res.list[0].second, sx, sy);

  for (uint i=0; i<res.list.size(); i++) {
    const Value& val = res.list[i];
    int ex = x+int(val.first*xscale),
        ey = y-int(val.second*yscale);

    MSG_DEBUG("menu", "   Next point: (%u,%u) -> (%i,%i)",
              val.first, val.second, ex, ey);
    surface.BoxColor(Rectanglei(sx, sy, ex-sx, thickness), color);
    surface.BoxColor(Rectanglei(ex, std::min(sy,ey), thickness, abs(ey-sy)), color);

    sx = ex;
    sy = ey;
  }

  // Missing point
  if (res.list[res.list.size()-1].first < xmax) {
    int ex = x+int(xmax*xscale);
    MSG_DEBUG("menu", "   Last point -> (%i,%i)", ex, sy);
    surface.BoxColor(Rectanglei(sx, sy, ex-sx, thickness), color);
  }
}

void GraphCanvas::DrawGraph(int x, int y, int w, int h) const
{
  // Value to determine normalization
  float  max_value = 0;
  float  xmax      = 0;
  uint   graph_h   = h-32;
  uint   graph_w   = w-32;
  uint   graph_x   = x+32;

  for (uint i=0; i<results.size(); i++) {
    if (results[i].ymax > max_value)
      max_value = results[i].ymax;
    if (results[i].xmax > xmax)
      xmax = results[i].xmax;
  }
  // needed to see correctly energy at the end if two teams have same
  // energy just before the final blow
  xmax += xmax/50.0f;

  // Draw here the graph and stuff
  Surface &surface = GetMainWindow();
  surface.BoxColor(Rectanglei(graph_x, y, thickness, graph_h), black_color);
  surface.BoxColor(Rectanglei(graph_x, y+graph_h, graph_w, thickness), black_color);
  surface.Blit(xaxis, Point2i(graph_x+graph_w/2, y+graph_h+8));
  surface.Blit(yaxis, Point2i(x+4, graph_h/2));
  char buffer[16];
  snprintf(buffer, 16, "%.1f", xmax/1000.0f);
  surface.Blit(Font::GetInstance(Font::FONT_MEDIUM, Font::FONT_BOLD)->CreateSurface(buffer, black_color),
               Point2i(x+graph_w-20, y+graph_h+8));

  // Draw each team graph
  float yscale = graph_h / (1.05f*max_value);
  float xscale = graph_w / (1.05f*xmax);

  for (uint i=0; i<results.size(); i++) {
    if (results[i].item) {
      // Legend line
      surface.BoxColor(Rectanglei(x+w-112, y+12+i*40, 56, thickness), results[i].color);
      // Legend icon
      surface.Blit(*results[i].item, Point2i(x+w-48, y+12+i*40-20));
    }
    DrawGraph(i, xmax, graph_x, xscale, y+graph_h, yscale);
  }
}
