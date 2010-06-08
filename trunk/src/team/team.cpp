/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 * A team
 *****************************************************************************/

#include "ai/ai_stupid_player.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "character/character.h"
#include "character/body_list.h"
#include "game/config.h"
#include "game/game.h"
#include "game/game_mode.h"
#include "graphic/sprite.h"
#include "interface/cursor.h"
#include "include/base.h"
#include "include/constant.h"
#include "map/camera.h"
#include "map/map.h"
#include "network/network.h"
#include "sound/jukebox.h"
#include "team/custom_team.h"
#include <WORMUX_debug.h>
#include <WORMUX_file_tools.h>
#include <WORMUX_point.h>
#include "tool/resource_manager.h"
#include "tool/xml_document.h"
#include "weapon/weapons_list.h"
#include <sstream>
#include <iostream>

Team* Team::LoadTeam(const std::string &teams_dir, const std::string &id, std::string& error)
{
  std::string nomfich = teams_dir+id+ PATH_SEPARATOR "team.xml";
  std::string name;
  XmlReader   doc;

  // Load XML
  if (!doc.Load(nomfich)) {
    error = "Unable to load file of team data";
    return NULL;
  }

  if (!XmlReader::ReadString(doc.GetRoot(), "name", name)) {
    error = "Invalid file structure: cannot find a name for team";
    return NULL;
  }

  Profile *res = GetResourceManager().LoadXMLProfile(nomfich, true);
  if (!res) {
    error = "Invalid file structure: cannot load resources";
    return NULL;
  }

  // The constructor will unload res
  return new Team(doc, res, name, teams_dir, id);
}

Team::Team(XmlReader& doc, Profile* res,
           const std::string& name, const std::string &teams_dir, const std::string &id)
  : m_teams_dir(teams_dir)
  , m_id(id)
  , m_name(name)
  , m_player_name("")
  , active_weapon(NULL)
  , ai(NULL)
  , ai_name(NO_AI_NAME)
  , remote(false)
  , abandoned(false)
  , attached_custom_team(NULL)
  , energy(this)
{
  // Load flag
  flag = GetResourceManager().LoadImage(res, "flag");
  mini_flag = flag.RotoZoom(0.0, 0.5, 0.5, true);
  death_flag = GetResourceManager().LoadImage(res, "death_flag");
  big_flag = GetResourceManager().LoadImage(res, "big_flag");
  GetResourceManager().UnLoadXMLProfile(res);

  // Get sound profile
  if (!XmlReader::ReadString(doc.GetRoot(), "sound_profile", m_sound_profile))
    m_sound_profile = "default";

  active_character = characters.end();
  nb_characters = GameMode::GetInstance()->nb_characters;
}

bool Team::LoadCharacters()
{
  ASSERT(characters.size() == 0);
  ASSERT(nb_characters <= 10);

  std::string nomfich = m_teams_dir+m_id+ PATH_SEPARATOR "team.xml";
  // Load XML
  if (!DoesFileExist(nomfich))
    return false;

  XmlReader doc;
  if (!doc.Load(nomfich))
    return false;

  // Create the characters
  xmlNodeArray nodes = XmlReader::GetNamedChildren(XmlReader::GetMarker(doc.GetRoot(), "team"), "character");
  xmlNodeArray::const_iterator it = nodes.begin();

  active_character = characters.end();
  do
  {
    Body* body;
    std::string character_name = "Unknown Soldier (it's all over)";
    std::string body_name = "";
    XmlReader::ReadStringAttr(*it, "name", character_name);
    XmlReader::ReadStringAttr(*it, "body", body_name);

    if (!(body = BodyList::GetRef().GetBody(body_name)) )
    {
      std::cerr
          << Format(_("Error: can't find the body \"%s\" for the team \"%s\"."),
                    body_name.c_str(),
                    m_name.c_str())
          << std::endl;
      return false;
    }

    // Create a new character and add him to the team
    Character new_character(*this, character_name, body);
    if((attached_custom_team != NULL) && (IsLocalHuman()) && !Network::IsConnected())
    {
      new_character.SetCustomName(attached_custom_team->GetCharactersNameList().at(characters.size()));
    }
    characters.push_back(new_character);
    active_character = characters.begin(); // we need active_character to be initialized here !!
    if (!characters.back().PutRandomly(false, GetWorld().GetDistanceBetweenCharacters()))
    {
      // We haven't found any place to put the characters!!
      if (!characters.back().PutRandomly(false, GetWorld().GetDistanceBetweenCharacters() / 2)) {
        std::cerr << std::endl;
        std::cerr << "Error: player " << character_name.c_str() << " will be probably misplaced!" << std::endl;
        std::cerr << std::endl;

        // Put it with no space...
        characters.back().PutRandomly(false, 0);
      }
    }
    characters.back().Init();

    MSG_DEBUG("team", "Add %s in team %s", character_name.c_str(), m_name.c_str());

    // Did we reach the end ?
    ++it;
  } while (it != nodes.end() && characters.size() < nb_characters );

  active_character = characters.begin();

  return (characters.size() == nb_characters);
}

