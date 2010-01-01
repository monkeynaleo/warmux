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

#ifndef _MOVE_INTENTION_H
#define _MOVE_INTENTION_H

#include <WORMUX_types.h>

class LRMoveIntention
{
  private:
    LRDirection direction;
    bool slowly;
  public:
    LRMoveIntention(LRDirection direction, bool slowly);
    LRDirection GetDirection() const;
    bool IsToDoItSlowly() const;
};
extern const LRMoveIntention * const INTENTION_MOVE_LEFT_SLOWLY;
extern const LRMoveIntention * const INTENTION_MOVE_LEFT;
extern const LRMoveIntention * const INTENTION_MOVE_RIGHT;
extern const LRMoveIntention * const INTENTION_MOVE_RIGHT_SLOWLY;


#endif
