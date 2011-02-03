/******************************************************************************
 *  Warmux is a convivial mass murder game.
 *  Copyright (C) 2001-2011 Warmux Team.
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
 *****************************************************************************
 * Information on/for a replay file
 *****************************************************************************/

#include <string>
#include <SDL_net.h>
#include <time.h>

#include <WARMUX_i18n.h>

#include "include/constant.h"
#include "game/game_time.h"
#include "game/game_mode.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "map/maps_list.h"

#include "replay_info.h"

const Uint32 HEADER_MAGIC = 0x57524D58; /* WRMX */
const Uint32 DATA_MAGIC   = 0xDEADBEEF;

void Write32(std::ofstream& out, Uint32 val)
{
  char  render[4];
  SDLNet_Write32(val, render);
  out.write(render, sizeof(Sint32));
}

Uint32 Read32(std::ifstream& in)
{
  char  render[4];
  in.read(render, 4);
  return SDLNet_Read32(render);
}

ReplayInfo::ReplayInfo(time_t d, Uint32 dms)
  : valid(false)
  , date(d)
  , duration_ms(dms)
{
  teams.clear();
  map_id.clear(); map_id = _("Unknown.");
  comment.clear(); comment = _("No comment.");
};

ReplayInfo *ReplayInfo::ReplayInfoFromFile(std::ifstream &in)
{
#define TEMP_SIZE 256
  char       temp[TEMP_SIZE];
  ReplayInfo *info = new ReplayInfo(0, 0);
  Uint32     marker;

  info->last_error = _("Unspecified error or end of file");
  info->valid = false;

  // Header magic
  marker = Read32(in);                      // Header marker
  if (marker != HEADER_MAGIC) {
    info->last_error =
      Format(_("Bad header 0x%08X instead of 0x%08X"), marker, HEADER_MAGIC);
    return info;
  }

  // Version
  in.getline(temp, TEMP_SIZE-1);            // Version
  info->version = temp;
  if (!in) return info;
  
  // Time
  info->duration_ms = Read32(in);           // Duration
  info->date        = Read32(in);           // Return of time(NULL)
  if (!in) return info;

  // Comment
  in.getline(temp, TEMP_SIZE-1);            // Comment
  if (!in) return info;
  info->comment = temp;

  // map ID
  in.getline(temp, TEMP_SIZE-1);            // Map ID
  if (!in) return info;
  info->map_id = temp;

  // Teams
  Uint32 num_teams = Read32(in);            // Number of teams
  if (num_teams > 8) {
    info->last_error =
      Format(_("Suspicious number of teams 0x%08X"), num_teams);
    return info;
  }
  while (num_teams) {
    ConfigTeam team_cfg;

    in.getline(temp, TEMP_SIZE-1);          // Team No.i name
    if (!in)
      goto team_error;
    team_cfg.id = std::string(temp);

    in.getline(temp, TEMP_SIZE-1);          // Player name for team No.i 
    if (!in)
      goto team_error;
    team_cfg.player_name = std::string(temp);

    team_cfg.nb_characters = Read32(in);
    if (!in)
      goto team_error;

    in.getline(temp, TEMP_SIZE-1);          // Nb characters for team ID No.i
    if (!in)
      goto team_error;
    team_cfg.ai = std::string(temp);

    info->teams.push_back(team_cfg);
    num_teams--;
    continue;

team_error:
    info->last_error = _("End of file while parsing teams");
    return info;
  }

  // Game mode
  info->mode_info.allow_character_selection = Read32(in);
  info->mode_info.turn_duration = Read32(in);
  info->mode_info.duration_before_death_mode = Read32(in);
  info->mode_info.damage_per_turn_during_death_mode = Read32(in);
  info->mode_info.init_energy = Read32(in);
  info->mode_info.max_energy = Read32(in);
  info->mode_info.gravity = Read32(in);

  if (Read32(in) != DATA_MAGIC) {           // Data magic
    info->last_error =
      Format(_("Bad data marker 0x%08X instead of 0x%08X"), marker, DATA_MAGIC);
    return info;
  }

  fprintf(stderr, "Everything went OK\n");
  info->valid = true;
  return info;
}

ReplayInfo *ReplayInfo::ReplayInfoFromCurrent(Uint32 duration, const char* comment)
{
  ReplayInfo *info    = new ReplayInfo(time(NULL), duration);
  
  info->version = Constants::WARMUX_VERSION; // Copy ?
  info->comment = (comment) ? comment : _("No comment.");
  info->map_id  = ActiveMap()->GetRawName();

  //Teams
  const std::vector<Team*>& plist = GetTeamsList().playing_list;
  for (uint i=0; i<plist.size(); i++) {
    ConfigTeam team_cfg = { plist[i]->GetId(), plist[i]->GetPlayerName(),
                            plist[i]->GetNbCharacters(), plist[i]->GetAIName() };
    info->teams.push_back(team_cfg);
  }

  // Game mode
  const GameMode * game_mode = GameMode::GetInstance();
  info->mode_info.allow_character_selection = game_mode->allow_character_selection;
  info->mode_info.turn_duration = game_mode->duration_turn;
  info->mode_info.duration_before_death_mode = game_mode->duration_before_death_mode;
  info->mode_info.damage_per_turn_during_death_mode = game_mode->damage_per_turn_during_death_mode;
  info->mode_info.init_energy = game_mode->character.init_energy;
  info->mode_info.max_energy = game_mode->character.max_energy;
  info->mode_info.gravity = (int)game_mode->gravity;

  // Everything ready
  info->valid   = true;
  return info;
}

bool
ReplayInfo::DumpToFile(std::ofstream &out)
{
  if (!valid) return false;

  Write32(out, HEADER_MAGIC);               // Header magic
  out << version << "\n";                   // Version

  Write32(out, duration_ms);                // Duration
  Write32(out, date);                       // Date
  out << comment << "\n";                   // Comment

  out << map_id << "\n";                    // Mad ID

  Write32(out, teams.size());
  for (uint i=0; i<teams.size(); i++) {     // Team No.i
    out << teams[i].id << "\n";
    out << teams[i].player_name << "\n";
    Write32(out, teams[i].nb_characters);
    out << teams[i].ai << "\n";
  }
  
  //Game mode
  Write32(out, mode_info.allow_character_selection);
  Write32(out, mode_info.turn_duration);
  Write32(out, mode_info.duration_before_death_mode);
  Write32(out, mode_info.damage_per_turn_during_death_mode);
  Write32(out, mode_info.init_energy);
  Write32(out, mode_info.max_energy);
  Write32(out, mode_info.gravity);

  Write32(out, DATA_MAGIC);                 // Data magic
  return (!!out);
}