void Team::InitEnergy (uint max)
{
  energy.Config(ReadEnergy(), max);
}

uint Team::ReadEnergy () const
{
  uint total_energy = 0;

  const_iterator it = characters.begin(), end = characters.end();

  for (; it != end; ++it) {
    if ( !(*it).IsDead() )
      total_energy += (*it).GetEnergy();
  }

  return total_energy;
}

void Team::UpdateEnergyBar ()
{
  energy.SetValue(ReadEnergy());
}

void Team::SelectCharacter(const Character * c)
{
  ASSERT(c != NULL);

  if (!c->IsActiveCharacter()) {
    ActiveCharacter().StopPlaying();

    active_character = characters.begin();
    while (!c->IsActiveCharacter() && active_character != characters.end())
      active_character++;

    ASSERT(active_character != characters.end());
  }

  // StartPlaying (if needed) even if c was already ActiveCharacter() thanks to
  // the team change...
  ActiveCharacter().StartPlaying();
}

void Team::NextCharacter(bool newturn)
{
  ASSERT (0 < NbAliveCharacter());

  ActiveCharacter().StopPlaying();

  // we change character:
  // - if user asked so
  // - if it's a new turn and game mode requests a change of character
  if (!newturn || GameMode::GetInstance()->auto_change_character) {

    do {
      ++active_character;
      if (active_character == characters.end())
  active_character = characters.begin();
    } while (ActiveCharacter().IsDead());
  }
  ActiveCharacter().StartPlaying();

  Camera::GetInstance()->CenterOnActiveCharacter();

  MSG_DEBUG("team", "%s (%d, %d)is now the active character",
            ActiveCharacter().GetName().c_str(),
            ActiveCharacter().GetX(),
            ActiveCharacter().GetY());
}

void Team::PreviousCharacter()
{
  ASSERT (0 < NbAliveCharacter());
  ActiveCharacter().StopPlaying();
  do
  {
    if (active_character == characters.begin())
      active_character = characters.end();
    --active_character;
  } while (ActiveCharacter().IsDead());

  ActiveCharacter().StartPlaying();

  Camera::GetInstance()->FollowObject(&ActiveCharacter());
  MSG_DEBUG("team", "%s (%d, %d)is now the active character",
            ActiveCharacter().GetName().c_str(),
            ActiveCharacter().GetX(),
            ActiveCharacter().GetY());
}

int Team::NbAliveCharacter() const
{
  uint nbr = 0;
  const_iterator it= characters.begin(), end=characters.end();

  for (; it!=end; ++it)
    if ( !(*it).IsDead() ) nbr++;

  return nbr;
}

// Prepare a new team turn
void Team::PrepareTurn()
{
  current_turn++;

  // Get a living character if possible
  if (ActiveCharacter().IsDead()) {
    NextCharacter();
  }

  Camera::GetInstance()->FollowObject(&ActiveCharacter(),true);
  CharacterCursor::GetInstance()->FollowActiveCharacter();

  // Updating weapon ammos (some weapons are not available from the beginning)
  std::list<Weapon *> l_weapons_list = weapons_list->GetList() ;
  std::list<Weapon *>::iterator itw = l_weapons_list.begin(),
  end = l_weapons_list.end();
  for (; itw != end ; ++itw) {
    if ((*itw)->AvailableAfterTurn() == (int)current_turn) {
      // this weapon is available now
      m_nb_ammos[ (*itw)->GetType() ] += (*itw)->ReadInitialNbAmmo();
      m_nb_units[ (*itw)->GetType() ] += (*itw)->ReadInitialNbUnit();
    }
  }

  // Active last weapon use if EnoughAmmo
  if (AccessWeapon().EnoughAmmo())
    AccessWeapon().Select();
  else { // try to find another weapon !!
    active_weapon = weapons_list->GetWeapon(Weapon::WEAPON_BAZOOKA);
    AccessWeapon().Select();
  }

  // Sound the bell, so the local players know when it is their turn
  if (IsLocalHuman())
    JukeBox::GetInstance()->Play("default", "start_turn");
  if (ai != NULL)
    ai->PrepareTurn();
}

Character& Team::ActiveCharacter() const
{
  return (*active_character);
}

