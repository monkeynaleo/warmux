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
 * Objet Mine: Si on active les mines, elles seront placées de maniere
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

ObjMine::ObjMine(GameLoop &p_game_loop, Mine& p_launcher) : 
  PhysicalObj(p_game_loop, "mine", 0.0),
  launcher(p_launcher)
{
  SetTestRect (0, 4, 0, 3);
  m_allow_negative_y = true; 
  animation = false;
  m_rebounding = true;
  affiche = true;
  non_defectueuse = randomObj.GetLong(0, 9);
  channel = -1;

  Profile *res = resource_manager.LoadXMLProfile( "weapons.xml", false);
 
  detection = resource_manager.LoadSprite(res,"mine_anim");
  SetSize(detection->GetSize());

  impact = resource_manager.LoadImage(res,"mine_impact");
  SetMass (launcher.cfg().mass);
  
  explosion = resource_manager.LoadSprite(res,"explosion");
   
  armer = global_time.Read() + launcher.cfg().temps_fuite;
  depart = uint(global_time.Read() + DEPART_FONCTIONNEMENT * 1000);
}

void ObjMine::Reset()
{
  affiche = true;
  animation = false;
  ver_declancheur = NULL;

  detection->SetCurrentFrame(0);
   
  uint bcl=0;
  //uint x = randomObj.GetLong(0, world.GetWidth() - GetWidth()), y;
  Point2i pos;
  bool ok;
  do
  {
    ok = true;

    Ready();
    FORCE_ASSERT (++bcl < NBR_BCL_MAX_EST_VIDE);
	pos = randomObj.GetPoint( world.GetSize() - GetSize() );
    
    SetXY( pos );
    DirectFall();
	MSG_DEBUG("mine", "SetXY( %d, %d )", pos.x, pos.y);
    ok &= !IsGhost() && IsInVacuum( Point2i(0,0) );
    if (!ok) continue;

    FOR_ALL_LIVING_CHARACTERS(equipe, ver)
    {
      if (MeterDistance (GetCenter(), ver->GetCenter()) 
	   < launcher.cfg().detection_range)
      { 
	    MSG_DEBUG("mine", "Touche le ver %s", (*ver).m_name);
	    ok = false; 
      }
    }
    ok &= !IsGhost() && !IsInWater() && IsInVacuum( Point2i(0,0) );
  } while (!ok);
  MSG_DEBUG("mine", "Placé.");

  DirectFall();
  SetMass(launcher.cfg().mass);
}

void ObjMine::Draw()
{ 
  if (!affiche) return;

  detection->Draw(GetX(), GetY());
}

void ObjMine::SignalFallEnding()
{
  MSG_DEBUG("mine", "Fin de la chute: la mine est a terre.");
  SetMass (launcher.cfg().mass);
}

void ObjMine::Explosion ()
{
  MSG_DEBUG("mine", "Explosion");
  affiche = false;

  Point2i centre = GetCenter();
  AppliqueExplosion(centre, centre, impact, launcher.cfg(), NULL);
  DesactiveDetection();
}

void ObjMine::ActiveDetection()
{
  if (!animation)
  {
    animation=true;
    affiche = true;
    armer = global_time.Read() + launcher.cfg().temps_fuite;
    attente = global_time.Read() + launcher.cfg().temps_fuite;
    m_ready = false;
	MSG_DEBUG("mine", "IsReady() = %d", IsReady());

    channel = jukebox.Play("share", "weapon/mine_beep", -1);
  }
}

void ObjMine::DesactiveDetection()
{
  if (animation )//&& !repos)
  {
	MSG_DEBUG("mine", "Desactive detection..");

    animation = false;
    m_ready = true;

    detection->SetCurrentFrame(0);
  }
}

void ObjMine::Detection()
{
  FOR_ALL_LIVING_CHARACTERS(equipe, ver)
  { 
    if (MeterDistance (GetCenter(), ver->GetCenter())
	 < launcher.cfg().detection_range && !animation)
    {
      ver_declancheur = &(*ver);
      std::string txt = Format(_("%s is next to a mine!"),
			       ver -> m_name.c_str());
      game_messages.Add (txt);
      ActiveDetection();
      return;
    }
  }
}

void ObjMine::Refresh()
{
  if (global_time.Read() < depart) return;

  if (!affiche)
  {
    jukebox.Stop(channel);
    channel = -1;

    Ghost ();
    return;
  }

  if (!animation) Detection();

  if (animation) {
     detection->Update();

     if (attente < global_time.Read())
       {
	 jukebox.Stop(channel);
	 
	 if (non_defectueuse)
	   {
	     Explosion ();
	   }
	 else
	   {
	     affiche = false;
	     DesactiveDetection();
	   }
       }
  }
}

void ObjMine::SignalGhostState (bool)
{
  if (!affiche) return;

  affiche=false;

  MSG_DEBUG("mine", "Une mine sort de l'écran");

  Ghost();
}

Mine::Mine() : Weapon(WEAPON_MINE, "mine", new MineConfig(), VISIBLE_ONLY_WHEN_INACTIVE)
{
  m_name = _("Mine");

  already_put = false;
}

bool Mine::p_Shoot()
{
  int x,y;
  PosXY (x,y);
  Add (x, y);

  already_put = true;

  return true;
}

void Mine::Add (int x, int y)
{
  ObjMine *obj = new ObjMine(game_loop, *this);
  obj -> SetXY ( Point2i(x, y) );

  Point2d speed_vector;
  ActiveCharacter().GetSpeedXY(speed_vector);
  obj -> SetSpeedXY (speed_vector);
  lst_objets.AjouteObjet (obj, true);
  fuite = global_time.Read()+3000;
}

void Mine::Refresh()
{
  if (!already_put) return ;

  if (fuite < global_time.Read())
  {
    already_put = false;
    m_is_active = false;
    game_loop.SetState (gameHAS_PLAYED);
  }
}

MineConfig& Mine::cfg()
{ return static_cast<MineConfig&>(*extra_params); }

MineConfig::MineConfig()
{
  detection_range= 1;
  temps_fuite = 3000;
}

void MineConfig::LoadXml(xmlpp::Element *elem) 
{
  ExplosiveWeaponConfig::LoadXml (elem);
  LitDocXml::LitDouble (elem, "detection_range", detection_range);
  LitDocXml::LitUint (elem, "temps_fuite", temps_fuite);
}

