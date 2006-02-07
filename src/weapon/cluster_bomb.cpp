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
 * Arme grenade à fragmentation(cluster bomb)
 * Explose au bout de quelques secondes 
 *****************************************************************************/

#include "cluster_bomb.h"
#include <sstream>
#include <math.h>
#include "weapon_tools.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../graphic/video.h"
#include "../interface/game_msg.h"
#include "../interface/interface.h"
#include "../include/global.h"
#include "../map/camera.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/math_tools.h"
#include "../tool/i18n.h"
#include "../tool/random.h"

#ifdef DEBUG
#define COUT_DBG std::cout << "[ClusterBomb] "
#endif

Cluster::Cluster(GameLoop &p_game_loop, ClusterLauncher& p_launcher) :
  WeaponProjectile (p_game_loop, "Cluster"),
  launcher(p_launcher)
{
  m_allow_negative_y = true;
  touche_ver_objet = true;
}

void Cluster::Tire (int x, int y)
{
  PrepareTir();
  SetXY(x,y);
}

void Cluster::Init()
{
  image = resource_manager.LoadSprite( weapons_res_profile, "cluster");
  image->EnableRotationCache(32);
  SetSize (image->GetWidth(), image->GetHeight()); 

  SetMass (launcher.cfg().mass / (launcher.cfg().nbr_fragments+1));
  SetAirResistFactor(launcher.cfg().air_resist_factor);

  int dx = image->GetWidth()/2-1;
  int dy = image->GetHeight()/2-1;
  SetTestRect (dx, dx, dy, dy);
}

void Cluster::Refresh()
{
  if (!is_active) return;

  double angle = GetSpeedAngle() * 180/M_PI ;

  image->SetRotation_deg( angle);
}

void Cluster::Draw()
{
  image->Draw(GetX(), GetY());
}

void Cluster::SignalCollision()
{
  is_active = false;
  lst_objets.RetireObjet((PhysicalObj*)this);
  if (IsGhost())
  {
    game_messages.Add (_("The rocket left the battlefield..."));
    return;
  }
  AppliqueExplosion (GetPos(), GetPos(), launcher.impact, launcher.cfg(), NULL);
}

//-----------------------------------------------------------------------------

ClusterBomb::ClusterBomb(GameLoop &p_game_loop, ClusterLauncher& p_launcher) :
  WeaponProjectile (p_game_loop, "cluster_bomb"),
  launcher(p_launcher)
{
  m_allow_negative_y = true;
  m_rebound_sound = "weapon/grenade_bounce";
  touche_ver_objet = false;
  m_rebounding = true;
}

void ClusterBomb::Tire (double force)
{
  SetAirResistFactor(launcher.cfg().air_resist_factor);

  PrepareTir();

  // Set the initial position.
  int x,y;
  ActiveCharacter().GetHandPosition(x, y);
  SetXY (x,y);

  // Set the initial speed
  double angle = ActiveTeam().crosshair.GetAngleRad();
  SetSpeed (force, angle);

  PutOutOfGround(angle);
#ifdef MSG_DBG
  COUT_DBG << "ClusterBomb::Tire()" << std::endl;
#endif

  // Recupere le moment du départ
  temps_debut_tir = global_time.Read();
}

void ClusterBomb::Init()
{
  image = resource_manager.LoadSprite( weapons_res_profile, "clusterbomb_sprite");
  image->EnableRotationCache(32);
  SetSize (image->GetWidth(), image->GetHeight()); 

  SetMass (launcher.cfg().mass);
  m_rebound_factor = launcher.cfg().rebound_factor;

  int dx = image->GetWidth()/2-1;
  int dy = image->GetHeight()/2-1;
  SetTestRect (dx, dx, dy, dy);

#ifdef MSG_DBG
  COUT_DBG << "ClusterBomb::Init()" << std::endl;
#endif


  tableau_cluster.clear();
  const uint nb = launcher.cfg().nbr_fragments;
  for (uint i=0; i<nb; ++i)
  {
    Cluster cluster(game_loop, launcher);
    cluster.Init();
    tableau_cluster.push_back( cluster );
  }
}

