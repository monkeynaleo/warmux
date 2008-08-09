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
 * Team handling
 *****************************************************************************/

#include "game/config.h"
#include "team/custom_teams_list.h"
#include "team/custom_team.h"
#include "tool/file_tools.h"
#include "tool/i18n.h"
#include <iostream>

CustomTeamsList::CustomTeamsList()
{
  LoadList();
}

CustomTeamsList::~CustomTeamsList()
{

  if (!singleton)
  {
    fprintf(stderr, "Destructor still called on unexisting CustomTeamsList\n");
    return;
  }

  for(unsigned i = 0; i <  full_list.size(); i++)
  {
    delete full_list[i];
  }
  full_list.clear();
  singleton = NULL;
}

std::vector<CustomTeam *> CustomTeamsList::GetList(){

  return full_list;

}

void CustomTeamsList::LoadList()
{

  //Delete ?
  full_list.clear();

  const Config *config = Config::GetInstance();

  // Load personal custom teams
  std::string dirname = config->GetPersonalDataDir() + "custom_team" PATH_SEPARATOR;
  FolderSearch *f = OpenFolder(dirname);
  if (f) {
    const char *name;
    while ((name = FolderSearchNext(f)) != NULL) LoadOneTeam(dirname, name);
    CloseFolder(f);
  } else {
    std::cerr << std::endl
      << Format(_("Cannot open personal custom teams directory (%s)!"), dirname.c_str())
      << std::endl;
  }

  std::cout << std::endl;

}


void CustomTeamsList::LoadOneTeam(const std::string &dir, const std::string &custom_team_name)
{
  // Skip '.', '..' and hidden files
  if (custom_team_name[0] == '.') return;

  // Is it a directory ?
  if (!IsFolderExist(dir+custom_team_name)) return;

  // Add the team
  try {
    full_list.push_back(new CustomTeam(dir, custom_team_name));
    std::cout << ((1<full_list.size())?", ":" ") << custom_team_name;
    std::cout.flush();
  }

  catch (char const *error) {
    std::cerr << std::endl
              << Format(_("Error loading team :")) << custom_team_name <<":"<< error
              << std::endl;
    return;
  }
}

