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
 * Une équipe de vers.
 *****************************************************************************/

#include "team.h"
#include "../game/game.h"
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../interface/cursor.h"
#include "../include/constant.h"
#include "../game/config.h"
#include "../map/camera.h"
#include "../weapon/weapons_list.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../tool/resource_manager.h"
#include "../graphic/sprite.h"
#include <sstream>
#include <iostream>

Team::Team(const std::string& _teams_dir,
	   const std::string& _id, 
	   const std::string& _name, 
	   const Surface& _flag, 
	   const std::string& _sound_profile)
{
  is_local = true;
  active_character = NULL;

  camera_est_sauve = false;
  active_weapon = weapons_list.GetWeapon(WEAPON_DYNAMITE);

  m_teams_dir = _teams_dir;
  m_id = _id;
  m_name = _name;
  m_sound_profile = _sound_profile;

  flag = _flag;
  crosshair.Init();
}

Team * Team::CreateTeam (const std::string& teams_dir, 
			 const std::string& id)
{
  std::string nomfich;
  try
  {
    LitDocXml doc;

    // Load XML
    nomfich = teams_dir+id+PATH_SEPARATOR+ "team.xml";
    if (!IsFileExist(nomfich)) return false;
    if (!doc.Charge (nomfich)) return false;

    // Load name
    std::string name;
    if (!LitDocXml::LitString(doc.racine(), "name", name)) return NULL;

    // Load flag
    Profile *res = resource_manager.LoadXMLProfile( nomfich, true);
    Surface flag = resource_manager.LoadImage(res, "flag");

    // Get sound profile
    std::string sound_profile;
    if (!LitDocXml::LitString(doc.racine(), "sound_profile", sound_profile))
      sound_profile = "default";

    return new Team(teams_dir, id, name, flag, sound_profile) ;
  }
  catch (const xmlpp::exception &e)
  {
    std::cerr << std::endl
              << Format(_("Error loading team %s:"), id.c_str())
              << std::endl << e.what() << std::endl;
    return NULL;
  }

  return NULL;
}


bool Team::LoadCharacters(uint howmany)
{
  assert(howmany <= GameMode::GetInstance()->max_characters);

  std::string nomfich;
  try
  {
    LitDocXml doc;

    // Load XML
    nomfich = m_teams_dir+m_id+PATH_SEPARATOR+ "team.xml";
    if (!IsFileExist(nomfich)) return false;
    if (!doc.Charge (nomfich)) return false;

    // Create the characters
    xmlpp::Element *xml = LitDocXml::AccesBalise (doc.racine(), "team");
    
    xmlpp::Node::NodeList nodes = xml -> get_children("character");
    xmlpp::Node::NodeList::iterator 
      it=nodes.begin(),
      fin=nodes.end();

    characters.clear();
    bool fin_bcl;
    active_character = NULL;
    do
      {
	xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it);
	Skin *skin;
	std::string character_name="Soldat Inconnu";
	std::string skin_name="ver_jaune";
	LitDocXml::LitAttrString(elem, "name", character_name);
	LitDocXml::LitAttrString(elem, "skin", skin_name);
	
	if (skins_list.find(skin_name) != skins_list.end()) {
	  skin = &skins_list[skin_name];
	} else {
	  std::cerr 
	    << Format(_("Error: can't find the skin \"%s\" for the team \"%s\"."),
		      skin_name.c_str(),
		      m_name.c_str()) 
	    << std::endl;
	  return false;
	}
	
	// Initialise les variables du ver, puis l'ajoute à la liste
	Character new_character(*this, character_name, skin);
	characters.push_back(new_character);
	characters.back().Reset();
	
	MSG_DEBUG("team", "Add %s in team %s", character_name.c_str(), m_name.c_str());
	
	// C'est la fin ?
	++it;
	fin_bcl = (it == fin);
	fin_bcl |= (howmany <= characters.size());
      } while (!fin_bcl);
    
    active_character = characters.begin();
    
  }
  catch (const xmlpp::exception &e)
    {
      std::cerr << std::endl
		<< Format(_("Error loading team's data %s:"), m_id.c_str())
		<< std::endl << e.what() << std::endl;
      return false;
    }

  return (characters.size() == howmany);
}

void Team::InitEnergy (uint max)
{
  energy.Init();
  energy.ChoisitNom(m_name);
  energy.FixeMax(max);
  energy.FixeValeur(ReadEnergy());
}

uint Team::ReadEnergy ()
{
  uint total_energy = 0;
  
  iterator it = characters.begin(), end = characters.end();

  for (; it != end; ++it) {
    if ( !(*it).IsDead() )
      total_energy += (*it).GetEnergy();
  }

  return total_energy;
}

void Team::UpdateEnergyBar ()
{
  energy.NouvelleValeur(ReadEnergy());
}

