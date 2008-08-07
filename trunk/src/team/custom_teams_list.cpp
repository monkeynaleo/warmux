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

 #include "team/custom_teams_list.h"
 #include "team/custom_team.h"
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

  for(full_iterator it = full_list.begin(); it != full_list.end(); ++it)
  {
    delete (*it);
  }
  full_list.clear();
  singleton = NULL;
}

void CustomTeamsList::LoadList()
{
  full_list.clear() ;

  //std::cout << "o " << _("Load custom teams:");

 /* const Config * config = Config::GetInstance();
  // Load Wormux teams
  std::string dirname = config->GetDataDir() + "team" PATH_SEPARATOR;
  FolderSearch *f = OpenFolder(dirname);
  if (f) {
    const char *name;
    while ((name = FolderSearchNext(f)) != NULL) LoadOneTeam(dirname, name);
    CloseFolder(f);
  } else {
    Error (Format(_("Cannot open teams directory (%s)!"), dirname.c_str()));
  }

  // Load personal teams
  dirname = config->GetPersonalDataDir() + "team" PATH_SEPARATOR;
  f = OpenFolder(dirname);
  if (f) {
    const char *name;
    while ((name = FolderSearchNext(f)) != NULL) LoadOneTeam(dirname, name);
    CloseFolder(f);
  } else {
    std::cerr << std::endl
      << Format(_("Cannot open personal teams directory (%s)!"), dirname.c_str())
      << std::endl;
  }

  full_list.sort(compareTeams);

  // We need at least 2 teams
  if (full_list.size() < 2)
    Error(_("You need at least two valid teams !"));

  // Default selection
  std::list<uint> nv_selection;
  nv_selection.push_back (0);
  nv_selection.push_back (1);
  ChangeSelection (nv_selection);

  std::cout << std::endl;
  InitList(Config::GetInstance()->AccessTeamList());*/
}
