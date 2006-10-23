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
 * Base classes for weapons configuration.
 *****************************************************************************/

#include "weapon_cfg.h"
#include <iostream>
//-----------------------------------------------------------------------------

void EmptyWeaponConfig::LoadXml(xmlpp::Element *elem) 
{}

//-----------------------------------------------------------------------------

WeaponConfig::WeaponConfig() 
{ damage = 10; }

void WeaponConfig::LoadXml(xmlpp::Element *elem) 
{
  LitDocXml::LitUint (elem, "damage", damage);  
}

//-----------------------------------------------------------------------------

ExplosiveWeaponConfig::ExplosiveWeaponConfig()
{
  timeout = 0; 
  allow_change_timeout = false;
  explosion_range = 0 ;
  particle_range = explosion_range;
  blast_range = 0 ;
  blast_force = 0 ;
}

void ExplosiveWeaponConfig::LoadXml(xmlpp::Element *elem) 
{
  WeaponConfig::LoadXml (elem);
  LitDocXml::LitUint (elem, "timeout", timeout);
  LitDocXml::LitBool (elem, "allow_change_timeout", allow_change_timeout);
  LitDocXml::LitUint (elem, "explosion_range", explosion_range);
  LitDocXml::LitUint (elem, "particle_range", particle_range);
  LitDocXml::LitDouble (elem, "blast_range", blast_range);
  LitDocXml::LitDouble (elem, "blast_force", blast_force);
}

//-----------------------------------------------------------------------------

// timeout before explosion
// max_strength
// min_angle, max_angle
// coeff rebond ?
// nb_replay
// can_change_weapon
// force, longueur (baseball)
// nbr_obus
