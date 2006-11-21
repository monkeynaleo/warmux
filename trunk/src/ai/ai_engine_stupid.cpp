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
 * Artificial intelligence stupid engine
 *****************************************************************************/

#include "ai_engine_stupid.h"
#include "../include/action_handler.h"
#include "../character/body.h"
#include "../character/move.h"
#include "../interface/game_msg.h"
#include "../map/map.h"
#include "../network/randomsync.h"
#include "../team/macro.h"
#include "../team/teams_list.h"
#include "../tool/error.h"
#include "../tool/math_tools.h"

#include <iostream>

AIStupidEngine * AIStupidEngine::singleton = NULL;
  
AIStupidEngine::AIStupidEngine()
{
  std::cout << "o Artificial Stupid engine Initialization" << std::endl;
}

AIStupidEngine* AIStupidEngine::GetInstance()
{
  if (singleton == NULL)
    singleton = new AIStupidEngine();

  return singleton;
}

void AIStupidEngine::BeginTurn()
{
  m_last_char = &ActiveCharacter();
  m_nearest_enemy = NULL;

  m_begin_turn_time = Time::GetInstance()->ReadSec();
  m_last_shoot_time = 0;
  m_step = 0;
  m_is_walking = false;

  // find the nearest enemy
//   FOR_ALL_LIVING_ENEMIES(team, character) {
//     if (m_nearest_enemy == NULL 
// 	|| ( character->GetCenter().Distance( ActiveCharacter().GetCenter()) < 
// 	     m_nearest_enemy->GetCenter().Distance( ActiveCharacter().GetCenter()))
// 	)
//       m_nearest_enemy = &(*character);
//   }
//   assert(m_nearest_enemy != NULL);

  FOR_ALL_LIVING_ENEMIES(team, character) {
    if ( IsDirectlyShootable(*character) ) {
      m_nearest_enemy = &(*character);
      std::cout << "Try to shoot " << (*character).GetName() << std::endl;

      goto end_boucle;
    } else {
      std::cout << (*character).GetName() << "is not directly shootable" << std::endl;
    }
  }
  
  std::cout <<std::endl;
 end_boucle:
  ChooseDirection();

  ChooseWeapon();

  m_current_time = Time::GetInstance()->ReadSec();
}

void AIStupidEngine::ChooseDirection()
{
  if ( m_nearest_enemy ) {

    if ( ActiveCharacter().GetCenterX() < m_nearest_enemy->GetCenterX())
      ActiveCharacter().SetDirection(DIRECTION_RIGHT);
    else
      ActiveCharacter().SetDirection(DIRECTION_LEFT);

  } else {
    // we do not have found anybody to shoot
    ActiveCharacter().SetDirection( randomSync.GetBool()?DIRECTION_LEFT:DIRECTION_RIGHT );
  }

}

void AIStupidEngine::Walk()
{
  if (!m_is_walking) { 
    ActiveCharacter().InitMouvementDG(100);
    ActiveCharacter().body->StartWalk();
  }

  m_is_walking = true;

  if(ActiveCharacter().IsImmobile()) {
    if (ActiveCharacter().GetDirection() == DIRECTION_RIGHT)
      MoveCharacterRight(ActiveCharacter());
    else
      MoveCharacterLeft(ActiveCharacter());
  }

  if (Time::GetInstance()->ReadSec() > m_time_at_last_position +2) {

    if (m_last_position == ActiveCharacter().GetPosition()) {
      // we are probably blocked
      // try to jump
      StopWalk();
      m_step++;
      //ActionHandler::GetInstance()->NewAction (new Action(ACTION_HIGH_JUMP));
    }
 
    m_last_position = ActiveCharacter().GetPosition();
    m_time_at_last_position = Time::GetInstance()->ReadSec();
  }

}

void AIStupidEngine::StopWalk()
{
  m_is_walking = false;
  ActiveCharacter().body->StopWalk();
}

void AIStupidEngine::ChooseWeapon()
{

  if ( m_nearest_enemy ) {
    // we choose between gun, sniper_rifle, shotgun and submachine gun
    uint selected = uint(randomSync.GetDouble(0.0, 3.5));
    switch (selected) {
    case 0:
      ActiveTeam().SetWeapon(WEAPON_SHOTGUN);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
    case 1:
      ActiveTeam().SetWeapon(WEAPON_SNIPE_RIFLE);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
    case 2:
      //ActiveTeam().SetWeapon(WEAPON_SUBMACHINE_GUN);
      //if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
    case 3:
    default:
      ActiveTeam().SetWeapon(WEAPON_GUN);
    }
    ActiveTeam().crosshair.ChangeAngleVal(m_angle);
    std::cout << "2-Angle Radian: " << ActiveTeam().crosshair.GetAngleRad() << std::endl;
    std::cout << "2-Angle Degree: " << ActiveTeam().crosshair.GetAngle() << std::endl;
    
  } else {
    // we choose between dynamite, mine, polecart and gnu
    uint selected = uint(randomSync.GetDouble(0.0, 3.5));
    
    switch (selected) {
    case 0: 
      ActiveTeam().SetWeapon(WEAPON_DYNAMITE);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
      
    case 1:
      ActiveTeam().SetWeapon(WEAPON_GNU);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
      
    case 2:
      ActiveTeam().SetWeapon(WEAPON_POLECAT);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
      
    case 3:
    default:
      ActiveTeam().SetWeapon(WEAPON_MINE);
    }
  }

  // not enough ammo !!
  if ( ! ActiveTeam().GetWeapon().EnoughAmmo() ) {
    ActiveTeam().SetWeapon(WEAPON_SKIP_TURN);
  }

}

