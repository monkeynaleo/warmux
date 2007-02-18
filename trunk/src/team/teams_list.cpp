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
 * Team handling
 *****************************************************************************/

#include "teams_list.h"
//-----------------------------------------------------------------------------
#include "../character/body_list.h"
#include "../include/action_handler.h"
#include "../game/config.h"
#include "../tool/file_tools.h"
#include "../tool/i18n.h"
#include "team_energy.h"
#include <algorithm>
#include <iostream>

#if !defined(WIN32) || defined(__MINGW32__)
#include <dirent.h>
#include <sys/stat.h>
#endif
//-----------------------------------------------------------------------------
TeamsList teams_list;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TeamsList::TeamsList()
{}

TeamsList::~TeamsList()
{
  Clear();
  full_list.clear();
}

//-----------------------------------------------------------------------------

void TeamsList::NextTeam (bool begin_game)
{
  // End of turn for active team
  if (begin_game) return;

  // Next team
  std::vector<Team*>::iterator it=active_team;
  do
  {
    ++it;
    if (it == playing_list.end()) it = playing_list.begin();
  } while ((**it).NbAliveCharacter() == 0);
  ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_GAMELOOP_NEXT_TEAM, (**it).GetId()));
}

//-----------------------------------------------------------------------------

Team& TeamsList::ActiveTeam()
{
  assert (active_team != playing_list.end());
  return **active_team;
}

//-----------------------------------------------------------------------------

void TeamsList::LoadOneTeam(const std::string &dir, const std::string &team)
{
  // Skip '.', '..' and hidden files
  if (team[0] == '.') return;

#if !defined(WIN32) || defined(__MINGW32__)
  // Is it a directory ?
  struct stat stat_file;
  std::string filename = dir+team;
  if (stat(filename.c_str(), &stat_file) != 0) return;
  if (!S_ISDIR(stat_file.st_mode)) return;
#endif

  // Add the team
  Team * tmp = Team::CreateTeam (dir, team);
  if (tmp != NULL) {
    full_list.push_back(*tmp);
    std::cout << ((1<full_list.size())?", ":" ") << team;
    std::cout.flush();
  }
}

//-----------------------------------------------------------------------------

void TeamsList::LoadList()
{
  playing_list.clear() ;

  std::cout << "o " << _("Load teams:");

  // Load Wormux teams
  std::string dirname = Config::GetInstance()->GetDataDir() + PATH_SEPARATOR + "team" + PATH_SEPARATOR;
#if !defined(WIN32) || defined(__MINGW32__)
  struct dirent *file;
  DIR *dir = opendir(dirname.c_str());
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL)  LoadOneTeam (dirname, file->d_name);
    closedir (dir);
  } else {
    Error (Format(_("Cannot open teams directory (%s)!"), dirname.c_str()));
  }
#else
  std::string pattern = dirname + "*.*";
  WIN32_FIND_DATA file;
  HANDLE file_search;
  file_search=FindFirstFile(pattern.c_str(),&file);
  if(file_search != INVALID_HANDLE_VALUE)
  {
    while (FindNextFile(file_search,&file))
    {
      if(file.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
        LoadOneTeam(dirname,file.cFileName);
    }
  } else {
    Error (Format(_("Cannot open teams directory (%s)!"), dirname.c_str()));
  }
  FindClose(file_search);
#endif

  // Load personal teams
#if !defined(WIN32) || defined(__MINGW32__)
  dirname = Config::GetInstance()->GetPersonalDir() + PATH_SEPARATOR
    + "team" + PATH_SEPARATOR;
  dir = opendir(dirname.c_str());
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL) LoadOneTeam (dirname, file->d_name);
    closedir (dir);
  }
#endif

  teams_list.full_list.sort(compareTeams);

  // We need at least 2 teams
  if (full_list.size() < 2)
    Error(_("You need at least two valid teams !"));

  // Default selection
  std::list<uint> nv_selection;
  nv_selection.push_back (0);
  nv_selection.push_back (1);
  ChangeSelection (nv_selection);

  std::cout << std::endl;
  InitList(Config::GetInstance()->AccessTeamList());
}

//-----------------------------------------------------------------------------

void TeamsList::LoadGamingData(uint how_many_characters)
{
  active_team = playing_list.begin();

  iterator it=playing_list.begin(), end=playing_list.end();

  // Load the data of all teams
  for (; it != end; ++it) (**it).LoadGamingData(how_many_characters);
}

//-----------------------------------------------------------------------------

