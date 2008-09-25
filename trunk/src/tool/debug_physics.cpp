/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Box2D debugging class for Wormux
 *****************************************************************************/

#include "graphic/color.h"
#include "graphic/surface.h"
#include "graphic/video.h"
#include "map/camera.h"
#include "tool/debug_physics.h"
#include "tool/math_tools.h"

extern const double PIXEL_PER_METER;

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
  Color mycolor(color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

  std::list<Point2i> points;
  for (int32 i = 0; i < vertexCount; i++) {
    points.push_back(Point2i(lround(vertices[i].x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x,
                             lround(vertices[i].y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y));
  }

  GetMainWindow().AAPolygonColor(points, mycolor);
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
  Color mycolor(color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

  std::list<Point2i> points;
  for (int32 i = 0; i < vertexCount; i++) {
    points.push_back(Point2i(lround(vertices[i].x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x,
                             lround(vertices[i].y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y));
  }

  GetMainWindow().FilledPolygon(points, mycolor);
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
  Color mycolor(color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

  GetMainWindow().CircleColor(lround(center.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x,
                              lround(center.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y,
                              lround(radius*PIXEL_PER_METER),
                              mycolor);
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& /*axis*/, const b2Color& color)
{
  Color mycolor(color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

  GetMainWindow().FilledCircleColor(lround(center.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x,
                                    lround(center.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y,
                                    lround(radius*PIXEL_PER_METER),
                                    mycolor);
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
  Color mycolor(color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

  GetMainWindow().LineColor(lround(p1.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x,
                            lround(p2.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x,
                            lround(p1.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y,
                            lround(p2.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y,
                            mycolor);
}

void DebugDraw::DrawXForm(const b2XForm& /*xf*/)
{
  // TODO
}

void DrawPoint(const b2Vec2& p, float32 /*size*/, const b2Color& color)
{
  Color mycolor(color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

  GetMainWindow().LineColor(lround(p.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x -1,
                            lround(p.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x -1,
                            lround(p.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y +1,
                            lround(p.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y +1,
                            mycolor);
  GetMainWindow().LineColor(lround(p.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x +1,
                            lround(p.x*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x +1,
                            lround(p.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y -1,
                            lround(p.y*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y -1,
                            mycolor);
}
