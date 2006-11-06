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
 * A team.
 *****************************************************************************/

#ifndef TEAM_H
#define TEAM_H

#include <list>
#include <map>
#include <string>
#include "team_energy.h"
#include "../character/character.h"
#include "../graphic/surface.h"
#include "../include/enum.h"
#include "../include/base.h"
#include "../weapon/crosshair.h"
#include "../weapon/weapon.h"
#include "../tool/point.h"
#include "../tool/resource_manager.h"

class Character;
class Weapon;

typedef enum {
  TEAM_human_local,
  TEAM_ai_local,
  TEAM_remote
} team_player_type_t;

class Team
{
  public:
    typedef std::list<Character>::iterator iterator;
    typedef std::list<Character>::const_iterator const_iterator;

    std::map<std::string, int> m_nb_ammos;
    std::map<std::string, int> m_nb_units;

  // Autres
    CrossHair crosshair;
    Surface flag;
    Point2i sauve_camera;
    TeamEnergy energy;

  private:
    bool is_camera_saved;
    std::string m_teams_dir; // parent directory hosting the data
    std::string m_id;
    std::string m_name;
    std::string m_player_name;
    std::string m_sound_profile;
    std::list<Character> characters;
    iterator active_character;
    Weapon *active_weapon;
    uint nb_characters;

    team_player_type_t type_of_player; 

    Team (const std::string& _teams_dir,
          const std::string& _id,
          const std::string& _name,
          const Surface &_flag,
          const std::string& _sound_profile);

    bool LoadCharacters(uint howmany);
  public:
    static Team* CreateTeam (const std::string &teams_dir, const std::string &id);

    void LoadGamingData(uint howmany);
    void UnloadGamingData();

    bool IsSameAs(const Team& other) const;

  // Manage number of characters
    void SetNbCharacters(uint howmany);
    uint GetNbCharacters() const { return nb_characters; }

  // Switch to next worm.
    void NextCharacter();

  // Prepare turn.
    void PrepareTurn();

  // Access to the character.
    Character& ActiveCharacter();

    void DrawEnergy(const Point2i& pos);
    void Refresh();

  // Change the weapon.
    void SetWeapon (Weapon_type nv_arme);
    int NbAliveCharacter() const;

  // Access to the active weapon.
    Weapon& AccessWeapon() const;
    const Weapon& GetWeapon() const;
    Weapon_type GetWeaponType() const;

  // Init the energy bar of the team.
    void InitEnergy (uint max);

  // Update the energy bar values of the team.
    void UpdateEnergyBar();

  // Read the total energy of the team.
    uint ReadEnergy();

  // Access to data.
    const std::string& GetName() const { return m_name; }
    const std::string& GetId() const { return m_id; }
    const std::string& GetPlayerName() const { return m_player_name; }
    const std::string& GetSoundProfile() const { return m_sound_profile; }
    iterator begin();
    iterator end();
    Character* FindByIndex(uint index);

    void SetPlayerName(const std::string& player_name);

  // Number of ammo for the current selected weapon.
  // (return INFINITE_AMMO is ammo are unlimited !)
    int ReadNbAmmos() const;
    int ReadNbAmmos(const std::string &weapon_name) const;
    int& AccessNbAmmos();


  // Number of current unit per ammo for the selected weapon.
    int ReadNbUnits() const;
    int ReadNbUnits(const std::string &weapon_name) const;
    int& AccessNbUnits();
    void ResetNbUnits();

  // Only for network:
  // true if the team belong to a local player
  // false if the team belong to a player on the network or on the AI
    bool IsLocal() const;

  // true if the team belong to a local AI
    bool IsLocalAI() const;

    void SetLocal();
    void SetLocalAI();
    void SetRemote();

};

#endif /* TEAM_H */

