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
 * Bonus box : fall from the sky at random time.
 * The box can contain bonus or mallus (dynamite, Guns, loss of energy etc).
 *****************************************************************************/

#include "bonus_box.h"
#include <sstream>
#include <iostream>
#include <math.h>
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
#include "../weapon/weapons_list.h"

const uint SPEED = 5; // meter / seconde
const uint SPEED_PARACHUTE = 170; // ms per frame
const uint NB_MAX_TRY = 20;

BonusBox::BonusBox()
  : PhysicalObj("bonus_box"){
  SetTestRect (29, 29, 63, 6);
  m_allow_negative_y = true;
  enable = false;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  anim = resource_manager.LoadSprite( res, "objet/caisse");
  resource_manager.UnLoadXMLProfile(res);

  SetSize(anim->GetSize());
  anim->animation.SetLoopMode(false);
  anim->SetCurrentFrame(0);

  parachute = true;

  //these values will get read from XML soon
  life_points = 41;
  nbr_ammo = 3;

  SetSpeed (SPEED, M_PI_2);
  PickRandomWeapon();
}

BonusBox::~BonusBox(){
  delete anim;
}

void BonusBox::Draw()
{
  anim->Draw(GetPosition());
}

void BonusBox::Refresh()
{
  // If we touch a character, we remove the bonus box
  FOR_ALL_LIVING_CHARACTERS(team, character)
  {
    if( ObjTouche(*character) )
    {
      // here is the gift (truly a gift ?!? :)
      ApplyBonus (**team, *character);
      Ghost();
      return;
    }
  }

  // Refresh animation
  if (!anim->IsFinished() && !parachute) anim->Update();
}

// Say hello to the ground
void BonusBox::SignalCollision()
{
  SetAirResistFactor(1.0);

  MSG_DEBUG("bonus", "End of the fall: parachute=%d", parachute);
  if (!parachute) return;

  MSG_DEBUG("bonus", "Start of the animation 'fold of the parachute'.");
  parachute = false;

  anim->SetCurrentFrame(0);
  anim->Start();
}

// Boxes can explode too ...
void BonusBox::SignalGhostState(bool was_already_dead)
{
  if(life_points > 0) return;
  ParticleEngine::AddNow(GetCenter() , 10, particle_FIRE, true);
  ApplyExplosion(GetCenter(), GameMode::GetInstance()->bonus_box_explosion_cfg);
}

void BonusBox::PickRandomWeapon() {

  int weapon_count = 0;
  int weapon_num = 0;
  WeaponsList::weapons_list_it it;
  for(it = Config::GetInstance()->GetWeaponsList()->GetList().begin(); it != Config::GetInstance()->GetWeaponsList()->GetList().end(); it++) {
    weapon_count++;
  }
  weapon_num = (int)randomSync.GetDouble(1,weapon_count);
  it = Config::GetInstance()->GetWeaponsList()->GetList().begin();
  int a=0;
  while(a < weapon_num && it != Config::GetInstance()->GetWeaponsList()->GetList().end()) {
    it++;
    a++;
  }
  contents = (*it)->GetType();
  std::cout<<"Out of "<<weapon_count<<" weapons, weapon "<<weapon_num<<" was selected."<<std::endl;
}

void BonusBox::ApplyBonus(Team &equipe, Character &ver) {
  std::ostringstream txt;
    if(ActiveTeam().ReadNbAmmos(Config::GetInstance()->GetWeaponsList()->GetWeapon(contents)->GetName())!=INFINITE_AMMO) {
        equipe.m_nb_ammos[ Config::GetInstance()->GetWeaponsList()->GetWeapon(contents)->GetName() ] += nbr_ammo;
        txt << Format(ngettext(
                "%s team has won %u %s!",
                "%s team has won %u %ss!",
                2),
            ActiveTeam().GetName().c_str(), nbr_ammo, Config::GetInstance()->GetWeaponsList()->GetWeapon(contents)->GetName().c_str());
    }
    else {
        txt << Format(gettext("%s team already has infinite ammo for the %s!"),
            ActiveTeam().GetName().c_str(), Config::GetInstance()->GetWeaponsList()->GetWeapon(contents)->GetName().c_str());
    }
  GameMessages::GetInstance()->Add (txt.str());
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Static methods
bool BonusBox::enable = false;

// Active les caisses ?
void BonusBox::Enable (bool _enable)
{
  MSG_DEBUG("bonus", "Enable ? %d", _enable);
  enable = _enable;
}

uint BonusBox::CountTeams() {
  uint nbr_teams=0;
  FOR_EACH_TEAM(team) { 
    nbr_teams++;
  }
  return nbr_teams;
}

bool BonusBox::PlaceBonusBox (BonusBox& bonus_box)
{
  if (!bonus_box.PutRandomly(true, 0)) return false;
  return true;
}

bool BonusBox::NewBonusBox()
{

  if (!enable) {
     enable=true;
    return false;
  }
  uint nbr_teams=CountTeams();
  if(nbr_teams<=1) {
	MSG_DEBUG("bonus", "There is less than 2 teams in the game");
    return false;
  }
  // .7 is a magic number to get the probability of boxes falling once every round close to .333
  double randValue = randomSync.GetDouble();
  if(randValue > (1-pow(.7,1.0/nbr_teams))) {
       return false;
  }

  BonusBox * box = new BonusBox();
  if (!PlaceBonusBox(*box)) {
    MSG_DEBUG("bonus", "Missed to put the bonus box");
    delete box;
  } else {
    lst_objects.AddObject(box);
    camera.FollowObject(box, true, true);
    GameMessages::GetInstance()->Add (_("Is it a gift?"));
    return true;
  }

  return false;
}