void TeamsList::UnloadGamingData()
{
  body_list.FreeMem();
  iterator it=playing_list.begin(), end=playing_list.end();

  // Unload the data of all teams
  for (; it != end; ++it) (**it).UnloadGamingData();
}

//-----------------------------------------------------------------------------

Team *TeamsList::FindById (const std::string &id, int &pos)
{
  full_iterator it=full_list.begin(), fin=full_list.end();
  int i=0;
  for (; it != fin; ++it, ++i)
  {
    if (it -> GetId() == id)
    {
      pos = i;
      return &(*it);
    }
  }
  pos = -1;
  return NULL;
}

//-----------------------------------------------------------------------------

Team *TeamsList::FindByIndex (uint index)
{
  full_iterator it=full_list.begin(), fin=full_list.end();
  uint i=0;
  for (; it != fin; ++it, ++i)
  {
    if (i == index) return &(*it);
  }
  assert (false);
  return NULL;
}

//-----------------------------------------------------------------------------

Team *TeamsList::FindPlayingByIndex (uint index)
{
  assert(index < playing_list.size());
  return playing_list[index];
}

//-----------------------------------------------------------------------------

Team* TeamsList::FindPlayingById(const std::string &id, uint &index)
{
  iterator it = playing_list.begin(), end = playing_list.end();
  index=0;
  for (; it != end; ++it, ++index)
  {
    if ((*it) -> GetId() == id)
      return *it;
  }
  assert(false);
  return NULL;
}

//-----------------------------------------------------------------------------

void TeamsList::InitList (const std::list<ConfigTeam> &lst)
{
  Clear();
  std::list<ConfigTeam>::const_iterator it=lst.begin(), end=lst.end();
  for (; it != end; ++it) {
    AddTeam (*it, false);
  }
  active_team = playing_list.begin();
}

//-----------------------------------------------------------------------------

void TeamsList::InitEnergy()
{
  // Looking at team with the greatest energy
  // (in case teams does not have same amount of character)
  iterator it=playing_list.begin(), fin=playing_list.end();
  uint max = 0;
  for (; it != fin; ++it)
  {
    if( (**it).ReadEnergy() > max)
      max = (**it).ReadEnergy();
  }

  // Init each team's energy bar
  it=playing_list.begin();
  uint i = 0;
  for (; it != fin; ++it)
  {
    (**it).InitEnergy (max);
    ++i;
  }

  // Initial ranking
  it=playing_list.begin();
  for (; it != fin; ++it)
  {
    uint rank = 0;
    iterator it2=playing_list.begin();
    for (; it2 != fin; ++it2)
    {
      if((it != it2)
          && (**it2).ReadEnergy() > (**it).ReadEnergy() )
        ++rank;
    }
    (**it).energy.rank_tmp = rank;
  }
  it=playing_list.begin();
  for (; it != fin; ++it)
  {
    uint rank = (**it).energy.rank_tmp;
    iterator it2=playing_list.begin();
    for (it2 = it; it2 != fin; ++it2)
    {
      if((it != it2)
          && (**it2).ReadEnergy() == (**it).ReadEnergy() )
        ++rank;
    }
    (**it).energy.SetRanking(rank);
  }
}

//-----------------------------------------------------------------------------

void TeamsList::RefreshEnergy()
{
  // In the order of the priorit :
  // - finish current action
  // - change a teams energy
  // - change ranking
  // - prepare energy bar for next event

  iterator it=playing_list.begin(), fin=playing_list.end();
  energy_t status;

  bool waiting = true; // every energy bar are waiting

  for (; it != fin; ++it) {
    if( (**it).energy.status != EnergyStatusWait) {
      waiting = false;
      break;
    }
  }

  // one of the energy bar is changing ?
  if(!waiting) {
    status = EnergyStatusOK;

    // change an energy bar value ?
    for (it=playing_list.begin(); it != fin; ++it) {
      if( (**it).energy.status == EnergyStatusValueChange) {
        status = EnergyStatusValueChange;
        break;
      }
    }

    // change a ranking ?
    for (it=playing_list.begin(); it != fin; ++it) {
      if( (**it).energy.status == EnergyStatusRankChange
             && ((**it).energy.IsMoving() || status == EnergyStatusOK)) {
        status = EnergyStatusRankChange;
        break;
      }
    }
  }
  else {
    // every energy bar are waiting
    // -> set state ready for a new event
    status = EnergyStatusOK;
  }

  // Setting event to process in every energy bar
  if(status != EnergyStatusOK || waiting) {
    it=playing_list.begin();
    for (; it != fin; ++it) {
      (**it).energy.status = status;
    }
  }

  // Actualisation des valeurs (pas d'actualisation de l'affichage)
  for (it=playing_list.begin(); it != fin; ++it) {
    (**it).UpdateEnergyBar();
    RefreshSort();
  }
}
//-----------------------------------------------------------------------------

