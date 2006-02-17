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
 * WeaponLauncher: generic weapon to launch a projectile
 *****************************************************************************/

#include "launcher.h"

#include <sstream>
#include "weapon_tools.h"
#include "../game/config.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../graphic/video.h"
#include "../include/global.h"
#include "../interface/game_msg.h"
#include "../interface/interface.h"
#include "../map/camera.h"
#include "../object/objects_list.h"
#include "../team/macro.h"
#include "../team/teams_list.h"
#include "../tool/math_tools.h"
#include "../tool/i18n.h"


WeaponProjectile::WeaponProjectile (GameLoop &p_game_loop, const std::string &name, 
				    WeaponLauncher * p_launcher)
  : PhysicalObj (p_game_loop, name, 0.0)
{
  launcher = p_launcher;
  
  dernier_ver_touche = NULL;
  dernier_obj_touche = NULL;

  m_allow_negative_y = true;
  touche_ver_objet = true;
  m_wind_factor = 1.0;

  image = resource_manager.LoadSprite( weapons_res_profile, name);
  image->EnableRotationCache(32);
  SetSize (image->GetWidth(), image->GetHeight());

  impact = resource_manager.LoadImage( weapons_res_profile, name + "_impact");

  if (launcher != NULL) {
    SetMass (launcher->cfg().mass);
    SetWindFactor(launcher->cfg().wind_factor);
    SetAirResistFactor(launcher->cfg().air_resist_factor);
  }

  // Set rectangle test
  int dx = image->GetWidth()/2-1;
  int dy = image->GetHeight()/2-1;
  SetTestRect (dx, dx, dy, dy);   
}

WeaponProjectile::~WeaponProjectile()
{
  delete image;
}

void WeaponProjectile::Shoot(double strength)
{
  Ready();
  camera.ChangeObjSuivi(this, true, false);
  is_active = true;

  // Set the initial position.
  int x,y;
  ActiveCharacter().GetHandPosition(x, y);
  SetXY (x,y);
  
  // Set the initial speed.
  double angle = ActiveTeam().crosshair.GetAngleRad();
  SetSpeed (strength, angle);
  PutOutOfGround(angle); 

  begin_time = global_time.Read();
}

bool WeaponProjectile::TestImpact()
{
  if (IsReady()) 
  {
#ifdef DEBUG_MSG_COLLISION
    COUT_DBG << "Impact because was ready." << std::endl;
#endif
    return true;
  }
  return CollisionTest (0,0);
}

bool WeaponProjectile::CollisionTest(int dx, int dy)
{
  dernier_ver_touche = NULL;
  dernier_obj_touche = NULL;

  if (!IsInVacuum (dx, dy)) return true;

  if (!touche_ver_objet) return false;

   Rectanglei test = GetTestRect();
   test.SetPositionX( test.GetPositionX() + dx);
   test.SetPositionY( test.GetPositionY() + dy);
   
  FOR_ALL_LIVING_CHARACTERS(equipe,ver)
  if (&(*ver) != &ActiveCharacter())
  {
    if (ver->GetTestRect().Intersect( test ))
       {
      dernier_ver_touche = &(*ver);
#ifdef DEBUG_MSG_COLLISION
      COUT_DBG << "On a touché le ver : " << ver -> m_name << std::endl;
#endif
      return true;
    }
  }

  FOR_EACH_OBJECT(objet)
  if (objet -> ptr != this)
  {
    if ( objet->ptr->GetTestRect().Intersect( test ) )
      {
      dernier_obj_touche = objet -> ptr;
#ifdef DEBUG_MSG_COLLISION
      COUT_DBG << "On a touché un objet : " 
                << objet -> ptr -> m_name << std::endl;      
#endif
      return true;
    }
  }

  return false;
}

void WeaponProjectile::Refresh()
{
  if( !is_active )
    return;

  if( TestImpact() ){
    SignalCollision();
    return;
  }  

  // Explose after timeout
  double tmp = global_time.Read() - begin_time;
  if(tmp>1000 * launcher->cfg().timeout) {
    is_active = false;
    return;
  }
}

void WeaponProjectile::Draw()
{
  if( !is_active )
    return;

  image->Draw(GetX(), GetY());   

  int tmp = launcher->cfg().timeout;
  if (tmp != 0) {
    tmp -= (int)((global_time.Read() - begin_time) / 1000);
    std::ostringstream ss;
    ss << tmp;
    int txt_x = GetX() + GetWidth() / 2;
    int txt_y = GetY() - GetHeight();
    global().small_font().WriteCenterTop (txt_x-camera.GetX(), txt_y-camera.GetY(), ss.str(), white_color);
  }
}

void WeaponProjectile::SignalGhostState (bool){
  SignalCollision();
}

void WeaponProjectile::SignalFallEnding(){
  SignalCollision();
}

void WeaponProjectile::Explosion()
{
  if (IsGhost()) return;

  // Applique les degats et le souffle aux vers
  Point2i pos = GetCenter();
  AppliqueExplosion (pos, pos, impact, launcher->cfg(), NULL);
}

//-----------------------------------------------------------------------------

WeaponLauncher::WeaponLauncher(Weapon_type type, 
			       const std::string &id,
			       EmptyWeaponConfig * params,
			       uint visibility) :
  Weapon(type, id, params, visibility)
{  
  projectile = NULL;
}

WeaponLauncher::~WeaponLauncher()
{
  if (projectile != NULL) delete projectile;
}

bool WeaponLauncher::p_Shoot ()
{
//   if (m_strength == max_strength)
//   {
//     m_strength = 0;
//     DirectExplosion();
//     return true;
//   }

  projectile->Shoot (m_strength);
  lst_objets.AjouteObjet (projectile, true);
  camera.ChangeObjSuivi(projectile, true, true, true);

  jukebox.Play(ActiveTeam().GetSoundProfile(), "fire");
  return true;
}

// Le bazooka explose car il a été poussé à bout !
void WeaponLauncher::DirectExplosion()
{
  Point2i pos = ActiveCharacter().GetCenter();
  AppliqueExplosion (pos, pos, projectile->impact, cfg(), NULL);
}

void WeaponLauncher::Explosion()
{
  m_is_active = false;
  
  lst_objets.RetireObjet (projectile);

  projectile->Explosion();
}

void WeaponLauncher::Refresh()
{
  if (!m_is_active) return;
  if (!projectile->is_active) Explosion();
}

ExplosiveWeaponConfig& WeaponLauncher::cfg()
{ return static_cast<ExplosiveWeaponConfig&>(*extra_params); }

