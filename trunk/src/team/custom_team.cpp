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

#include "game/config.h"
#include "team/custom_team.h"
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
    std::string body_name = "";
    XmlReader::ReadStringAttr(*it, "name", character_name);

    characters_name_list.push_back(character_name);


    MSG_DEBUG("team", "Add %s in  custom team %s", character_name.c_str(), name.c_str());

    // Did we reach the end ?
    ++it;
  } while (it != nodes.end() && characters_name_list.size() < nb_characters );



}


std::string CustomTeam::GetName(){
  return name;
}

CustomTeam::~CustomTeam()
{

}


