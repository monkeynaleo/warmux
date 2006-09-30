/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 * AirHammer - Use it to dig
 *****************************************************************************/

#include "../weapon/airhammer.h"
//-----------------------------------------------------------------------------
#include <sstream>
#include "../game/game.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../include/action_handler.h"
#include "../map/map.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../interface/game_msg.h"
#include "../weapon/explosion.h"

//-----------------------------------------------------------------------------

const uint MIN_TIME_BETWEEN_JOLT = 100; // in milliseconds

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Airhammer::Airhammer() : Weapon(WEAPON_AIR_HAMMER,"airhammer",new WeaponConfig())
{
  m_name = _("Airhammer");
  override_keys = true ;

  impact = resource_manager.LoadImage( weapons_res_profile, "airhammer_impact");
  m_last_jolt = 0;
}

//-----------------------------------------------------------------------------

void Airhammer::p_Deselect()
{
  m_is_active = false;
}

//-----------------------------------------------------------------------------

bool Airhammer::p_Shoot()
{
  jukebox.Play("share","weapon/airhammer");

  // initiate movement ;-)
  ActiveCharacter().SetRebounding(false);

  // Little hack, so the character notices he is in the vaccum and begins to fall in the hole
  ActiveCharacter().SetXY( ActiveCharacter().GetPosition() );

  Point2i pos = Point2i(ActiveCharacter().GetX() + ActiveCharacter().GetWidth()/2 - impact.GetWidth()/2,
                        ActiveCharacter().GetTestRect().GetPositionY() +
                        ActiveCharacter().GetHeight()  -15);

  ParticleEngine::AddNow(pos + Point2i(impact.GetWidth()/4,9), 1, particle_AIR_HAMMER,
                         true, -3.0 * M_PI_4, 5.0 + Time::GetInstance()->Read() % 5);
  ParticleEngine::AddNow(pos + Point2i(3*impact.GetWidth()/4,9), 1, particle_AIR_HAMMER,
                         true, -M_PI_4, 5.0 + Time::GetInstance()->Read() % 5);
  world.Dig( pos, impact );

  return true;
}

//-----------------------------------------------------------------------------

void Airhammer::RepeatShoot()
{  
  uint time = Time::GetInstance()->Read() - m_last_jolt; 
  uint tmp = Time::GetInstance()->Read();

  if (time >= MIN_TIME_BETWEEN_JOLT) 
  {
    m_is_active = false;
    ActionHandler::GetInstance()->NewAction(new Action(ACTION_SYNC_BEGIN));
    ActionHandler::GetInstance()->NewAction(
      BuildActionSendCharacterPhysics(ActiveCharacter().GetTeamIndex(), ActiveCharacter().GetCharacterIndex()));
    NewActionShoot();
    ActionHandler::GetInstance()->NewAction(new Action(ACTION_SYNC_END));
    m_last_jolt = tmp;
  }    

}

//-----------------------------------------------------------------------------

void Airhammer::Refresh()
{
}

//-----------------------------------------------------------------------------

void Airhammer::HandleKeyEvent(int action, int event_type)
{
  switch (action) {    

  case ACTION_SHOOT:
    
    if (event_type == KEY_REFRESH)
      RepeatShoot();
    
    if (event_type == KEY_RELEASED) {
      // stop when key is released
      ActiveTeam().AccessNbUnits() = 0;
      m_is_active = false;
      GameLoop::GetInstance()->SetState(GameLoop::HAS_PLAYED);
    }

    break ;
    
  default:
    break ;
  } ;
      
}
//-----------------------------------------------------------------------------