void TeamsList::RefreshSort ()
{
  iterator it=playing_list.begin(), fin=playing_list.end();
  uint rank;

  // Find a ranking without taking acount of the equalities
  it=playing_list.begin();
  for (; it != fin; ++it)
  {
    rank = 0;
    iterator it2=playing_list.begin();
    for (; it2 != fin; ++it2)
    {
      if((it != it2)
          && (**it2).ReadEnergy() > (**it).ReadEnergy() )
        ++rank;
    }
    (**it).energy.rank_tmp = rank;
  }

  // Fix equalities
  it=playing_list.begin();
  for (; it != fin; ++it)
  {
    rank = (**it).energy.rank_tmp;
    iterator it2=playing_list.begin();
    for (it2 = it; it2 != fin; ++it2)
    {
      if((it != it2)
          && (**it2).ReadEnergy() == (**it).ReadEnergy() )
        ++rank;
    }
    (**it).energy.NewRanking(rank);
  }
}

//-----------------------------------------------------------------------------

void TeamsList::ChangeSelection (const std::list<uint>& nv_selection)
{
  selection = nv_selection;

  selection_iterator it=selection.begin(), fin=selection.end();
  playing_list.clear();
  for (; it != fin; ++it) playing_list.push_back (FindByIndex(*it));
  active_team = playing_list.begin();
}

//-----------------------------------------------------------------------------

bool TeamsList::IsSelected (uint index)
{
  selection_iterator pos = std::find (selection.begin(), selection.end(), index);
  return pos != selection.end();
}

void TeamsList::Clear()
{
  selection.clear();
  playing_list.clear();
}

//-----------------------------------------------------------------------------

void TeamsList::AddTeam (const ConfigTeam &the_team_cfg, bool generate_error)
{
  int pos;
  Team *the_team = FindById (the_team_cfg.id, pos);
  if (the_team != NULL) {

    // set the player name and number of characters
    the_team->SetPlayerName(the_team_cfg.player_name);
    the_team->SetNbCharacters(the_team_cfg.nb_characters);

    selection.push_back (pos);
    playing_list.push_back (the_team);

  } else {
    std::string msg = Format(_("Can't find team %s!"), the_team_cfg.id.c_str());
    if (generate_error)
      Error (msg);
    else
      std::cout << "! " << msg << std::endl;
  }
  active_team = playing_list.begin();
}

//-----------------------------------------------------------------------------

void TeamsList::UpdateTeam (const ConfigTeam &the_team_cfg, bool generate_error)
{
  int pos;
  Team *the_team = FindById (the_team_cfg.id, pos);
  if (the_team != NULL) {

    // set the player name and number of characters
    the_team->SetPlayerName(the_team_cfg.player_name);
    the_team->SetNbCharacters(the_team_cfg.nb_characters);

  } else {
    std::string msg = Format(_("Can't find team %s!"), the_team_cfg.id.c_str());
    if (generate_error)
      Error (msg);
    else
      std::cout << "! " << msg << std::endl;
  }
}

//-----------------------------------------------------------------------------

void TeamsList::DelTeam (const std::string &id)
{
  int pos;
  Team *equipe = FindById (id, pos);
  assert(equipe != NULL);

  selection.erase(find(selection.begin(),selection.end(),(uint)pos));
  playing_list.erase(find(playing_list.begin(),playing_list.end(),equipe));

  active_team = playing_list.begin();
}

//-----------------------------------------------------------------------------

void TeamsList::SetActive(const std::string &id)
{
  iterator
      it = playing_list.begin(),
  end = playing_list.end();
  for (; it != end; ++it)
  {
    Team &team = **it;
    if (team.GetId() == id)
    {
      active_team = it;
      return;
    }
  }
  Error (Format(_("Can't find team %s!"), id.c_str()));
}

//-----------------------------------------------------------------------------

Team& ActiveTeam()
{
  return teams_list.ActiveTeam();
}

//-----------------------------------------------------------------------------

Character& ActiveCharacter()
{
  return ActiveTeam().ActiveCharacter();
}

//-----------------------------------------------------------------------------

bool compareTeams(const Team& a, const Team& b)
{
  return a.GetName() < b.GetName();
}

//-----------------------------------------------------------------------------
