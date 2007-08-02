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
 * Artificial intelligence Movement module
 *****************************************************************************/

#include <iostream>
#include "include/action_handler.h"
#include "character/character.h"
#include "character/move.h"
#include "map/map.h"
#include "team/teams_list.h"
#include "tool/debug.h"
#include "tool/error.h"

#include "ai_movement_module.h"

// TODO:
// - Be sure to not go out of the map
// - Fix hole detection
// - Do not go back on the shoot position !


// =================================================
// Go on !
// =================================================
void AIMovementModule::MakeStep() const
{
  if(ActiveCharacter().IsImmobile()) {
    if (ActiveCharacter().GetDirection() == Body::DIRECTION_RIGHT) {
      MoveActiveCharacterRight();
    }  else {
      MoveActiveCharacterLeft();
    }
  }
}

// =================================================
// Compute obstacle height (barrier or hole)
// Return true if sure that it does not need to jump
// or to use parachute
// =================================================
bool AIMovementModule::ObstacleHeight(int& height) const
{
  if (ComputeHeightMovement(ActiveCharacter(), height, false))
    return true;

  int y_floor=ActiveCharacter().GetY();
  if (height < 0) {

    for (height = -15; height >= -150 ; height--) {
      if ( ActiveCharacter().IsInVacuum( Point2i(ActiveCharacter().GetDirection(), height) ) ) {
        break;
      }
    }

  } else {

    // Compute exact whole size
    for (height = 15; height <= 150 ; height++){
      if ( !ActiveCharacter().IsInVacuum(Point2i(ActiveCharacter().GetDirection(), height))
          ||  ActiveCharacter().FootsOnFloor(y_floor+height)){
        break;
      }
    }

  }
  return false;
}

bool AIMovementModule::RiskGoingOutOfMap() const
{
  if ( ActiveCharacter().GetDirection() == Body::DIRECTION_LEFT &&
       ActiveCharacter().GetX() <= 5 ) {
    return true;
  } else if ( ActiveCharacter().GetDirection() == Body::DIRECTION_RIGHT &&
              world.GetWidth() - 5 <= ActiveCharacter().GetX() + ActiveCharacter().GetSize().GetX() ) {
    return true;
  }

  return false;
}

// =================================================
// A jump is made of many movement :
// 1) Decect collision and prepare to go back
// 2) Go back, then when far enought of the obstacle
// 3) Jump!
// 4) Detect when on the ground!
// =================================================

void AIMovementModule::PrepareJump()
{
  MSG_DEBUG("ai.move", "Go back before jumping");

  ActiveCharacter().body->StartWalk();

  current_movement = BACK_TO_JUMP;
  time_at_last_position = m_current_time;

  InverseDirection(false);
}

void AIMovementModule::GoBackToJump()
{
  ASSERT(current_movement == BACK_TO_JUMP);

  MakeStep();

  int height;
  bool blocked = !(ObstacleHeight(height));

  if ( abs(last_position.GetX() - ActiveCharacter().GetPosition().GetX()) >= 20
       || time_at_last_position +1 < m_current_time
       || blocked) {
    //it's time to jump!
    InverseDirection(false);
    Jump();
  }
}

void AIMovementModule::Jump()
{
  MSG_DEBUG("ai.move", "Jump!");

  //  GameMessages::GetInstance()->Add("try to jump!");
  current_movement = JUMPING;
  ActionHandler::GetInstance()->NewActionActiveCharacter(new Action(Action::ACTION_CHARACTER_HIGH_JUMP));
}

void AIMovementModule::EndOfJump()
{
  ASSERT(current_movement == JUMPING);
  MSG_DEBUG("ai.move", "End of Jump!");
  //GameMessages::GetInstance()->Add("finished to jump");

  if ( last_position.GetX() == ActiveCharacter().GetPosition().GetX() ) {
    // we have not moved since last movement

    if (ActiveCharacter().GetDirection() == Body::DIRECTION_RIGHT) {
      max_reachable_x = ActiveCharacter().GetPosition().GetX();
    } else {
      min_reachable_x = ActiveCharacter().GetPosition().GetX();
    }
    MSG_DEBUG("ai.move", "We are blocked");
    StopMoving();

  } else {
    // No more blocked !!
    MSG_DEBUG("ai.move", "We are NO MORE blocked");
    current_movement = WALKING;
  }
}


// =================================================
// The walking methods
// Walk() is responsible to detect obstacles
// =================================================

