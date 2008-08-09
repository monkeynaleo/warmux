/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Custom Team
 *****************************************************************************/

#include <sstream>
#include <iostream>
#include <errno.h>
#include <libxml/tree.h>

#include "game/config.h"
#include "team/custom_team.h"
#include "team/custom_teams_list.h"
#include "tool/file_tools.h"
#include "tool/i18n.h"
#include "tool/xml_document.h"

CustomTeam::CustomTeam()
{

}

CustomTeam::CustomTeam (const std::string &custom_teams_dir, const std::string &id){
std::string nomfich;
  XmlReader   doc;

  // Load XML
  nomfich = custom_teams_dir+id+ PATH_SEPARATOR "team.xml";

  if (!doc.Load(nomfich))
    throw "unable to load file of team data";

  if (!XmlReader::ReadString(doc.GetRoot(), "name", name))
    throw "Invalid file structure: cannot find a name for team ";


  // Load character names
  nb_characters = 10;

  // Create the characters
  xmlNodeArray nodes = XmlReader::GetNamedChildren(XmlReader::GetMarker(doc.GetRoot(), "team"), "character");
  xmlNodeArray::const_iterator it = nodes.begin();

  do
  {

    std::string character_name = "Unknown Soldier (it's all over)";

      XmlReader::ReadString(*it, "name", character_name);
      characters_name_list.push_back(character_name);


    MSG_DEBUG("team", "Add %s in  custom team %s", character_name.c_str(), name.c_str());

    // Did we reach the end ?
    ++it;
  } while (it != nodes.end() && characters_name_list.size() < nb_characters );



}


CustomTeam::~CustomTeam()
{

}


void CustomTeam::Delete()
{

}


std::vector<std::string> CustomTeam::GetCharactersNameList()
{
  return characters_name_list;
}

std::string CustomTeam::GetName()
{
  return name;
}

void CustomTeam::NewTeam()
{

  std::ostringstream oss;
    oss << GetCustomTeamsList().GetNumCustomTeam()+1;

    name = "team "+oss.str();
    for(unsigned i = 1; i<11; i++)
    {
      std::ostringstream oss2;
      oss2<<i;
     characters_name_list.push_back("character "+oss2.str());
    }
}

bool CustomTeam::Save()
{

 Config *config = Config::GetInstance();
std::string rep = config->GetPersonalConfigDir();
  // Create the directory if it doesn't exist
  if (!config->MkdirPersonalConfigDir())
  {
    std::cerr << "o "
	      << Format(_("Error while creating directory \"%s\": unable to store configuration file."),
			rep.c_str())
	      << " " << strerror(errno)
	      << std::endl;
    return false;
  }
  rep = config->GetPersonalConfigDir() + "custom_team" PATH_SEPARATOR;
  if (!CreateFolder(config->GetPersonalConfigDir() + "custom_team" PATH_SEPARATOR))
  {
    std::cerr << "o "
	      << Format(_("Error while creating directory \"%s\": unable to store configuration file."),
			rep.c_str())
	      << " " << strerror(errno)
	      << std::endl;
    return false;
  }

    rep = config->GetPersonalConfigDir() + "custom_team" PATH_SEPARATOR + name + PATH_SEPARATOR;

  if (!CreateFolder(config->GetPersonalConfigDir() + "custom_team" PATH_SEPARATOR + name + PATH_SEPARATOR))
  {
    std::cerr << "o "
	      << Format(_("Error while creating directory \"%s\": unable to store configuration file."),
			rep.c_str())
	      << " " << strerror(errno)
	      << std::endl;
    return false;
  }

  return SaveXml();
}

bool CustomTeam::SaveXml()
{

    const Config *config = Config::GetInstance();
  XmlWriter doc;
  std::string m_filename = config->GetPersonalConfigDir()  + "custom_team" PATH_SEPARATOR+ name + PATH_SEPARATOR + "team.xml";
  doc.Create(m_filename, "resources", "1.0", "utf-8");
  xmlNode *root = doc.GetRoot();
  doc.WriteElement(root, "name", name);


  xmlNode* team_node = xmlAddChild(root, xmlNewNode(NULL /* empty prefix */, (const xmlChar*)"team"));


  for(unsigned i=0 ; i < characters_name_list.size() ; i++){
    xmlNode* character = xmlAddChild(team_node, xmlNewNode(NULL /* empty prefix */, (const xmlChar*)"character"));
    doc.WriteElement(character, "name", characters_name_list[i]);
  }

  return doc.Save();
}




