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
 * Artificial intelligence Shoot module
 *****************************************************************************/

#include "ai_shoot_module.h"
#include "../include/action_handler.h"
#include "../interface/game_msg.h"
#include "../map/map.h"
#include "../network/randomsync.h"
#include "../team/macro.h"
#include "../tool/error.h"
#include "../tool/math_tools.h"
#include "../tool/string_tools.h"

#include <iostream>
// =================================================
// Try to find an enemy which is shootable by 
// weapons like gun, shotgun, m16
// =================================================
bool AIShootModule::FindShootableEnemy()
{
  FOR_ALL_LIVING_ENEMIES(team, character) {
    if ( IsDirectlyShootable(*character) ) {
      m_enemy = &(*character);
      return true;
    }
  }

  return false;
}

// =================================================
// Return true if there is a straight line with no
// collision between the active character and a 
// potential enemy
// =================================================
// This method is not perfect
// It tests from the Center of the current Character controlled by the AI
// and not from the gun hole
// =================================================
bool AIShootModule::IsDirectlyShootable(Character& character)
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
    ActiveCharacter().SetDirection(Body::DIRECTION_LEFT);
    m_angle = int(InverseAngleDeg(double(m_angle)));
  } else {
    ActiveCharacter().SetDirection(Body::DIRECTION_RIGHT);
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

// =================================================
// Try to find an enemy which is shootable by 
// weapons like dynamite, mine, ...
// =================================================
bool AIShootModule::FindProximityEnemy()
{
  FOR_ALL_LIVING_ENEMIES(team, character) {    
    if ( IsNear(*character) ) {
      m_enemy = &(*character);
      return true;
    }
  }
  
  return false;
  //     if (m_nearest_enemy == NULL
  // 	|| ( character->GetCenter().Distance( ActiveCharacter().GetCenter()) <
  // 	     m_nearest_enemy->GetCenter().Distance( ActiveCharacter().GetCenter()))
  // 	)
  //       m_nearest_enemy = &(*character);
  //   }
  //   assert(m_nearest_enemy != NULL);
}

// =================================================
// Return true if character seems to be accessible 
// =================================================
// This method is not perfect!!
// =================================================
bool AIShootModule::IsNear(Character& character)
{
  uint delta_x = abs(character.GetX() - ActiveCharacter().GetX());
  uint delta_y = abs(character.GetY() - ActiveCharacter().GetY());

  if (delta_x > 300)
    return false;
  
  if (delta_y > 100)
    return false;
  
  return true;
}

// =================================================
// Shoot!
// =================================================
void AIShootModule::Shoot()
{
  if (m_current_time > m_last_shoot_time + 1 ||
      m_last_shoot_time == 0) {
    ActiveTeam().GetWeapon().NewActionShoot();
    m_last_shoot_time = m_current_time;
  }

  if (!(ActiveTeam().GetWeapon().EnoughAmmoUnit())) {
    m_has_finished = true;
    ActiveCharacter().body->StartWalk();
  }
}

Character* AIShootModule::FindEnemy()
{
  if (m_has_finished) {
    return NULL;
  }

  if (m_enemy != NULL && !(m_enemy->IsDead())) {
    return m_enemy;
  }

  m_current_strategy = NO_STRATEGY;

  if (FindProximityEnemy()) {
    GameMessages::GetInstance()->Add(ActiveCharacter().GetName()+" has decided to injured "
				     + m_enemy->GetName());

    m_current_strategy = NEAR_FROM_ENEMY;

    // we choose between dynamite, mine, polecart and gnu
    uint selected = uint(randomSync.GetDouble(0.0, 3.5));

    switch (selected) {
    case 0:
    case 1:
      ActiveTeam().SetWeapon(Weapon::WEAPON_DYNAMITE);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;

//     case 1:
//       ActiveTeam().SetWeapon(Weapon::WEAPON_GNU);
//       if (ActiveTeam().GetWeapon().EnoughAmmo()) break;

//     case 2:
//       ActiveTeam().SetWeapon(Weapon::WEAPON_POLECAT);
//       if (ActiveTeam().GetWeapon().EnoughAmmo()) break;

    case 3:
    default:
      ActiveTeam().SetWeapon(Weapon::WEAPON_MINE);
    }
    m_angle = 0;
    ActiveTeam().crosshair.ChangeAngleVal(m_angle);
  }
  else if (FindShootableEnemy()) {
    
    m_current_strategy = SHOOT_FROM_POINT;
    GameMessages::GetInstance()->Add(ActiveCharacter().GetName()+" will shoot "
				     + m_enemy->GetName());

    // we choose between gun, sniper_rifle, shotgun and submachine gun
    uint selected = uint(randomSync.GetDouble(0.0, 3.5));
    switch (selected) {
    case 0:
      ActiveTeam().SetWeapon(Weapon::WEAPON_SHOTGUN);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
    case 1:
      ActiveTeam().SetWeapon(Weapon::WEAPON_SNIPE_RIFLE);
      if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
    case 2:
      //ActiveTeam().SetWeapon(WEAPON_SUBMACHINE_GUN);
      //if (ActiveTeam().GetWeapon().EnoughAmmo()) break;
    case 3:
    default:
      ActiveTeam().SetWeapon(Weapon::WEAPON_GUN);
    }

    int angle = BorneLong(m_angle, - (ActiveTeam().GetWeapon().max_angle),
			  - (ActiveTeam().GetWeapon().min_angle) );
    
    if (abs(angle-m_angle) < 5) {
      ActiveTeam().crosshair.ChangeAngleVal(m_angle);
    } else {
      GameMessages::GetInstance()->Add("Angle is too wide!");
				     
      m_current_strategy = NO_STRATEGY;
      m_angle = 0;
      m_enemy = NULL;

      return m_enemy;
    }
  }

  // not enough ammo !!
  if ( ! ActiveTeam().GetWeapon().EnoughAmmo() ) {
    ActiveTeam().SetWeapon(Weapon::WEAPON_SKIP_TURN);
  }

  ChooseDirection();

  return m_enemy;
}

void AIShootModule::ChooseDirection()
{
  if ( m_enemy ) {

    if ( ActiveCharacter().GetCenterX() < m_enemy->GetCenterX())
      ActiveCharacter().SetDirection(Body::DIRECTION_RIGHT);
    else
      ActiveCharacter().SetDirection(Body::DIRECTION_LEFT);

  }
}

bool AIShootModule::Refresh(uint current_time)
{
  if (m_has_finished) {
    return true;
  }

  m_current_time = current_time;

  FindEnemy();

  switch (m_current_strategy) {

  case NO_STRATEGY:
    //ActiveTeam().SetWeapon(Weapon::WEAPON_SKIP_TURN);
    //Shoot();
    break;

  case NEAR_FROM_ENEMY:
    // We are near enough of an enemy (perhaps not the first one we have choosen)
    FOR_ALL_LIVING_ENEMIES(team, character) {    
      if ( abs((*character).GetX() - ActiveCharacter().GetX()) <= 10 &&
	   abs ((*character).GetY() - ActiveCharacter().GetY()) < 60 ) {
      //if ( (*character).GetCenter().Distance( ActiveCharacter().GetCenter()) < 50) { 
	if (&(*character) != m_enemy) {
	  GameMessages::GetInstance()->Add(ActiveCharacter().GetName()+" changes target : "
					   + (*character).GetName());
	}
 	m_enemy = &(*character);
 	Shoot();
      }
    }
    break;
  
  case SHOOT_FROM_POINT:
    Shoot();
    return false;
    break;
  }

  return true;
}

// =================================================
// Initialize Shoot module when changing 
// character to control
// =================================================
void AIShootModule::BeginTurn()
{
  m_enemy = NULL;
  m_last_shoot_time = 0;
  m_angle = 0;
  m_current_strategy = NO_STRATEGY;
  m_has_finished = false;

  // Choose random direction for the moment
  ActiveCharacter().SetDirection( randomSync.GetBool()?Body::DIRECTION_LEFT:Body::DIRECTION_RIGHT );
}

AIShootModule::AIShootModule()
{
  std::cout << "o Artificial Intelligence Shoot module initialization" << std::endl;
}
