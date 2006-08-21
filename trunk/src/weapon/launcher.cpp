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

#include "explosion.h"
#include "../game/config.h"
#include "../game/time.h"
#include "../graphic/video.h"
#include "../include/action_handler.h"
#include "../interface/game_msg.h"
#include "../map/camera.h"
#include "../object/objects_list.h"
#include "../team/macro.h"
#include "../team/teams_list.h"
#include "../tool/debug.h"
#include "../tool/math_tools.h"
#include "../tool/i18n.h"

WeaponBullet::WeaponBullet(const std::string &name,
                           ExplosiveWeaponConfig& cfg,
                           WeaponLauncher * p_launcher) :
  WeaponProjectile(name, cfg, p_launcher)
{ 
  cfg.explosion_range = 1;
  explode_colliding_character = true;
  ResetTimeOut();
}

// Signal that the bullet has hit the ground or is out of the world
void WeaponBullet::SignalCollision()
{
  if ( GetLastCollidingObject() == NULL )
  {
    GameMessages::GetInstance()->Add (_("Your shot has missed!"));
  }
  is_active = false;

  // projectile explode and signal to the launcher the collision
  lst_objects.RemoveObject(this);
  if (!IsGhost()) Explosion();
  if (launcher != NULL) launcher->SignalProjectileCollision();
}

void WeaponBullet::Refresh()
{
  WeaponProjectile::Refresh();

  double angle = GetSpeedAngle() *180/M_PI;
  image->SetRotation_deg(angle);
}

// Explode if hit the ground or apply damage to character
void WeaponBullet::Explosion()
{
  MSG_DEBUG(m_name->c_str(), "Impact");
  if (IsGhost()) return;

  if ( GetLastCollidingObject() == NULL ) {
    Point2i pos = GetCenter();
    ApplyExplosion (pos, cfg, "", false, ParticleEngine::LittleESmoke);
  } else {
    PhysicalObj * obj = GetLastCollidingObject();

    if (typeid(*obj) == typeid(Character)) {
      Character * tmp = (Character*)(obj);      
      tmp -> SetEnergyDelta (-cfg.damage);
      tmp -> AddSpeed (2, GetSpeedAngle());
      tmp -> UpdatePosition();
    }
  }
}

//-----------------------------------------------------------------------------


WeaponProjectile::WeaponProjectile (const std::string &name, 
                                    ExplosiveWeaponConfig& p_cfg,
                                    WeaponLauncher * p_launcher)
  : PhysicalObj (name), cfg(p_cfg)
{
  m_allow_negative_y = true;
  SetCollisionModel(false, true, true);
  explode_colliding_character = false;
  launcher = p_launcher;

  image = resource_manager.LoadSprite( weapons_res_profile, name);
  image->EnableRotationCache(32);
  SetSize(image->GetSize());

  // Set rectangle test
  int dx = image->GetWidth()/2-1;
  int dy = image->GetHeight()/2-1;
  SetTestRect (dx, dx, dy, dy);
  
  ResetTimeOut();
}

WeaponProjectile::~WeaponProjectile()
{
  //  delete image; /*-> it causes a segfault :-/ */
}

void WeaponProjectile::Shoot(double strength)
{    
  MSG_DEBUG("weapon_projectile", "shoot.\n");

  Ready();

  if (launcher != NULL) launcher->IncActiveProjectile();

  is_active = true;

  // Set the physical factors
  ResetConstants();

  // Set the initial position.
  SetXY( ActiveCharacter().GetHandPosition() );
  
  // Set the initial speed.
  double angle = ActiveTeam().crosshair.GetAngleRad();
  SetSpeed (strength, angle);
  PutOutOfGround(angle);

  begin_time = Time::GetInstance()->Read();

  ShootSound();

  lst_objects.AddObject (this);
  camera.ChangeObjSuivi(this, true, true, true);
}

void WeaponProjectile::ShootSound()
{
  jukebox.Play(ActiveTeam().GetSoundProfile(), "fire");
}

// To remove ?
bool WeaponProjectile::TestImpact()
{
  if (IsReady()) 
  {
    MSG_DEBUG("weapon_collision", "Impact because was ready.\n");
    return true;
  }
//  return CollisionTest (0,0); ---> CollisionTest called from PhysicalObj::NotifyMove(...)
  return false;
}

// projectile explode and signal to the launcher the collision
void WeaponProjectile::SignalCollisionObject()
{  
  if (!explode_colliding_character) return;

  PhysicalObj * obj = GetLastCollidingObject();
  assert (obj != NULL);
  
  if (typeid(*obj) == typeid(Character)) is_active = false;

  lst_objects.RemoveObject(this);
  if (!IsGhost()) Explosion();
  if (launcher != NULL) launcher->SignalProjectileCollision();
}

void WeaponProjectile::Refresh()
{
  if( !is_active )
    return;

  // Explose after timeout
  double tmp = Time::GetInstance()->Read() - begin_time;
   
  if(cfg.timeout && tmp > 1000 * (GetTotalTimeout())) {
    is_active = false;
    return;
  }

  // To remove ???
  if( TestImpact() ){
    SignalCollision();
    return;
  }
}

