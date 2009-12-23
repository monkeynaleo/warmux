/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * Describes which left or right movement the user wants to do with a character.
 *****************************************************************************/

#include "character/walk_intention.h"
#include <WORMUX_error.h>

WalkIntention::WalkIntention():
  left_slowly(false),
  left_normal(false),
  right_slowly(false),
  right_normal(false),
  exists(false)
{
  // nothing else to do:
  // Values of direction and slowly are only needed if exists is true.
}

bool WalkIntention::IsToWalk() const
{
  return exists;
}

LRDirection WalkIntention::GetDirection() const
{
  ASSERT(IsToWalk());
  return direction;
}

bool WalkIntention::IsToDoItSlowly() const
{
  ASSERT(IsToWalk());
  return slowly;
}

void WalkIntention::Set(LRDirection direction, bool slowly, bool enabled)
{
  if (direction == DIRECTION_LEFT) {
    if (slowly)
      left_slowly = enabled;
    else
      left_normal = enabled;
  } else {
    if (slowly)
      right_slowly = enabled;
    else
      right_normal = enabled;
  }
  UpdateChachedValues();
}


void WalkIntention::SetAllFalse()
{
  left_slowly = false;
  left_normal = false;
  right_slowly = false;
  right_normal = false;
  UpdateChachedValues();
}

void WalkIntention::UpdateChachedValues()
{
  bool left = (left_normal || left_slowly);
  bool right = (right_normal || right_slowly);
  if (left && !right) {
    direction = DIRECTION_LEFT;
    exists = true;
    slowly = left_slowly;
  } else if (!left && right) {
    direction = DIRECTION_RIGHT;
    exists = true;
    slowly = right_slowly;
  } else {
    exists = false;
  }
}

bool WalkIntention::Get(LRDirection direction, bool slowly) const
{
  if (direction == DIRECTION_LEFT) {
    if (slowly)
      return left_slowly;
    else
      return left_normal;
  } else {
    if (slowly)
      return right_slowly;
    else
      return right_normal;
  }
}
