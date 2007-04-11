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
 * Refresh des armes.
 *****************************************************************************/

#include "weapons_list.h"
//-----------------------------------------------------------------------------
#include <algorithm>
#include "all.h"
#include "explosion.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../interface/interface.h"
#include "../object/objects_list.h"
#include "../map/camera.h"
#include "../team/macro.h"
#include "../map/maps_list.h"
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

WeaponsList * weapon_list = NULL;

WeaponsList * WeaponsList::GetInstance()
{
  if (weapon_list == NULL) {
    weapon_list = new WeaponsList();
  }
  weapon_list->ref_counter++;
  return weapon_list;
}

WeaponsList::~WeaponsList()
{
  ref_counter--;
  /* we can delete the list iif nobody has an instance somewhere */
  if (ref_counter == 0)
    {
      weapons_list_it it=m_weapons_list.begin(), end=m_weapons_list.end();
      for (; it != end; ++it)
        {
          delete *it;
        }
      weapon_list = NULL;
    }
}

//-----------------------------------------------------------------------------

void WeaponsList::AddToList(Weapon* weapon, uint num_sort)
{
  // insert the pointer
  m_weapons_list.push_back(weapon);

  m_weapons_map.insert(keybind(num_sort, weapon));

  Interface::GetInstance()->weapons_menu.AddWeapon(weapon, num_sort);
}

//-----------------------------------------------------------------------------

WeaponsList::WeaponsList()
{
  weapons_res_profile = resource_manager.LoadXMLProfile( "weapons.xml", false);
  Bazooka* bazooka = new Bazooka;
  SubMachineGun* submachine_gun = new SubMachineGun;
  Gun* gun = new Gun;
  Shotgun* shotgun = new Shotgun;
  SnipeRifle* snipe_rifle = new SnipeRifle;
  RiotBomb* riot_bomb = new RiotBomb;
  AutomaticBazooka* auto_bazooka = new AutomaticBazooka;
  Dynamite* dynamite = new Dynamite;
  GrenadeLauncher* grenade_launcher = new GrenadeLauncher;
  DiscoGrenadeLauncher* disco_grenade_launcher = new DiscoGrenadeLauncher;
  ClusterLauncher* cluster_launcher = new ClusterLauncher;
  Baseball* baseball = new Baseball;
  Mine* mine = new Mine;
  AirAttack* air_attack = new AirAttack;
  AnvilLauncher* anvil = new AnvilLauncher;
  TuxLauncher* tux = new TuxLauncher;
  GnuLauncher* gnu_launcher = new GnuLauncher;
  PolecatLauncher* polecat_launcher = new PolecatLauncher;
  BounceBallLauncher* bounce_ball_launcher = new BounceBallLauncher;
  Teleportation* teleportation = new Teleportation;
  Parachute* parachute = new Parachute;
  Suicide* suicide = new Suicide;
  SkipTurn* skipturn = new SkipTurn;
  JetPack* jetpack = new JetPack;
  Airhammer* airhammer = new Airhammer;
  Construct* construct = new Construct;
  LowGrav* lowgrav = new LowGrav;
  NinjaRope* ninjarope = new NinjaRope;
  Blowtorch* blowtorch = new Blowtorch;
  Syringe* syringe = new Syringe;

  int i = 1;
  // Category 1
  AddToList(bazooka, i);
  AddToList(riot_bomb, i);
  AddToList(auto_bazooka, i);
  AddToList(air_attack, i);

  // Category 2
  i++;
  AddToList(submachine_gun, i);
  AddToList(gun, i);
  AddToList(shotgun, i);
  AddToList(snipe_rifle, i);

  // Category 3
  i++;
  AddToList(cluster_launcher, i);
  AddToList(grenade_launcher, i);
  AddToList(disco_grenade_launcher, i);
  AddToList(dynamite, i);
  AddToList(mine, i);

  // Category 4
  i++;
  AddToList(gnu_launcher, i);
  AddToList(tux, i);
  AddToList(polecat_launcher, i);

  // Category 5
  i++;
  AddToList(baseball, i);
  AddToList(bounce_ball_launcher, i);
  AddToList(anvil, i);
  AddToList(syringe, i);
  AddToList(suicide, i);

  // Category 6
  i++;
  AddToList(ninjarope, i);
  AddToList(jetpack, i);
  AddToList(parachute, i);
  AddToList(teleportation, i);
  AddToList(lowgrav, i);

  // Category 7
  i++;
  AddToList(skipturn,i);
  AddToList(airhammer,i);
  AddToList(construct,i);
  AddToList(blowtorch,i);

  ref_counter = 0;
}

//-----------------------------------------------------------------------------

void WeaponsList::Refresh ()
{
  ActiveTeam().AccessWeapon().Manage();
}

//-----------------------------------------------------------------------------

WeaponsList::weapons_list_type& WeaponsList::GetList()
{
  return m_weapons_list;
}

//-----------------------------------------------------------------------------

Weapon* WeaponsList::GetNextWeapon(uint sort, uint index)
{
	uint n = m_weapons_map.count(sort);
	index = index % n;  // index can't be greater than number of weapons
	weapons_map_it it = m_weapons_map.lower_bound(sort);

	if(index + 1 < n)
	{
		for(uint i=0; i < index + 1; i++)
			it++;
	}

	return it->second;
}

//-----------------------------------------------------------------------------

bool WeaponsList::GetWeaponBySort(uint sort, Weapon::Weapon_type &type)
{
  uint nb_weapons = m_weapons_map.count(sort);
  if (nb_weapons == 0) return false;

  // One or many weapons on this key
  std::pair<weapons_map_it, weapons_map_it> p = m_weapons_map.equal_range(sort);
  weapons_map_it it = p.first, end = p.second;

  Weapon* next_weapon = it->second;

  if (nb_weapons > 1)
    {
    	// Find index of current weapon
    	uint current_weapon;
    	for(current_weapon=0; current_weapon < nb_weapons-1; current_weapon++, it++)
	{
		if(it->second == &(ActiveTeam().GetWeapon()))
			break;
	}

	// Get next weapon that has enough ammo and can be used on the map
	uint i = 0;
	do
	{
		next_weapon = GetNextWeapon(sort, current_weapon++);
	} while(i++ < nb_weapons && (ActiveTeam().ReadNbAmmos(next_weapon->GetName()) == 0 || !(next_weapon->CanBeUsedOnClosedMap() || ActiveMap().IsOpened())));
	// this corresponds to:  while (stop-condition && (not-enoughammo || not-usable-on-map))

	// no right weapon has been found
	if(i > nb_weapons)
		return false;
    }

  type = next_weapon->GetType();
  return true;
}

//-----------------------------------------------------------------------------

class test_weapon_type {
  private:
    Weapon::Weapon_type m_type;
  public:
    test_weapon_type(Weapon::Weapon_type &type) { m_type = type; }
	bool operator() ( Weapon* w) { return w->GetType()==m_type; }
};

Weapon* WeaponsList::GetWeapon (Weapon::Weapon_type type)
{
  weapons_list_it it;
  it = std::find_if(m_weapons_list.begin(), m_weapons_list.end(), test_weapon_type(type));
  assert (it != m_weapons_list.end());
  return *it;
}

//-----------------------------------------------------------------------------
