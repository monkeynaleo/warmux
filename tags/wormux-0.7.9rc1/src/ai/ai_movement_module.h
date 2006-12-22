/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
 * Artificial intelligence Movement module
 *****************************************************************************/
#ifndef AI_MOVEMENT_MODULE
#define AI_MOVEMENT_MODULE

#include "../tool/point.h"

class AIMovementModule
{
 private:
  typedef enum {
    NO_MOVEMENT,
    WALKING,
    BACK_TO_JUMP,
    JUMPING,
    FLYING,
    ROPING,
  } movement_type_t;

  uint m_current_time;

  Point2i last_position;
  uint time_at_last_position; 
  Point2i last_blocked_position;
  movement_type_t current_movement;

  void InverseDirection();

  void MakeStep();

  void Walk();
  void StopWalking();

  void PrepareJump();
  void GoBackToJump();
  void Jump();
  void EndOfJump();

  bool ObstacleHeight(int& height);
  bool RiskGoingOutOfMap();

 public:
  AIMovementModule();
  void BeginTurn();

  void Move(uint current_time);
  void StopMoving();

};

#endif
