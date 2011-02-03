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
 * Replay engine for Wormux parading as network layer
 *****************************************************************************/

#include "replay.h"

#include <SDL_net.h>
#include <assert.h>

#include <WARMUX_debug.h>

#include "replay_info.h"
#include "game/game.h"
#include "game/game_mode.h"
#include "game/config.h"
#include "map/maps_list.h"
#include "include/action_handler.h"
#include "include/constant.h"
#include "game/game_time.h"
#include "team/teams_list.h"

#define MAX_PACKET_SIZE 100

Uint32 DoAction(Uint32 /*interval*/, void *param)
{
  Sint32 next;
  Replay *rpl = static_cast<Replay *>(param);

  do {
    next = rpl->PlayOneAction();

    // Check playing status and abort if needed
    if (!rpl->IsPlaying()) return 0;

    // Compute actual next time so to match current total_time
    // (Current time - start_time) is the current play time, which
    // should be total_time. Difference must subtracted to avoid drift
    next -= (Sint32)GameTime::GetInstance()->Read()
          - rpl->GetStartTime() - rpl->GetTotalTime();

    MSG_DEBUG("replay", "Corrected next playing time to %i\n", next);
  } while (next <= 0);

  // Compute
  return next;
}

Replay::Replay()
  : buf(NULL)
  , ptr(NULL)
  , bufsize(0)
  , is_recorder(true)
  , current_action(NULL)
{
  DeInit();
}

Replay::~Replay()
{
  DeInit();
  if (buf) free(buf);
}

void Replay::Init(bool rec)
{
  DeInit();

  is_recorder = rec;
  replay_state = (rec) ? PAUSED_RECORD : PAUSED_PLAY;
}

void Replay::DeInit()
{
  if (!is_recorder)
    StopPlaying();

  is_recorder = true;
  duration = 0;
  config_loaded = false;
  wait_state = WAIT_NOT;
  old_time = 0;

  if (current_action)
    delete current_action;
  current_action = NULL;

  replay_state = NOTHING;
}

void Replay::ChangeBufsize(Uint32 n)
{
  if (n <= bufsize)
    return;

  // All data is supposed to be consumed
  Uint32 offset = (bufsize) ? ptr-buf : 0;
  buf = (Uint32*)realloc(buf, n*4);
  bufsize = n*4;
  ptr = buf + offset;
}

bool Replay::StartRecording()
{
  MSG_DEBUG("replay", "Asked to start recording\n");
  ASSERT(is_recorder && replay_state == PAUSED_RECORD);

  replay_state = RECORDING;
  wait_state   = WAIT_NOT;
  start_time   = 0;
  old_time     = 0;

  // Write game mode rules at start of data
  Action a(Action::ACTION_RULES_SET_GAME_MODE);
  std::string game_mode_name = "replay";
  a.Push(game_mode_name);
  std::string game_mode;
  std::string game_mode_objects;
  GameMode::GetInstance()->ExportToString(game_mode, game_mode_objects);
  a.Push(game_mode);
  a.Push(game_mode_objects);
  ChangeBufsize(a.GetSize()/2);
  a.Write((char*)ptr);
  ptr += a.GetSize()/4;
  MSG_DEBUG("replay", "Wrote game mode on %u bytes\n", a.GetSize());

  return true;
}

bool Replay::SaveReplay(const std::string& name, const char *comment)
{
  ASSERT(is_recorder);

  std::ofstream out(name.c_str(), std::ofstream::binary);
  if (!out) {
    Error(Format(_("Couldn't open %s\n"), name.c_str()));
    return false;
  }

  // Generate replay info and dump it to file
  total_time = old_time - start_time;
  ReplayInfo *info = ReplayInfo::ReplayInfoFromCurrent(total_time, comment);
  info->DumpToFile(out);
  delete info;

  // Save seed
  Write32(out, seed);

  // Flush actions recorded
  Uint32 pos = out.tellp();
  MSG_DEBUG("replay", "Actions stored at %u on %u bytes in %s, seed %08X\n",
            pos, MemUsed(), name.c_str(), seed);
  out.write((char*)buf, MemUsed());
  out.close();

  // should return length actually written
  return true;
}