void ClusterBomb::Refresh()
{
  if (!is_active) return;

#ifdef MSG_DBG
  COUT_DBG << "ClusterBomb::Refresh()" << std::endl;
#endif

  if (IsGhost())
  {
    game_messages.Add ("The Cluster Bomb left the battlefield before it could explode.");
    is_active = false;
    return;
  }

  // 5 sec après avoir été tirée, la grenade explose
  double tmp = global_time.Read() - temps_debut_tir;
  if(tmp>1000 * launcher.cfg().tps_avt_explosion)
  {
    DoubleVector speed_vector ;
    int x, y ;

    GetSpeedXY(speed_vector);

    iterator it=tableau_cluster.begin(), end=tableau_cluster.end();
    for (; it != end; ++it)
    {
      Cluster &cluster = *it;
      
      double angle = (double)RandomLong ((long)0.0, (long)(2.0 * M_PI));
      x = GetX()+(int)(cos(angle) * (double)launcher.cfg().blast_range*5);
      y = GetY()+(int)(sin(angle) * (double)launcher.cfg().blast_range*5);

      cluster.Tire(x,y);
      cluster.SetSpeedXY(speed_vector);
      lst_objets.AjouteObjet((PhysicalObj*)&cluster,false);
    }
    is_active = false;
    return;
  }

  if (TestImpact()) { SignalCollision(); return; }

  // rotation de l'image de la grenade...
  double angle = GetSpeedAngle() * 180/M_PI ;
  image->SetRotation_deg( angle);
}

void ClusterBomb::Draw()
{
  if (!is_active) return;
  if (IsGhost())
  {
    game_messages.Add ("The Cluster Bomb left the battlefield before it could explode.");
    is_active = false;
    return;
  }

  image->Draw(GetX(), GetY());

  int tmp = launcher.cfg().tps_avt_explosion;
  tmp -= (int)((global_time.Read() - temps_debut_tir) / 1000);
  std::ostringstream ss;
  ss << tmp;
  int txt_x = GetX() + GetWidth() / 2;
  int txt_y = GetY() - GetHeight();

  global().small_font().WriteCenterTop (txt_x-camera.GetX(), txt_y-camera.GetY(), ss.str(), white_color);
}

void ClusterBomb::SignalCollision()
{   
  if (IsGhost())
  {
    game_messages.Add ("The Cluster Bomb left the battlefield before it could explode.");
    is_active = false ;
  }
}

//-----------------------------------------------------------------------------

ClusterLauncher::ClusterLauncher() : 
  Weapon(WEAPON_CLUSTER_BOMB, "cluster_bomb", VISIBLE_ONLY_WHEN_INACTIVE),
  cluster_bomb(game_loop, *this)
{  
  m_name = _("ClusterBomb");  

  extra_params = new ClusterBombConfig();  

  cluster_bomb.Init();
  impact = resource_manager.LoadImage( weapons_res_profile, "grenade_impact");
}

bool ClusterLauncher::p_Shoot ()
{
  // Initialise la grenade
  cluster_bomb.Tire (m_strength);
  camera.ChangeObjSuivi (&cluster_bomb, true, false);
  lst_objets.AjouteObjet (&cluster_bomb, true);

  jukebox.Play(ActiveTeam().GetSoundProfile(), "fire");  

  return true;
}

void ClusterLauncher::Explosion()
{
  m_is_active = false;

#ifdef MSG_DBG
  COUT_DBG << "ClusterLauncher::Explosion()" << std::endl;
#endif
  
  lst_objets.RetireObjet (&cluster_bomb);

  // On fait un trou ?
  if (cluster_bomb.IsGhost()) return;

  // Applique les degats et le souffle aux vers
  Point2i pos = cluster_bomb.GetCenter();
  AppliqueExplosion (pos, pos, impact, cfg(), NULL);
}

void ClusterLauncher::Refresh()
{
  if (m_is_active)
  {
    if (!cluster_bomb.is_active) Explosion();
  } 
}

ClusterBombConfig& ClusterLauncher::cfg() 
{ return static_cast<ClusterBombConfig&>(*extra_params); }

//-----------------------------------------------------------------------------

ClusterBombConfig::ClusterBombConfig()
{
  tps_avt_explosion = 10;
  rebound_factor = 0.75;
  nbr_fragments = 5;
}


void ClusterBombConfig::LoadXml(xmlpp::Element *elem)
{
  ExplosiveWeaponConfig::LoadXml(elem);
  LitDocXml::LitUint (elem, "tps_avt_explosion", tps_avt_explosion);
  LitDocXml::LitDouble (elem, "rebound_factor", rebound_factor);
  LitDocXml::LitUint (elem, "nbr_fragments", nbr_fragments);
}

