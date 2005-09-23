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

#include "../weapon/mine.h"
//-----------------------------------------------------------------------------
#include "../game/config.h"
#include "../include/constant.h"
#include "../map/map.h"
#include "../team/macro.h"
#include "../game/time.h"
#include "../interface/game_msg.h"
#include "../tool/i18n.h"
#include "../object/objects_list.h"
#include "../game/game_loop.h"
#include "../tool/random.h"
#include "../weapon/weapon_tools.h"
#ifdef CL
#include "../tool/geometry_tools.h"
#else
#include "../tool/sprite.h"
#include "../tool/resource_manager.h"
#include "../include/app.h"
#endif

#include <sstream>
//-----------------------------------------------------------------------------
namespace Wormux {

  Mine mine;
#ifdef DEBUG

  //#define DBG_PLACEMENT
#define DBG_DETECTION

#define COUT_DBG0 std::cout << "[" << m_name << "]"
#define COUT_DBG COUT_DBG0 " "
#define COUT_PLACEMENT COUT_DBG0 "[Reset bcl=" << bcl << "] "
#endif

//-----------------------------------------------------------------------------

const double DEPART_FONCTIONNEMENT = 5;

//-----------------------------------------------------------------------------

ObjMine::ObjMine() : PhysicalObj("mine", 0.0)
{
  SetTestRect (0, 4, 0, 3);
  m_allow_negative_y = true; 
  animation = false;
  m_rebounding = true;
  affiche = true;
  non_defectueuse = RandomLong(0, 9);
  Init();
}

//-----------------------------------------------------------------------------

void ObjMine::Init()
{

#ifdef CL

   detection = CL_Sprite("mine_anim", &graphisme.weapons);

  SetSize (detection.get_width(), detection.get_height());

  impact = CL_Surface("mine_impact", &graphisme.weapons);
  SetMass (mine.cfg().mass);
  
  explosion = CL_Sprite("explosion", &graphisme.weapons);

#else

  Profile *res = resource_manager.LoadXMLProfile( "weapons.xml");
 
  detection = resource_manager.LoadSprite(res,"mine_anim");
  SetSize (detection->GetWidth(), detection->GetHeight());

  impact = resource_manager.LoadImage(res,"mine_impact");
  SetMass (mine.cfg().mass);
  
  explosion = resource_manager.LoadSprite(res,"explosion");
   
#endif
   
  armer = temps.Lit() + mine.cfg().temps_fuite;
  depart = uint(temps.Lit() + DEPART_FONCTIONNEMENT * 1000);
}

//-----------------------------------------------------------------------------

void ObjMine::Reset()
{
  affiche = true;
  animation = false;
  ver_declancheur = NULL;

#ifdef CL
  detection.set_frame(0);
#else
  detection->SetCurrentFrame(0);
#endif
   
  uint bcl=0;
  uint x = RandomLong(0, monde.GetWidth() - GetWidth()), y;
  bool ok;
  do
  {
    ok=true;

    Ready();
    FORCE_ASSERT (++bcl < NBR_BCL_MAX_EST_VIDE);
    x = RandomLong(0, monde.GetWidth() - GetWidth());
    y = RandomLong(0, monde.GetHeight() - GetHeight());
    
    SetXY (x, y);
    DirectFall ();
#ifdef DBG_PLACEMENT
    COUT_PLACEMENT << "SetXY (" << x << "," << y << ")" << std::endl;
#endif
    ok &= !IsGhost() && IsInVacuum(0,0);
    if (!ok) continue;

    POUR_TOUS_VERS_VIVANTS(equipe, ver)
    {
      if (MeterDistance (GetCenter(), ver->GetCenter()) 
	   < mine.cfg().detection_range)
      { 
#ifdef DBG_PLACEMENT
	COUT_PLACEMENT << "Touche le ver " << (*ver).m_name << std::endl;
#endif
	ok = false; 
      }
    }
    ok &= !IsGhost() && IsInVacuum(0,0);
  } while (!ok);
#ifdef DBG_PLACEMENT
  COUT_PLACEMENT << "Placé." << std::endl;
#endif

  DirectFall();
  SetMass(mine.cfg().mass);
}

//-----------------------------------------------------------------------------

void ObjMine::Draw()
{ 
  if (!affiche) return;

#ifdef CL
   detection.draw (GetX(), GetY());
#else
   detection->Blit( app.sdlwindow, GetX(), GetY());
#endif
   
#if defined(DEBUG_CADRE_TEST)
  CL_Display::draw_rect (LitRectTest(), CL_Color::red);
#endif  
}

//-----------------------------------------------------------------------------

void ObjMine::SignalFallEnding()
{
#ifdef MSG_DBG_MINE
  COUT_DBG << "Fin de la chute : la mine est a terre." << std::endl;
#endif

  SetMass (mine.cfg().mass);
}

//-----------------------------------------------------------------------------

void ObjMine::Explosion ()
{
#ifdef DBG_DETECTION
  COUT_DBG << "Explosion()" << std::endl;
#endif
  affiche = false;
#ifdef CL
  CL_Point centre = GetCenter();	
#else
   Point2i centre = GetCenter();
#endif
  AppliqueExplosion (centre, centre, impact, mine.cfg(), NULL);
  if (ver_declancheur == &ActiveCharacter()) game_loop.interaction_enabled = false;
  DesactiveDetection();
}

//-----------------------------------------------------------------------------

void ObjMine::ActiveDetection()
{
  if (!animation)
  {
#ifdef DBG_DETECTION
    COUT_DBG << "ActiveDetection()" << std::endl;
#endif

    animation=true;
    affiche = true;
    armer = temps.Lit() + mine.cfg().temps_fuite;
    attente = temps.Lit() + mine.cfg().temps_fuite;
    m_ready = false;
#ifdef DBG_DETECTION
    COUT_DBG << "IsReady() : " << IsReady() << std::endl;
#endif
#ifdef CL
     jukebox.Play("weapon/mine_beep", true);    
#else
//TODO
#endif
  }
}

//-----------------------------------------------------------------------------
  
void ObjMine::DesactiveDetection()
{
  if (animation )//&& !repos)
  {
#ifdef DBG_DETECTION
    COUT_DBG << "DesactiveDetection()" << std::endl;
#endif
    animation = false;
    m_ready = true;

#ifdef CL
     detection.set_frame(0);
#else
     detection->SetCurrentFrame(0);
#endif
  }
}

//-----------------------------------------------------------------------------

void ObjMine::Detection()
{
  POUR_TOUS_VERS_VIVANTS(equipe, ver)
  { 
    if (MeterDistance (GetCenter(), ver->GetCenter())
	 < mine.cfg().detection_range && !animation)
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

//-----------------------------------------------------------------------------

void ObjMine::Refresh()
{
  if (temps.Lit() < depart) return;

  if (!affiche)
  {
#ifdef CL
    jukebox.Stop("weapon/mine_beep");
#else
     // TODO
#endif
     Ghost ();
    return;
  }

  if (!animation) Detection();

  if (animation) {
#ifdef CL
     detection.update();
#else
     detection->Update();
#endif
     if (attente < temps.Lit())
    {
#ifdef CL
      jukebox.Stop("weapon/mine_beep");
#else
       // TODO
#endif
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

//-----------------------------------------------------------------------------

void ObjMine::SignalGhostState (bool)
{
  if (!affiche) return;

  affiche=false;

#ifdef MSG_DBG_MINE
  COUT_DBG << "Une mine sort de l'écran !" << std::endl;
#endif

  Ghost();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Mine::Mine() : Weapon(WEAPON_MINE, "mine")
{
  m_name = _("Mine");

  already_put = false;
  m_visibility = VISIBLE_ONLY_WHEN_INACTIVE;

  extra_params = new MineConfig();
}

//-----------------------------------------------------------------------------

bool Mine::p_Shoot()
{
  int x,y;
  PosXY (x,y);
  Add (x, y);

  already_put = true;

  return true;
}

//-----------------------------------------------------------------------------

void Mine::Add (int x, int y)
{
  ObjMine *obj = new ObjMine();
  //obj -> Init();
  obj -> SetXY (x, y);

  DoubleVector speed_vector ;
  ActiveCharacter().GetSpeedXY(speed_vector);
  obj -> SetSpeedXY (speed_vector);
  lst_objets.AjouteObjet (obj, true);
  fuite = temps.Lit()+3000;
}

//-----------------------------------------------------------------------------

void Mine::Refresh()
{
  if (!already_put) return ;

  if (fuite < temps.Lit())
  {
    already_put = false;
    m_is_active = false;
    game_loop.SetState (gameHAS_PLAYED);
  }
}

//-----------------------------------------------------------------------------

MineConfig& Mine::cfg()
{ return static_cast<MineConfig&>(*extra_params); }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
} // namespace Wormux