// The Replay packet header:
// u32: time (ms => 2^16=65s => u16 sufficient)
// Packet is directly an action:
//   . u32 => type
//   . u32 => length
//   . data
void Replay::StoreAction(Action* a)
{
  uint          size;

  ASSERT(is_recorder && replay_state==RECORDING);

  Action::Action_t type = a->GetType();
  if (type == Action::ACTION_NETWORK_VERIFY_RANDOM_SYNC ||
      type == Action::ACTION_TIME_VERIFY_SYNC ||
      type == Action::ACTION_GAME_CALCULATE_FRAME ||
      type == Action::ACTION_NETWORK_PING)
    return;

  // Enlarge buffer if it can't contain max packet size
  if (MemUsed() > bufsize - MAX_PACKET_SIZE*4)
    ChangeBufsize(2*bufsize);

  // Packet body
  a->Write((char*)ptr);
  size = a->GetSize()/4;
  if (size > MAX_PACKET_SIZE-4) {
    Error(Format("Bad packet, stream was: %l08X %l08X %l08X\n",
                 ptr[0], ptr[1], ptr[2]));
  }
  ptr += size;

  // Check time
  if (start_time == 0)
    start_time = a->GetTimestamp();
  else
    old_time = a->GetTimestamp();

  ActionHandler *ah = ActionHandler::GetInstance();
  MSG_DEBUG("replay", "Storing action %s: time=%u type=%i length=%i\n",
          ah->GetActionName(type).c_str(), a->GetTimestamp(), type, size*4);
}




bool Replay::LoadReplay(const std::string& name)
{
#define TEMP_SIZE 256
  char           temp[TEMP_SIZE];
  bool           status     = false;
  std::streampos pos;
  ReplayInfo     *info      = NULL;
  GameMode       *game_mode = GameMode::GetInstance();
  int            map_id, val;

  ASSERT(!is_recorder);

  std::ifstream in(name.c_str(), std::fstream::binary);
  if (!in) {
    Error(Format(_("Couldn't open %s\n"), name.c_str()));
    goto done;
  }

  info = ReplayInfo::ReplayInfoFromFile(in);
  if (!info->IsValid())
    goto done;

  if (info->GetVersion() != Constants::WARMUX_VERSION) {
    Error(Format(_("Bad version: %s != %s"),
                 info->GetVersion().c_str(),
                 Constants::WARMUX_VERSION.c_str()));
    goto done;
  }
  goto ok;

done:
   in.close();
   if (info) delete info;
   return status;

ok:
  // map ID
  map_id = MapsList::GetInstance()->FindMapById(info->GetMapId());
  if (map_id == -1) {
    Error(Format(_("Couldn't find map %s"), temp));
    return false;
  }
  MapsList::GetInstance()->SelectMapByIndex(map_id);

  // Backup playing list
  TeamsList& teams_list = GetTeamsList();
  backup_list = teams_list.playing_list;
  teams_list.playing_list.clear();

  // Teams
  for (uint i = 0; i<info->GetTeams().size(); i++) {
    ConfigTeam team_cfg;
    teams_list.AddTeam(info->GetTeams()[i], true);
  }

  // Game mode
  memcpy(&mode_info, info->GetGameModeInfo(), sizeof(GameModeInfo));

  // Set GameMode
  val = mode_info.allow_character_selection;
  mode_info.allow_character_selection = game_mode->allow_character_selection;
  game_mode->allow_character_selection = (GameMode::manual_change_character_t)val;
#define SWAP(a, b) val = a; a = b; b = val
  SWAP(mode_info.turn_duration, game_mode->duration_turn);
  SWAP(mode_info.duration_before_death_mode, game_mode->duration_before_death_mode);
  SWAP(mode_info.damage_per_turn_during_death_mode, game_mode->damage_per_turn_during_death_mode);
  SWAP(mode_info.init_energy, game_mode->character.init_energy);
  SWAP(mode_info.max_energy, game_mode->character.max_energy);
  SWAP(mode_info.gravity, game_mode->gravity);

  MSG_DEBUG("replay", "Game mode: turn=%us move_player=%u max_energy=%u init_energy=%u\n",
          game_mode->duration_turn, game_mode->duration_move_player,
          game_mode->character.max_energy, game_mode->character.init_energy);

  // All of the above could be avoided through a GameMode::Load
  config_loaded = true;

  seed = Read32(in);

  // Get remaining data
  pos = in.tellg();
  in.seekg(0, std::fstream::end);
  uint size = in.tellg()-pos;
  in.seekg(pos);
  MSG_DEBUG("replay", "Allocated %ib for actions found at %i\n", int(bufsize), int(pos));

  if (size%4) {
    // Make it fatal
    Error(Format(_("Warning, malformed replay with data of size %u"), bufsize));
    return false;
  }

  ChangeBufsize(size/4);

  in.read((char*)buf, size);
  if (!in) {
    Error(Format(_("Warning, malformed replay with data of size %u"), bufsize));
    return false;
  }
  Action *a = new Action((char*)buf, NULL);
  if (!a || a->GetType() != Action::ACTION_RULES_SET_GAME_MODE ||
      a->PopString() != "replay") {
    Error(Format(_("Warning, malformed replay with data of size %u"), bufsize));
    return false;
  }
  std::string mode = a->PopString();
  std::string mode_objects = a->PopString();
  game_mode->LoadFromString("replay", mode, mode_objects);
  ptr += a->GetSize()/4;
  delete a;

  goto done;
}

