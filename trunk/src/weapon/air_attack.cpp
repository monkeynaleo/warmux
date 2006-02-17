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
 * Air attack.
 *****************************************************************************/

#include "air_attack.h"
#include <sstream>
#include "../game/game_loop.h"
#include "../graphic/sprite.h"
#include "../include/action_handler.h"
#include "../interface/mouse.h"
#include "../map/map.h"
#include "../map/camera.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/random.h"
#include "../tool/i18n.h"
#include "../weapon/weapon_tools.h"

const uint FORCE_X_MIN = 10;
const uint FORCE_X_MAX = 120;
const uint FORCE_Y_MIN = 1;
const uint FORCE_Y_MAX = 40;

const double OBUS_SPEED = 7 ;

Obus::Obus(GameLoop &p_game_loop, AirAttack& p_air_attack) :
  WeaponProjectile(p_game_loop, "obus", NULL),
  air_attack(p_air_attack)
{
  SetMass (air_attack.cfg().mass);
  SetWindFactor (0.1);
}

void Obus::Refresh()
{
}

void Obus::Reset()
{
  is_active = true;
  Ready();
}

void Obus::SignalCollision()
{ 
  is_active = false; 

  if (IsGhost()) return;

  Point2i pos = GetCenter();

  AppliqueExplosion (pos, pos,
		     impact,
		     air_attack.cfg(),
		     this);
}

//-----------------------------------------------------------------------------

Avion::Avion(GameLoop &p_game_loop, AirAttack& p_air_attack) : 
  PhysicalObj(p_game_loop, "Avion", 0.0),
  air_attack(p_air_attack)
{
  m_type = objUNBREAKABLE;
  SetWindFactor(0.0);
  m_gravity_factor = 0.0;
  m_alive = GHOST;

  image = new Sprite( resource_manager.LoadImage( weapons_res_profile, "air_attack_plane"));
  SetSize (image->GetWidth(), image->GetHeight());   
  SetMass (3000);
  obus_dx = 100;
  obus_dy = 50;
}

void Avion::Reset()
{
  m_alive = GHOST;
}

void Avion::Tire()
{
  DoubleVector speed_vector ;
  int dir = ActiveCharacter().GetDirection();
  cible_x = mouse.GetXmonde();
  SetY (0);

  image->Scale(dir, 1);
   
  Ready();

  if (dir == 1)
    {
      speed_vector.SetValues( air_attack.cfg().speed, 0);
      SetX (-image->GetWidth()+1);
    }
  else
    {
      speed_vector.SetValues( -air_attack.cfg().speed, 0) ;
      SetX (world.GetWidth()-1);
    }

  SetSpeedXY (speed_vector);

  camera.ChangeObjSuivi (this, true, true);
}

void Avion::Refresh()
{
  if (IsGhost()) return;
  UpdatePosition();
}

int Avion::LitCibleX() const { return cible_x; }

int Avion::GetDirection() const { 
  float x,y;
  image->GetScaleFactors(x,y);
  return (x<0)?-1:1;
}

void Avion::Draw()
{
  if (IsGhost()) return;
  image->Draw( GetX(), GetY());
}

bool Avion::PeutLacherObus() const
{
  if (GetDirection() == 1) 
    return (cible_x <= GetX()+obus_dx);
  else
    return (GetX()+(int)image->GetWidth()-obus_dx <= cible_x);
}

//-----------------------------------------------------------------------------

AirAttack::AirAttack() :
  Weapon(WEAPON_AIR_ATTACK, "air_attack",new AirAttackConfig()),
  avion(game_loop, *this)
{  
  m_name = _("Air attack");
  can_be_used_on_closed_map = false;
}

void AirAttack::p_Select()
{
  avion.Reset();
}

void AirAttack::ChooseTarget()
{
  ActiveTeam().GetWeapon().NewActionShoot();
}

bool AirAttack::p_Shoot ()
{
  game_loop.interaction_enabled=false;

  assert (obus.size() == 0);
  avion.Tire ();
  obus_laches = false;
  obus_actifs = false;
  
  return true;
}

void AirAttack::Refresh()
{
  if (!m_is_active) return;

  // L'avion est arriv� au bon endroit ? Largue les obus
  if (!obus_laches && avion.PeutLacherObus())
  {
    obus_laches = true;
    obus_actifs = true;

    int x=avion.LitCibleX();
    Obus *instance;
    for (uint i=0; i<cfg().nbr_obus; ++i) 
    {
      std::ostringstream ss;
      ss.str("");
      ss << "Obus(" << i << ')';
      instance = new Obus(game_loop, *this);
      instance -> m_name = ss.str();
      instance -> Reset();
      instance -> SetXY (x, avion.obus_dy);

      DoubleVector speed_vector ;

      int fx = RandomLong (FORCE_X_MIN, FORCE_X_MAX);
      fx *= avion.GetDirection();
      int fy = RandomLong (FORCE_Y_MIN, FORCE_Y_MAX);

      speed_vector.SetValues( fx/30.0, fy/30.0);
      instance -> SetSpeedXY (speed_vector);
      obus.push_back (instance);
    }

    camera.ChangeObjSuivi (instance, true, true);
  }

  // D�place l'avion
  avion.Refresh();

  if (obus_actifs)
  {
    iterator it=obus.begin(), fin=obus.end();
    for (; it != fin; ++it)
    {
      (**it).Refresh();
      (**it).UpdatePosition();
    }

    // Tous les obus ont touch�s leur cible ?
    it=obus.begin(), fin=obus.end();
    uint nbr_obus_actif = 0;
    for (; it != fin; ++it)
    {
      if ((**it).is_active) nbr_obus_actif++;
    }
    if (!nbr_obus_actif) obus_actifs = false;
  }
  if (!obus_actifs && avion.IsGhost()) FinTir();
}

void AirAttack::FinTir()
{
  m_is_active = false;

  camera.ChangeObjSuivi (NULL, false, false);

  iterator it=obus.begin(), fin=obus.end();
  for (; it != fin; ++it)
  {
    delete *it;
  }
  obus.clear();

  game_loop.interaction_enabled=true;
}

void AirAttack::Draw()
{
  Weapon::Draw() ;
  
  // Dessine les obus
  if (obus_actifs)
  {
    const_iterator it=obus.begin(), fin=obus.end();
    for (; it != fin; ++it) (**it).Draw ();
  }

  avion.Draw ();
}

AirAttackConfig& AirAttack::cfg() 
{ return static_cast<AirAttackConfig&>(*extra_params); }

//-----------------------------------------------------------------------------

AirAttackConfig::AirAttackConfig()
{
  nbr_obus = 3;
}

void AirAttackConfig::LoadXml(xmlpp::Element *elem)
{
  ExplosiveWeaponConfig::LoadXml(elem);
  LitDocXml::LitUint (elem, "nbr_obus", nbr_obus);
  LitDocXml::LitDouble (elem, "speed", speed);
}