void Team::SetWeapon (Weapon::Weapon_type type)
{

  ASSERT (type >= Weapon::FIRST && type <= Weapon::LAST);
  AccessWeapon().Deselect();
  active_weapon = weapons_list->GetWeapon(type);
  AccessWeapon().Select();
}

int Team::ReadNbAmmos() const
{
  return ReadNbAmmos(active_weapon->GetType());
}

int Team::ReadNbUnits() const
{
  return ReadNbUnits( active_weapon->GetType());
}

int Team::ReadNbAmmos(const Weapon::Weapon_type &weapon_type) const
{
  ASSERT((unsigned int)weapon_type < m_nb_ammos.size());
  return m_nb_ammos[weapon_type];
}

int Team::ReadNbUnits(const Weapon::Weapon_type &weapon_type) const
{
  ASSERT((unsigned int)weapon_type < m_nb_units.size());
  return m_nb_units[weapon_type];
}

int& Team::AccessNbAmmos()
{
  // if value not initialized, it initialize to 0 and then return 0
  return m_nb_ammos[ active_weapon->GetType() ] ;
}

int& Team::AccessNbUnits()
{
  // if value not initialized, it initialize to 0 and then return 0
  return m_nb_units[ active_weapon->GetType() ] ;
}

void Team::ResetNbUnits()
{
  m_nb_units[ active_weapon->GetType() ] = active_weapon->ReadInitialNbUnit();
}

Team::iterator Team::begin() { return characters.begin(); }
Team::iterator Team::end() { return characters.end(); }

Character* Team::FindByIndex(uint index)
{
  ASSERT(index < characters.size());
  iterator it= characters.begin(), end=characters.end();

  while(index != 0 && it != end)
  {
    index--;
    it++;
  }
  return &(*it);
}

void Team::LoadGamingData(WeaponsList * weapons)
{
  weapons_list = weapons;
  current_turn = 0;

  // Reset ammos
  m_nb_ammos.clear();
  m_nb_units.clear();
  std::list<Weapon *> l_weapons_list = weapons_list->GetList() ;
  std::list<Weapon *>::iterator itw = l_weapons_list.begin(),
  end = l_weapons_list.end();

  m_nb_ammos.assign(l_weapons_list.size(), 0);
  m_nb_units.assign(l_weapons_list.size(), 0);

  for (; itw != end ; ++itw) {
    if ((*itw)->AvailableAfterTurn() == 0) {
      // this weapon is available now
      m_nb_ammos[ (*itw)->GetType() ] = (*itw)->ReadInitialNbAmmo();
      m_nb_units[ (*itw)->GetType() ] = (*itw)->ReadInitialNbUnit();
    } else {
      // this weapon will be available later
      m_nb_ammos[ (*itw)->GetType() ] = 0;
      m_nb_units[ (*itw)->GetType() ] = 0;
    }
  }

  // Disable non-working weapons in network games
  if(Network::GetInstance()->IsConnected())
  {
    //m_nb_ammos[ Weapon::WEAPON_GRAPPLE ] = 0;
  }

  active_weapon = weapons_list->GetWeapon(Weapon::WEAPON_DYNAMITE);

  abandoned = false;
  LoadCharacters();
}

void Team::UnloadGamingData()
{
  // Clear list of characters
  characters.clear();
  if (ai) {
    delete ai;
    ai = NULL;
  }
  weapons_list = NULL;
}

void Team::LoadAI()
{
  ASSERT(IsLocalAI());
  if (ai) {
    delete ai;
  }
  ai = new AIStupidPlayer(this);
}

void Team::SetNbCharacters(uint howmany)
{
  ASSERT(howmany >= 1 && howmany <= 10);
  nb_characters = howmany;
}

void Team::DrawEnergy(const Point2i& pos)
{
  energy.Draw(pos);
}

void Team::Refresh()
{
  energy.Refresh();
}

void Team::RefreshAI()
{
  if (ai != NULL)
    ai->Refresh();
}

Weapon& Team::AccessWeapon() const { return *active_weapon; }
const Weapon& Team::GetWeapon() const { return *active_weapon; }
Weapon::Weapon_type Team::GetWeaponType() const { return GetWeapon().GetType(); }

bool Team::IsSameAs(const Team& other) const
{
  return (strcmp(m_id.c_str(), other.GetId().c_str()) == 0);
}

bool Team::IsActiveTeam() const
{
  return this == &ActiveTeam();
}

void Team::SetDefaultPlayingConfig()
{
  SetRemote(false);
  SetPlayerName("");
  SetNbCharacters(GameMode::GetInstance()->nb_characters);
  SetAIName(NO_AI_NAME);
}

void Team::AttachCustomTeam(CustomTeam *custom_team)
{
 attached_custom_team = custom_team;
}

