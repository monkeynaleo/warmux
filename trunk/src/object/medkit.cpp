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
 * Medkit : fall from the sky at random time.
 * Contains health for a character.
 *****************************************************************************/

#include "medkit.h"
#include <sstream>
#include <iostream>
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../graphic/sprite.h"
#include "../include/app.h"
#include "../interface/game_msg.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../network/randomsync.h"
#include "../object/objects_list.h"
#include "../team/macro.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
#include "../tool/resource_manager.h"
#include "../weapon/explosion.h"

const uint SPEED = 5; // meter / seconde
const uint SPEED_PARACHUTE = 170; // ms par image
const uint NB_MAX_TRY = 20;

Medkit::Medkit()
  : PhysicalObj("medkit"){
  SetTestRect (29, 29, 63, 6);
  m_allow_negative_y = true;
  //enable = false; //see bonus_box for why this is wrong

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  anim = resource_manager.LoadSprite( res, "object/medkit");
  resource_manager.UnLoadXMLProfile(res);

  SetSize(anim->GetSize());
  anim->animation.SetLoopMode(false);
  anim->SetCurrentFrame(0);

  parachute = true;

  life_points = start_life_points;

  SetSpeed (SPEED, M_PI_2);
}

Medkit::~Medkit(){
  delete anim;
  GameLoop::GetInstance()->SetCurrentMedkit(NULL);
}

void Medkit::Draw()
{
  anim->Draw(GetPosition());
}

void Medkit::Refresh()
{
  // If we touch a character, we remove the medkit
  FOR_ALL_LIVING_CHARACTERS(equipe, ver)
  {
    if( ObjTouche(*ver) )
    {
      // here is the gift (truly a gift ?!? :)
      ApplyMedkit (**equipe, *ver);
      Ghost();
      return;
    }
  }

  // Refresh animation
  if (!anim->IsFinished() && !parachute) anim->Update();
}

// Say hello to the ground
void Medkit::SignalCollision()
{
  SetAirResistFactor(1.0);
  GameLoop::GetInstance()->SetCurrentMedkit(NULL);
  MSG_DEBUG("medkit", "End of the fall: parachute=%d", parachute);
  if (!parachute) return;

  MSG_DEBUG("medkit", "Start of the animation 'fold of the parachute'.");
  parachute = false;

  anim->SetCurrentFrame(0);
  anim->Start();
}

void Medkit::SignalDrowning()
{
  SignalCollision();
}

void Medkit::DropMedkit()
{
  if(parachute) {
    SetAirResistFactor(1.0);
    parachute = false;
    anim->SetCurrentFrame(anim->GetFrameCount() - 1);
  } else {
    m_ignore_movements = true;
  }
}

//Medkits can explode too...
void Medkit::SignalGhostState(bool was_already_dead)
{
  if(life_points > 0) return;
  ParticleEngine::AddNow(GetCenter() , 10, particle_FIRE, true);
  ApplyExplosion(GetCenter(), GameMode::GetInstance()->bonus_box_explosion_cfg); //reuse the bonus_box explosion
}

void Medkit::ApplyMedkit(Team &equipe, Character &ver) {
  std::ostringstream txt;
 txt << Format(ngettext(
                "%s has won %u point of energy!",
                "%s has won %u points of energy!",
                100),
            ver.GetName().c_str(), nbr_health);
  ver.SetEnergyDelta (nbr_health);
  GameMessages::GetInstance()->Add (txt.str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Static methods
bool Medkit::enable = true;
int Medkit::start_life_points = 41;
int Medkit::nbr_health = 24;

// Make the medkit active?
void Medkit::Enable (bool _enable)
{
  MSG_DEBUG("medkit", "Enable ? %d", _enable);
  enable = _enable;
}

bool Medkit::NewMedkit()
{
  if (!enable) { // Medkits are disabled on closed map
    return false;
  }

  uint nbr_teams=teams_list.playing_list.size();
  if(nbr_teams<=1) {
    MSG_DEBUG("medkit", "There is less than 2 teams in the game");
    return false;
  }
  // .7 is a magic number to get the probability of boxes falling once every round close to .333
  double randValue = randomSync.GetDouble();
  if(randValue > (1-pow(.5,1.0/nbr_teams))) {
       return false;
  }

  Medkit * box = new Medkit();
  if(!box->PutRandomly(true,0)) {
    MSG_DEBUG("medkit", "Missed to put the medkit");
    delete box;
  } else {
    lst_objects.AddObject(box);
    camera.FollowObject(box, true, true);
    GameMessages::GetInstance()->Add (_("Healing from above."));
    GameLoop::GetInstance()->SetCurrentMedkit(box);
    return true;
  }

  return false;
}

void Medkit::LoadXml(xmlpp::Element * object)
{
  XmlReader::ReadInt(object,"life_points",start_life_points);
  XmlReader::ReadInt(object,"energy_boost",nbr_health);
}