// Only use is internal, but let those parameters be available
Action* Replay::GetAction(Sint32 *tick_time)
{
  Action::Action_t   type;
  Action             *a;

  ASSERT(!is_recorder && replay_state == PLAYING);

  // Does it contain the 4 elements needed to decode at least
  // action header?
  if (MemUsed() > bufsize-3*4) {
    StopPlaying();
    return NULL;
  }

  // Read action
  a = new Action((char*)ptr, NULL);
  type = a->GetType();
  if (type > Action::ACTION_TIME_VERIFY_SYNC) {
    Error(Format(_("Malformed replay: action with unknow type %08X"), type));
    StopPlaying();
    return NULL;
  }

  // Move pointer
  Uint16 size = a->GetSize()/4;
  if (size > MAX_PACKET_SIZE || MemUsed() > bufsize-size*4) {
    Error(Format(_("Malformed replay: action with datasize=%u"), size));
    StopPlaying();
    return NULL;
  }
  ptr += size;

  // Set duration
  if (start_time == 0) {
    start_time = a->GetTimestamp();
    *tick_time = 0;
  } else {
    *tick_time = a->GetTimestamp() - start_time;
  }

  ActionHandler *ah = ActionHandler::GetInstance();
  MSG_DEBUG("replay", "Read action %s: type=%i time=%i length=%i\n",
          ah->GetActionName(type).c_str(), type, *tick_time, size*4);

  return a;
}

bool Replay::StartPlaying()
{
  ASSERT(!is_recorder && replay_state == PAUSED_PLAY);

  // Check GameMode
  GameMode * game_mode = GameMode::GetInstance();
  MSG_DEBUG("replay", "Game mode: turn=%us move_player=%u max_nrg=%u init_nrg=%u\n",
          game_mode->duration_turn, game_mode->duration_move_player,
          game_mode->character.max_energy, game_mode->character.init_energy);

  replay_state = PLAYING;
  wait_state   = WAIT_FOR_SOURCE;
  total_time   = 0;

  current_action = GetAction(&duration);
  start_time     = GameTime::GetInstance()->Read();
  while (1) {
    if (current_action) {
      if (current_action->IsFrameLess())
        PlayOneAction(); // Refills current_action
      else {
        id = SDL_AddTimer(duration, DoAction, this);
        MSG_DEBUG("replay", "Started playing\n");
        return true;
      }
    } else {
      StopPlaying();
      return false;
    }
  }
}

// Returns time of next action
Uint32 Replay::PlayOneAction()
{
  Uint32 wait_time;

  ASSERT(!is_recorder);
  if (current_action == NULL || replay_state != PLAYING) {
    MSG_DEBUG("replay", "Nothing to do\n");
    StopPlaying();
    return 0;
  }

  // Wait for Sink
  wait_time = GameTime::GetInstance()->Read();
  while (wait_state == WAIT_FOR_SINK) {
    MSG_DEBUG("replay", "Waiting for sink...\n");
    SDL_Delay(100);
  }
  start_time += GameTime::GetInstance()->Read() - wait_time;

  // Perform the action
  total_time += duration;
  ActionHandler::GetInstance()->NewAction(current_action, false);
  // ActionHandler clears the action
  current_action = NULL;

  // Get new action
  current_action = GetAction(&duration);

  // Keep pushing and getting new action untill not in the same frame
  while (current_action && current_action->IsFrameLess()) {
    ActionHandler::GetInstance()->NewAction(current_action, false);
    current_action = GetAction(&duration);
  }

  if (!current_action) {
    StopPlaying();
    return 0;
  }

  return duration;
}

void Replay::StopPlaying()
{
  ASSERT(!is_recorder);

  if (replay_state != PLAYING)
    return;

  replay_state = PAUSED_PLAY;
  wait_state = WAIT_NOT;
  SDL_Delay(200);
  if (id)
    SDL_RemoveTimer(id);
  id = 0;

  // Only replay seems to use this, so we can quit it now
  replay_state = NOTHING;

  // Restore game mode
  GameMode * game_mode = GameMode::GetInstance();
  game_mode->allow_character_selection = (GameMode::manual_change_character_t)
    mode_info.allow_character_selection;
  game_mode->duration_turn = mode_info.turn_duration;
  game_mode->damage_per_turn_during_death_mode = mode_info.damage_per_turn_during_death_mode;
  game_mode->duration_before_death_mode = mode_info.duration_before_death_mode;
  game_mode->character.init_energy = mode_info.init_energy;
  game_mode->character.max_energy = mode_info.max_energy;
  game_mode->gravity = mode_info.gravity;

  // Restore playing list
  GetTeamsList().SetPlayingList(backup_list);
}

void StartPlaying(const std::string& name)
{
  Replay *replay = Replay::GetInstance();

  replay->Init(false);
  if (replay->LoadReplay(name.c_str())) {
    if (replay->StartPlaying()) {
      Game::GetInstance()->Start();
      replay->StopPlaying();
    }
  }
  replay->DeInit();
}