void WeaponProjectile::Draw()
{
  if( !is_active )
    return;

  image->Draw(GetPosition());
  
  int tmp = GetTotalTimeout();

  if (cfg.timeout && tmp != 0) { 
    tmp -= (int)((Time::GetInstance()->Read() - begin_time) / 1000);

    if (tmp >= 0) {
      std::ostringstream ss;
      ss << tmp ;
      int txt_x = GetX() + GetWidth() / 2;
      int txt_y = GetY() - GetHeight();
      (*Font::GetInstance(Font::FONT_SMALL)).WriteCenterTop( Point2i(txt_x, txt_y) - camera.GetPosition(),
      ss.str(), white_color);
    }
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
  MSG_DEBUG (m_name, "Explosion");
  if (IsGhost()) return;
  Point2i pos = GetCenter();
  ApplyExplosion (pos, cfg);
}


void WeaponProjectile::IncrementTimeOut()
{
  if (cfg.allow_change_timeout)
    if (GetTotalTimeout()<(int)cfg.timeout*2) 
      m_timeout_modifier += 1 ;

}

void WeaponProjectile::DecrementTimeOut()
{
  if (cfg.allow_change_timeout)
    if (GetTotalTimeout()>1) 
      m_timeout_modifier -= 1 ;      //-1s for grenade timout. 1 is min.
        
}

void WeaponProjectile::SetTimeOut(int timeout)
{
  if (cfg.allow_change_timeout)
    if (timeout <= (int)cfg.timeout*2 && timeout >= 1) 
      m_timeout_modifier = timeout - cfg.timeout ;

}

void WeaponProjectile::ResetTimeOut()
{
  m_timeout_modifier = 0 ;       
        
}

int WeaponProjectile::GetTotalTimeout()
{
  return (int)(cfg.timeout)+m_timeout_modifier;
}

//Public function which let know if changing timeout is allowed.
bool WeaponProjectile::change_timeout_allowed()
{
  return cfg.allow_change_timeout;
}

//-----------------------------------------------------------------------------

WeaponLauncher::WeaponLauncher(Weapon_type type, 
                               const std::string &id,
                               EmptyWeaponConfig * params,
                               weapon_visibility_t visibility) :
    Weapon(type, id, params, visibility)
{  
  projectile = NULL;
  nb_active_projectile = 0;
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
  return true;
}

// Direct Explosion when pushing weapon to max power !
void WeaponLauncher::DirectExplosion()
{
  Point2i pos = ActiveCharacter().GetCenter();
  ApplyExplosion (pos, cfg());
}

// Signal that a projectile fired by this weapon has hit something (ground, character etc)
void WeaponLauncher::SignalProjectileCollision()
{
  DecActiveProjectile();
  m_is_active = false;
}

// Keep the total amount of active projectile
void WeaponLauncher::IncActiveProjectile()
{
  ++nb_active_projectile;
}

void WeaponLauncher::DecActiveProjectile()
{
  --nb_active_projectile;
}

// Call by the object list class to refresh the weapon's state
void WeaponLauncher::Refresh()
{
}

void WeaponLauncher::Draw()
{
  //Display timeout for projectil if can be changed.
  if (projectile->change_timeout_allowed())
  {
    if( IsActive() ) //Do not display after launching.
      return;
      
    int tmp = projectile->GetTotalTimeout();
    std::ostringstream ss;
    ss << tmp;
    ss << "s";
    int txt_x = ActiveCharacter().GetX() + ActiveCharacter().GetWidth() / 2;
    int txt_y = ActiveCharacter().GetY() - ActiveCharacter().GetHeight();
    (*Font::GetInstance(Font::FONT_SMALL)).WriteCenterTop( Point2i(txt_x, txt_y) - camera.GetPosition(),
    ss.str(), white_color);
  }
  
  Weapon::Draw();
}

void WeaponLauncher::p_Select()
{
  if (projectile->change_timeout_allowed())
  {
    force_override_keys = true; //Allow overriding key during movement.
    projectile->ResetTimeOut(); 
  }
}

void WeaponLauncher::p_Deselect()
{
  if (projectile->change_timeout_allowed())
  {
    force_override_keys = false;
  }
}

void WeaponLauncher::HandleKeyEvent(int action, int event_type)
{
  if (event_type == KEY_RELEASED)
    switch (action) {
      case ACTION_WEAPON_1:
        projectile->SetTimeOut(1);
        break;
      case ACTION_WEAPON_2:
        projectile->SetTimeOut(2);
        break;
      case ACTION_WEAPON_3:
        projectile->SetTimeOut(3);
        break;
      case ACTION_WEAPON_4:
        projectile->SetTimeOut(4);
        break;
      case ACTION_WEAPON_5:
        projectile->SetTimeOut(5);
        break;
      case ACTION_WEAPON_6:
        projectile->SetTimeOut(6);
        break;
      case ACTION_WEAPON_7:
        projectile->SetTimeOut(7);
        break;
      case ACTION_WEAPON_8:
        projectile->SetTimeOut(8);
        break;
      case ACTION_WEAPON_9:
        projectile->SetTimeOut(9);
        break;

      case ACTION_WEAPON_MORE:  
        projectile->IncrementTimeOut();
        break ;

      case ACTION_WEAPON_LESS:  
        projectile->DecrementTimeOut();
        break   ;

      default:
        break ;     

    };

    if((action >= ACTION_WEAPON_1 && action <= ACTION_WEAPON_9)
        || action == ACTION_WEAPON_MORE || action == ACTION_WEAPON_LESS)
      ActionHandler::GetInstance()->NewAction(new Action(ACTION_SET_TIMEOUT, projectile->m_timeout_modifier));

    ActiveCharacter().HandleKeyEvent(action, event_type);
}

void WeaponLauncher::ActionUp(){ //called by mousse.cpp when mousewhellup 
  projectile->IncrementTimeOut();
}


void WeaponLauncher::ActionDown(){//called by mousse.cpp when mousewhelldown
  projectile->DecrementTimeOut();
}

ExplosiveWeaponConfig& WeaponLauncher::cfg()
{ return static_cast<ExplosiveWeaponConfig&>(*extra_params); }
