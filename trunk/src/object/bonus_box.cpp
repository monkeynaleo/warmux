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

//#define FAST

#ifdef FAST
  const uint MIN_TIME_BETWEEN_CREATION = 1; // seconds
  const uint MAX_TIME_BETWEEN_CREATION = 3; // seconds
#else
  const uint MIN_TIME_BETWEEN_CREATION = 10; // seconds
  const uint MAX_TIME_BETWEEN_CREATION = 5*60; // seconds
#endif

const uint SPEED = 5; // meter / seconde
const uint SPEED_PARACHUTE = 170; // ms par image
const uint NB_MAX_TRY = 20;

// Bonus offert par la caisse
const uint BONUS_ENERGY=100;
const uint BONUS_TRAP=75;
const uint BONUS_DYNAMITE=3;
const uint BONUS_ANVIL=1;
const uint BONUS_BASEBALL=3;
const uint BONUS_HOLLY_GRENADE=1;
const uint BONUS_LOWGRAV=2;
const uint BONUS_AIR_ATTACK=1;
const uint BONUS_TELEPORTATION=2;
const uint BONUS_AUTO_BAZOOKA=3;
const uint BONUS_RIOT_BOMB=2;



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
  nb_ammo = 3;

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
  FOR_ALL_LIVING_CHARACTERS(equipe, ver)
  {
    if( ObjTouche(*ver) )
    {
      // here is the gift (truly a gift ?!? :)
      ApplyBonus (**equipe, *ver);
      Ghost();
      return;
    }
  }

  // Refresh animation
  if (!anim->IsFinished() && !parachute) anim->Update();
}

// Signale la fin d'une chute
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

//Boxes can explode too...
void BonusBox::SignalDeath()
{
  ParticleEngine::AddNow(GetCenter() , 10, particle_FIRE, true);

  ExplosiveWeaponConfig cfg;
  cfg.blast_range = 3;
  cfg.blast_force = 25;
  cfg.explosion_range = 3;
  cfg.particle_range = 25;
  lst_objects.RemoveObject(this);
  ApplyExplosion(GetCenter(), cfg);
  Ghost();
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
        equipe.m_nb_ammos[ Config::GetInstance()->GetWeaponsList()->GetWeapon(contents)->GetName() ] += nb_ammo;
        txt << Format(ngettext(
                "%s team has won %u %s!",
                "%s team has won %u %ss!",
                2),
            ActiveTeam().GetName().c_str(), nb_ammo, Config::GetInstance()->GetWeaponsList()->GetWeapon(contents)->GetName().c_str());
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
uint BonusBox::time = 0;

// Active les caisses ?
void BonusBox::Enable (bool _enable)
{
  MSG_DEBUG("bonus", "Enable ? %d", _enable);
  enable = _enable;
}

bool BonusBox::PlaceBonusBox (BonusBox& bonus_box)
{
  if (!bonus_box.PutRandomly(true, 0)) return false;

  time = randomSync.GetLong(MIN_TIME_BETWEEN_CREATION,
			   MAX_TIME_BETWEEN_CREATION-MIN_TIME_BETWEEN_CREATION);
  time *= 1000;
  time += Time::GetInstance()->Read();

  return true;
}

bool BonusBox::NewBonusBox()
{

  if (!enable || (Time::GetInstance()->Read() < time)) {
    return false;
  }

  BonusBox * box = new BonusBox();
  if (!PlaceBonusBox(*box)) {
    MSG_DEBUG("bonus", "Missed to put the bonus box");
    delete box;
  } else {
    lst_objects.AddObject(box);
    camera.ChangeObjSuivi(box, true, true);
    GameMessages::GetInstance()->Add (_("Is it a gift?"));
    return true;
  }

  return false;
}