void Team::NextCharacter()
{
  // Passe au ver suivant
  assert (0 < NbAliveCharacter());
  do
  { 
    ++active_character;
    if (active_character == characters.end()) 
      active_character = characters.begin();
  } while (ActiveCharacter().IsDead());
}

int Team::NbAliveCharacter() const
{
  uint nbr = 0;
  const_iterator it= characters.begin(), end=characters.end();
  
  for (; it!=end; ++it)
    if ( !(*it).IsDead() ) nbr++;

  return nbr;
}

// Prepare le tour d'une equipe
void Team::PrepareTurn()
{
  // Choisi un ver vivant si possible
  if (ActiveCharacter().IsDead())
  {
    camera_est_sauve = false;
    NextCharacter();
  }

  if (camera_est_sauve) camera.SetXYabs (sauve_camera.x, sauve_camera.y);
  camera.ChangeObjSuivi (&ActiveCharacter(), 
			 !camera_est_sauve, !camera_est_sauve, 
			 true);
  CurseurVer::GetInstance()->SuitVerActif();

  // Active last weapon use if EnoughAmmo
  if (AccessWeapon().EnoughAmmo())
    AccessWeapon().Select();
  else { // try to find another weapon !!
    active_weapon = weapons_list.GetWeapon(WEAPON_BAZOOKA);
    AccessWeapon().Select();
  }
}

Character& Team::ActiveCharacter()
{ 
  return (*active_character);
}

// Change d'arme
void Team::SetWeapon (Weapon_type type)
{
  AccessWeapon().Deselect();
  active_weapon = weapons_list.GetWeapon(type);
  AccessWeapon().Select();
}

int Team::ReadNbAmmos() const
{
  // Read in the Map
  // The same method as in AccesNbAmmos can't be use on const team !
  std::map<std::string, int>::const_iterator it 
    = m_nb_ammos.find( active_weapon->GetName() ) ;
  
  if (it != m_nb_ammos.end())  return ( it->second ) ;  
  return 0 ;
}

int Team::ReadNbUnits() const
{
  std::map<std::string, int>::const_iterator it 
    = m_nb_units.find( active_weapon->GetName() ) ;
  
  if (it !=  m_nb_units.end())  return ( it->second ) ;  
  return 0 ;
}

int Team::ReadNbAmmos(const std::string &weapon_name) const
{
  // Read in the Map
  // The same method as in AccesNbAmmos can't be use on const team !
  std::map<std::string, int>::const_iterator it = 
    m_nb_ammos.find( weapon_name );
  
  if (it !=  m_nb_ammos.end()) return ( it->second ) ;  
  return 0 ;
  
}

int Team::ReadNbUnits(const std::string &weapon_name) const
{
  std::map<std::string, int>::const_iterator it = 
    m_nb_units.find( weapon_name );
  
  if (it !=  m_nb_units.end()) return ( it->second ) ;  
  return 0 ;
  
}

int& Team::AccessNbAmmos()
{
  // if value not initialized, it initialize to 0 and then return 0
  return m_nb_ammos[ active_weapon->GetName() ] ;
}

int& Team::AccessNbUnits()
{
  // if value not initialized, it initialize to 0 and then return 0
  return m_nb_units[ active_weapon->GetName() ] ;
}

void Team::ResetNbUnits()
{
  m_nb_units[ active_weapon->GetName() ] = active_weapon->ReadInitialNbUnit();
}

Team::iterator Team::begin() { return characters.begin(); }
Team::iterator Team::end() { return characters.end(); }

void Team::LoadGamingData(uint how_many_characters)
{
  // Reset ammos
  m_nb_ammos.clear();
  m_nb_units.clear();
  std::list<Weapon *> l_weapons_list = weapons_list.GetList() ;
  std::list<Weapon *>::iterator itw = l_weapons_list.begin(), 
    end = l_weapons_list.end();

  for (; itw != end ; ++itw) {
    m_nb_ammos[ (*itw)->GetName() ] = (*itw)->ReadInitialNbAmmo();
    m_nb_units[ (*itw)->GetName() ] = (*itw)->ReadInitialNbUnit();
  }
  
  active_weapon = weapons_list.GetWeapon(WEAPON_DYNAMITE);
  camera_est_sauve = false;

  LoadCharacters(how_many_characters);

  crosshair.Reset();
}

void Team::UnloadGamingData()
{
  // Clear list of characters
  characters.clear();
}

void Team::Draw()
{
  energy.Draw ();
}

void Team::Refresh()
{
  energy.Refresh();
}

Weapon& Team::AccessWeapon() const { return *active_weapon; }
const Weapon& Team::GetWeapon() const { return *active_weapon; }
Weapon_type Team::GetWeaponType() const { return GetWeapon().GetType(); }

bool Team::IsSameAs(const Team& other)
{
  return (strcmp(m_id.c_str(), other.GetId().c_str()) == 0);
}
