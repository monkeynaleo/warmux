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
#include "../character/body.h"
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
}

void AIStupidEngine::Refresh()
{
  // new character to control
  if (&ActiveCharacter() != m_last_char)
    BeginTurn();

  /* thinks about movement */
  if ( m_step == 0 &&
       Time::GetInstance()->Read() > m_begin_turn_time + 200) {
    ActiveCharacter().SetDirection(randomSync.GetBool());
    m_step++;
  }
    
  if ( m_step == 1 &&
       Time::GetInstance()->Read() > m_begin_turn_time + 400) {
    ActiveCharacter().Jump();
    m_step++;
  }
    
  if ( m_step == 2 &&
       Time::GetInstance()->Read() < m_begin_turn_time + 600) {
    ActiveCharacter().InitMouvementDG(100);
    ActiveCharacter().body->StartWalk();
    m_step++;
  }

  /* thinks about shooting */
  if ( m_step == 3 &&
       Time::GetInstance()->Read() > m_begin_turn_time + 1000) {
    ActiveTeam().GetWeapon().NewActionShoot();
    m_step++;
  }
  
  if ( m_step == 4 ) {
    ActiveCharacter().SetDirection(randomSync.GetBool());
    m_step++;
  }

  if ( m_step == 5 ) {
    ActiveCharacter().InitMouvementDG(100);
    ActiveCharacter().body->StartWalk();
  }
}