void AIMovementModule::Walk()
{
  // Animate skin
  if ( current_movement != WALKING ) {
    ActiveCharacter().BeginMovementRL(100);
    ActiveCharacter().body->StartWalk();
    current_movement = WALKING;
  }

  MakeStep();

  int height;
  bool blocked = !(ObstacleHeight(height));

  // we are blocked, what next ?
  if ( blocked ) {

    if ( last_blocked_position.GetX() != ActiveCharacter().GetPosition().GetX() ) {

      last_blocked_position = ActiveCharacter().GetPosition();

      if (height < 0 ) {
        // There's a barrier

        if (height >= -80) { // we can try to jump!
          PrepareJump();
          return; // do not update position
        } else { // it's too high!
          // GameMessages::GetInstance()->Add("It's too high!!");
          MSG_DEBUG("ai.move", "It's too high, we have to go back");
          InverseDirection(true);
        }
      } else {
        // There's a hole

        if (height >= 100) { // it's too deep, go back!!
          MSG_DEBUG("ai.move", "It's too deep, we have to go back");
          // GameMessages::GetInstance()->Add("It's too deep!" + ulong2str(height));
          InverseDirection(true);
        }
      }
    } else {
      // already have been blocked here...
      MSG_DEBUG("ai.move", "already have been blocked here...");
      //InverseDirection(true);
      StopMoving();
    }
  }

  // Inverse direction if there is a risk to go out of the map
  if (RiskGoingOutOfMap()) {
    MSG_DEBUG("ai.move", "RiskGoingOutOfMap : go back");
    InverseDirection(true);
  }

  // Update position if we are not jumping
  last_position = ActiveCharacter().GetPosition();
  time_at_last_position = m_current_time;
}

void AIMovementModule::StopWalking()
{
  MSG_DEBUG("ai.move", "Stop to walk");
  current_movement = NO_MOVEMENT;
  ActiveCharacter().body->StopWalk();
}


// =================================================
// Invert walking direction
// =================================================
void AIMovementModule::InverseDirection(bool completely_blocked)
{
  MSG_DEBUG("ai.move", "Inverse direction");

  if (ActiveCharacter().GetDirection() == Body::DIRECTION_RIGHT) {

    ActiveCharacter().SetDirection(Body::DIRECTION_LEFT);
    if (completely_blocked)
      max_reachable_x = ActiveCharacter().GetPosition().GetX();

  } else {

    ActiveCharacter().SetDirection(Body::DIRECTION_RIGHT);
    if (completely_blocked)
      min_reachable_x = ActiveCharacter().GetPosition().GetX();

  }
}

// =================================================
// Public method updating the movement
// =================================================
void AIMovementModule::Move(uint current_time)
{
  m_current_time = current_time;

  // are we on the ground ?
  if ( ActiveCharacter().FootsInVacuum() ) { // No!
    return;
  }

  switch (current_movement) {

  case NO_MOVEMENT:
    // Begin to walk
    Walk();
    break;

  case WALKING:
    // Continue to walk
    Walk();
    break;

  case BACK_TO_JUMP:
    // Go back to have enough place to jump
    GoBackToJump();
    break;

  case JUMPING:
    EndOfJump();

    break;
  default:
    break;
  }
}

void AIMovementModule::StopMoving()
{
  //  GameMessages::GetInstance()->Add("stop moving");
  StopWalking();
  //m_step++;
}

// =================================================
// Initialize Movement module when changing
// character to control
// =================================================
void AIMovementModule::BeginTurn()
{
  current_movement = NO_MOVEMENT;
  time_at_last_position = 0;
  last_position = Point2i(0,0);
  last_blocked_position = Point2i(0,0);

  min_reachable_x = 0;
  max_reachable_x = world.GetWidth();
}

AIMovementModule::AIMovementModule() :
  m_current_time(0),
  points_to_avoid(),
  min_reachable_x(0),
  max_reachable_x(0),
  destination_point(Point2i(-1,-1)),
  current_movement(NO_MOVEMENT),
  last_position(Point2i(-1,-1)),
  time_at_last_position(0),
  last_blocked_position(Point2i(-1,-1))

{
  std::cout << "o Artificial Intelligence Movement module initialization" << std::endl;
}

// ====================== Points to avoid
// private:
//  std::set<Point2i> points_to_avoid;

void AIMovementModule::UpdateListOfPointsToAvoid()
{
  // TODO : Refresh position of mines
}

void AIMovementModule::AddPointToAvoid(const Point2i& dangerous_point)
{
  points_to_avoid.insert(dangerous_point);
}

// ======================================

// ==================== Destination point
// private:
//  uint min_reachable_x, max_reachable_x;
//  Point2i destination_point;

void AIMovementModule::SetDestinationPoint(const Point2i& _destination_point)
{
  destination_point = _destination_point;
}

// =================================================
// Return true if character seems to be accessible
// =================================================
// This method is not perfect!!
// =================================================
bool AIMovementModule::SeemsToBeReachable(const Character& shooter,
                                          const Character& enemy) const
{
  int delta_x = abs(shooter.GetX() - enemy.GetX());
  int delta_y = abs(shooter.GetY() - enemy.GetY());

  if (delta_x > 300)
    return false;

  if (delta_y > 100)
    return false;

  if (min_reachable_x>enemy.GetX() || enemy.GetX()>max_reachable_x)
    return false;
  return true;
}

bool AIMovementModule::IsProgressing() const
{
  if (destination_point.GetX()>max_reachable_x ||
          destination_point.GetX()<min_reachable_x)
    return false;
  return true;
}


bool AIMovementModule::IsArrived() const
{
  return (ActiveCharacter().GetPosition() == destination_point);
}

// ======================================
