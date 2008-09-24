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

#ifndef DEBUG_PHYSICS_H
#define DEBUG_PHYSICS_H

#include "Box2D.h"

// This class implements debug drawing callbacks that are invoked
// inside b2World::Step.
class DebugDraw : public b2DebugDraw
{
public:
  void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

  void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

  void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);

  void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);

  void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);

  void DrawXForm(const b2XForm& xf);
};

#endif

