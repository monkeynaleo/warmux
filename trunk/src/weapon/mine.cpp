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
 * Objet Mine: Si on active les mines, elles seront plac�es de maniere
 * aleatoire sur le terrain. Au contact d'un ver, ca fait BOOM!!! ;)
 *****************************************************************************/

#include "mine.h"
#include <iostream>
#include <sstream>
#include "weapon_tools.h"
#include "../game/config.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../graphic/sprite.h"
#include "../include/app.h"
#include "../include/constant.h"
#include "../interface/game_msg.h"
#include "../map/map.h"
#include "../object/objects_list.h"
#include "../team/macro.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
#include "../tool/random.h"
#include "../tool/resource_manager.h"

#ifdef __MINGW32__
#undef LoadImage
#endif

const double DEPART_FONCTIONNEMENT = 5;

ObjMine::ObjMine(GameLoop &p_game_loop, MineConfig& cfg) : 
  WeaponProjectile(p_game_loop, "mine", cfg)
{
  m_allow_negative_y = true; 
  animation = false;
  m_rebounding = true;
  channel = -1;

  escape_time = 0;

  Profile *res = resource_manager.LoadXMLProfile( "weapons.xml", false); 
  explosion = resource_manager.LoadSprite(res,"explosion");  

  Ready();
  is_active = true;
}

void ObjMine::SignalCollision() 
{
  if (IsGhost()) is_active = false;
}


void ObjMine::Explosion ()
{
  MSG_DEBUG("mine", "Explosion");

  Point2i centre = GetCenter();
  AppliqueExplosion(centre, centre, impact, cfg, NULL);
  DisableDetection();  
  lst_objects.RemoveObject (this);
}

void ObjMine::EnableDetection()
{
  if (!animation)
  {
    animation=true;
    MSG_DEBUG("mine", "EnableDetection - CurrentTime : %d",global_time.ReadSec() );
    attente = global_time.ReadSec() + cfg.timeout;
    MSG_DEBUG("mine", "EnableDetection : %d", attente);
    m_ready = false;
    MSG_DEBUG("mine", "IsReady() = %d", IsReady());

    channel = jukebox.Play("share", "weapon/mine_beep", -1);
  }
}

void ObjMine::DisableDetection()
{
  if (animation )//&& !repos)
  {
    MSG_DEBUG("mine", "Desactive detection..");

    animation = false;
    m_ready = true;

    image->SetCurrentFrame(0);
  }
}

void ObjMine::Detection()
{
  uint current_time = global_time.ReadSec();

  if (escape_time == 0) {
    escape_time = current_time + 3;
    MSG_DEBUG("mine", "Initialize escape_time : %d", current_time);
    return;
  }

  if (current_time < escape_time) {
    return;
  }

  MSG_DEBUG("mine", "Escape_time is finished : %d", current_time);

  FOR_ALL_LIVING_CHARACTERS(equipe, ver)
  { 
    if (MeterDistance (GetCenter(), ver->GetCenter())
	 < static_cast<MineConfig&>(cfg).detection_range && !animation)
    {
      std::string txt = Format(_("%s is next to a mine!"),
			       ver -> m_name.c_str());
      game_messages.Add (txt);
      EnableDetection();
      return;
    }
  }
}

void ObjMine::Refresh()
{
  if (!is_active)
  {
    jukebox.Stop(channel);
    channel = -1;
    escape_time = 0;
    Ghost ();
    return;
  }

  if (!animation) Detection();

  if (animation) {
     image->Update();

     if (attente < global_time.ReadSec())
       {
	 jukebox.Stop(channel);
	 channel = -1;
	 
	 if (randomObj.GetLong(0, 9))
	   {
	     Explosion ();
	   }
	 else
	   {

	     DisableDetection();
	   }
       }
  }
}

//-----------------------------------------------------------------------------


Mine::Mine() : WeaponLauncher(WEAPON_MINE, "minelauncher", new MineConfig(), VISIBLE_ONLY_WHEN_INACTIVE)
{
  m_name = _("Mine");
}

bool Mine::p_Shoot()
{
  int x,y;
  PosXY (x,y);
  Add (x, y);

  return true;
}

void Mine::Add (int x, int y)
{
  ObjMine *obj = new ObjMine(game_loop, cfg());
  obj -> SetXY ( Point2i(x, y) );

  Point2d speed_vector;
  ActiveCharacter().GetSpeedXY(speed_vector);
  obj -> SetSpeedXY (speed_vector);
  lst_objects.AddObject (obj);
}

void Mine::Refresh()
{
  m_is_active = false;
}

MineConfig& Mine::cfg()
{ return static_cast<MineConfig&>(*extra_params); }

MineConfig::MineConfig()
{
  detection_range= 1;
  timeout = 3;
}

void MineConfig::LoadXml(xmlpp::Element *elem) 
{
  ExplosiveWeaponConfig::LoadXml (elem);
  LitDocXml::LitDouble (elem, "detection_range", detection_range);
}

