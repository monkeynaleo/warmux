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

#ifndef _WALK_INTENTION_H
#define _WALK_INTENTION_H

#include <WORMUX_types.h>

class WalkIntention
{
  private:
    bool left_slowly;
    bool left_normal;
    bool right_slowly;
    bool right_normal;

    // cached values for simpler getters:
    LRDirection direction;
    bool exists;
    bool slowly;

    void UpdateChachedValues();
  public:
    WalkIntention();
    bool IsToWalk() const;
    LRDirection GetDirection() const;
    bool IsToDoItSlowly() const;
    void Set(LRDirection direction, bool slowly, bool enabled);
    void SetAllFalse();
    bool Get(LRDirection direction, bool slowly) const;
};

#endif
