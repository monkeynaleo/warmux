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
#include "../network/randomsync.h"
#include "../team/teams_list.h"

#include <iostream>

AIStupidEngine * AIStupidEngine::singleton = NULL;
  
AIStupidEngine::AIStupidEngine()
{
  std::cout << "* Artificial Stupid engine Initialisation" << std::endl;
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

  m_begin_turn_time = Time::GetInstance()->Read();
  m_step = 0;
  m_is_walking = false;

  ChooseDirection();
}

void AIStupidEngine::ChooseDirection()
{
  m_goes_right = randomSync.GetBool();

  ActiveCharacter().SetDirection(m_goes_right);
}

void AIStupidEngine::Walk()
{
  if (!m_is_walking) { 
    ActiveCharacter().InitMouvementDG(100);
    ActiveCharacter().body->StartWalk();
  }

  m_is_walking = true;

  if(ActiveCharacter().IsImmobile()) {
    if (m_goes_right)
      MoveCharacterRight(ActiveCharacter());
    else
      MoveCharacterLeft(ActiveCharacter());
  }
}

void AIStupidEngine::StopWalk()
{
  m_is_walking = false;
  ActiveCharacter().body->StopWalk();
}


void AIStupidEngine::Refresh()
{
  // new character to control
  if (&ActiveCharacter() != m_last_char) 
    BeginTurn();

  // wait some seconds as if we are thinking...
  if (Time::GetInstance()->Read() < m_begin_turn_time + 300)
    return;

  // walk !
  if ( m_step == 0 ) {
    Walk();

    if (Time::GetInstance()->Read() > m_begin_turn_time + 600)
      m_step++;

    return;
  }
  
  // jump !!
  if ( m_step == 1 ) {
    StopWalk();
    ActionHandler::GetInstance()->NewAction (new Action(ACTION_JUMP));
    m_step++;

    return;
  }
    
  // walk
  if ( m_step == 2 ) {
    Walk();

    if ( Time::GetInstance()->Read() > m_begin_turn_time + 1000 )
      m_step++;

    return;
  }

  // Shoot !!
  if ( m_step == 3 ) {
    StopWalk();
    ActiveTeam().GetWeapon().NewActionShoot();
    ActionHandler::GetInstance()->NewAction (new Action(ACTION_HIGH_JUMP));
    m_step++;

    return;
  }

  // go go go !!
  if ( m_step == 4 ) {
    ChooseDirection();
    m_step++;
    return;
  } 
 
  if ( m_step == 5 ) {
    ActionHandler::GetInstance()->NewAction (new Action(ACTION_HIGH_JUMP));
    m_step++;
    return;
  }

  if ( m_step == 6 ) {
    Walk();
  }

}