bool AIStupidEngine::IsDirectlyShootable(Character& character)
{
  Point2i pos = ActiveCharacter().GetCenter();
  Point2i arrival = character.GetCenter();
  Point2i departure = pos;
  Point2i delta_pos;
  
  double original_angle = pos.ComputeAngle(arrival);

  // compute to see if there any part of ground between the 2 characters
  // While test is not finished
  while (pos != arrival) {

    // is there a collision on the ground ??
    if ( !world.EstDansVide(pos.x, pos.y)) {
      return false;
    }

    // the point is outside the map
    if ( world.EstHorsMondeX(pos.x) || world.EstHorsMondeY(pos.y) ) {
      break;
    }

    // is there a collision with another character ?
    FOR_ALL_CHARACTERS(team, other_character) {
      if ( &(*other_character) != &ActiveCharacter() 
	   && &(*other_character) != &character ) {

	if ( other_character->GetTestRect().Contains(pos) )
	  return false;

      }
    }

    // next step
    int diff_x = pos.x - arrival.x;
    int diff_y = pos.y - arrival.y;

    delta_pos.x = 0;
    delta_pos.y = 0;

    if (abs(diff_x) > abs(diff_y)) {
      if (pos.x < arrival.x)
	delta_pos.x = 1;   //Increment x
      else 
	delta_pos.x = -1;
    } else {
      if (pos.y < arrival.y)
	delta_pos.y = 1;
      else 
	delta_pos.y = -1;
    }

    pos += delta_pos;
  }

  // Convert radian angle into degree
  m_angle = Rad2Deg(original_angle);

  // Set direction
  if (departure.x > arrival.x) {
    ActiveCharacter().SetDirection(DIRECTION_LEFT);
    m_angle = int(InverseAngleDeg(double(m_angle)));
  } else {
    ActiveCharacter().SetDirection(DIRECTION_RIGHT);
  }

  // Prepare game message
  std::string s = "Try to shoot "+character.GetName();
  char buff[3];
  sprintf(buff, "%d", m_angle); // to manage angle equals to 0
  s += " with angle ";
  s += buff;
  GameMessages::GetInstance()->Add(s); 

  return true;

}

void AIStupidEngine::Shoot()
{
  if (Time::GetInstance()->ReadSec() > m_last_shoot_time + 1 || 
      m_last_shoot_time == 0) {
    ActiveTeam().GetWeapon().NewActionShoot();
    m_last_shoot_time = Time::GetInstance()->ReadSec();
  }
  
  if (!(ActiveTeam().GetWeapon().EnoughAmmoUnit())) {
    m_step++;
  }
}

void AIStupidEngine::Refresh()
{
  // new character to control
  if (&ActiveCharacter() != m_last_char) 
    BeginTurn();

  // Get time
  uint local_time = Time::GetInstance()->ReadSec(); 
  if (local_time != m_current_time) {
    //printf("TIME: %2d - begin:%2d - last shoot:%2d - Step: %d\n", 
    //	   local_time, m_begin_turn_time, m_last_shoot_time, m_step);
    m_current_time = local_time;
  }

  // wait some seconds as if we are thinking...
  if (m_current_time < m_begin_turn_time + 3)
    return;

  switch (m_step) 
    {
    case 0:
      if (m_nearest_enemy) {
	// we already knows who to shoot
	m_step = 3;
      } else {
	// walk
	Walk();
     
	if (m_current_time > m_begin_turn_time + 5)
	  m_step++;
      }
      break;
    case 1:
      // Jump
      StopWalk();
      ActionHandler::GetInstance()->NewAction (new Action(ACTION_JUMP));
      m_step++;
      break;
    case 2:
      // used in the future
      m_step++;
      break;
    case 3:
      // shoot !!
      Shoot();
      break;
    case 4:
      // go go go !!
      ChooseDirection();
      m_step++;
      break;
    case 5:
      //ActionHandler::GetInstance()->NewAction (new Action(ACTION_HIGH_JUMP));
      m_step++;
      break;
    case 6:
      Walk();
      break;
    default:
      assert(false);
    }

}

