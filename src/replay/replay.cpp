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

Replay::Replay()
  : buf(NULL)
  , bufsize(0)
  , is_recorder(true)
{
  DeInit();
}

Replay::~Replay()
{
  DeInit();
  if (buf)
    free(buf);
}

void Replay::Init(bool rec)
{
  DeInit();

  is_recorder = rec;
  replay_state = (rec) ? PAUSED_RECORD : PAUSED_PLAY;
}

void Replay::DeInit()
{
  ptr = buf;
  config_loaded = false;
  old_time = 0;

  replay_state = NOTHING;
}

void Replay::ChangeBufsize(uint32_t n)
{
  if (n <= bufsize)
    return;

  // All data is supposed to be consumed
  uint32_t offset = (bufsize) ? ptr-buf : 0;
  buf = (uint32_t*)realloc(buf, n*4);
  bufsize = n*4;
  ptr = buf + offset;
}

bool Replay::StartRecording(const std::string& game_mode_name,
                            const std::string& game_mode,
                            const std::string& game_mode_objects)
{
  MSG_DEBUG("replay", "Asked to start recording\n");
  ASSERT(is_recorder && replay_state == PAUSED_RECORD);

  replay_state = RECORDING;
  start_time   = 0;
  old_time     = 0;

  // Write game mode rules at start of data
  Action a(Action::ACTION_RULES_SET_GAME_MODE);
  a.Push(game_mode_name);
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
  if (!out)
    return false;

  // Generate replay info and dump it to file
  uint32_t total_time = old_time - start_time;
  ReplayInfo *info = ReplayInfo::ReplayInfoFromCurrent(total_time, comment);
  if (!info->DumpToFile(out)) {
    delete info;
    return false;
  }
  delete info;

  // Save seed
  Write32(out, seed);

  // Flush actions recorded
  uint32_t pos = out.tellp();
  MSG_DEBUG("replay", "Actions stored at %u on %u bytes in %s, seed %08X\n",
            pos, MemUsed(), name.c_str(), seed);
  out.write((char*)buf, MemUsed());

  bool good = out.good();
  out.close();

  // should maybe return length actually written
  return good;
}

// The Replay packet header:
// u32: time (ms => 2^16=65s => u16 sufficient)
// Packet is directly an action:
//   . u32 => type
//   . u32 => length
//   . data
void Replay::StoreAction(const Action* a)
{
  uint          size;

  ASSERT(is_recorder && replay_state==RECORDING);

  Action::Action_t type = a->GetType();
  if ((a->IsFrameLess() && type!=Action::ACTION_CHAT_MESSAGE) ||
      type == Action::ACTION_NETWORK_PING ||
      type == Action::ACTION_NETWORK_VERIFY_RANDOM_SYNC ||
      type == Action::ACTION_TIME_VERIFY_SYNC ||
      type == Action::ACTION_RULES_SET_GAME_MODE)
    return;

  // Special case to convert into local packet
  if (type == Action::ACTION_REQUEST_BONUS_BOX_DROP) {
    // The timer shouldn't have moved
    Action a(Action::ACTION_DROP_BONUS_BOX);
    StoreAction(&a);
    return;
  }

  size = a->GetSize();
  if (type != Action::ACTION_GAME_CALCULATE_FRAME) {
    const ActionHandler *ah = ActionHandler::GetConstInstance();
    MSG_DEBUG("replay", "Storing action %s: type=%i length=%i\n",
              ah->GetActionName(type).c_str(), type, size);
  }

  // Enlarge buffer if it can't contain max packet size
  if (MemUsed() > bufsize - size*4)
    ChangeBufsize(2*bufsize);

  // Packet body
  a->Write((char*)ptr);
  ptr += size/4;

  // Check time
  if (start_time == 0)
    start_time = GameTime::GetInstance()->Read();
  else
    old_time = GameTime::GetInstance()->Read();
}


/////////////////////////////////////////////////////////////////////////////


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

err:
  Error(Format(_("Warning, malformed replay with data of size %u"), bufsize));

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
  MSG_DEBUG("replay", "Actions found at %u on %uB, seed=%08X\n", (uint)pos, size, seed);

  if (size%4) {
    // Make it fatal
    goto err;
  }

  // Explicit buffer change to avoid garbage
  if (buf)
    free(buf);
  buf = (uint32_t*)malloc(size);
  ptr = buf;
  bufsize = size;

  in.read((char*)buf, size);
  if (!in) {
    goto err;
  }
  Action *a = new Action((char*)buf, NULL);
  if (!a || a->GetType() != Action::ACTION_RULES_SET_GAME_MODE)
    goto err;
  ptr += a->GetSize()/4;
  const std::string& mode_name = a->PopString();
  const std::string& mode = a->PopString();
  const std::string& mode_objects = a->PopString();
  game_mode->LoadFromString(mode_name, mode, mode_objects);
  delete a;
  status = true;

  goto done;
}

// Only use is internal, but let those parameters be available
Action* Replay::GetAction()
{
  ASSERT(!is_recorder && replay_state == PLAYING);

  // Does it contain the 2 elements needed to decode at least
  // action header?
  if (MemUsed() > bufsize-sizeof(Action::Header)) {
    return NULL;
  }

  // Read action
  Action *a = new Action((char*)ptr, NULL);
  Action::Action_t type = a->GetType();
  if (type > Action::ACTION_TIME_VERIFY_SYNC) {
    Error(Format(_("Malformed replay: action with unknow type %08X"), type));
    StopPlaying();
    return NULL;
  }

  // Move pointer
  uint size = a->GetSize()/4;
  if (MemUsed() > bufsize-size*4) {
    Error(Format(_("Malformed replay: action with datasize=%u"), size));
    StopPlaying();
    return NULL;
  }
  ptr += size;

  const ActionHandler *ah = ActionHandler::GetConstInstance();
  MSG_DEBUG("replay", "Read action %s: type=%u length=%i\n",
            ah->GetActionName(type).c_str(), type, size*4);

  return a;
}

bool Replay::RefillActions()
{
  ActionHandler *ah = ActionHandler::GetInstance();

  ah->Lock();
  while (1) {
    Action *a = GetAction();
    if (a) {
      ah->NewAction(a, false);
    } else
      break;
  }
  ah->UnLock();

  return true;
}

bool Replay::StartPlaying()
{
  ASSERT(!is_recorder && replay_state == PAUSED_PLAY);

  // Check GameMode
  const GameMode * game_mode = GameMode::GetConstInstance();
  MSG_DEBUG("replay", "Game mode: turn=%us move_player=%u max_nrg=%u init_nrg=%u\n",
            game_mode->duration_turn, game_mode->duration_move_player,
            game_mode->character.max_energy, game_mode->character.init_energy);

  replay_state = PLAYING;

  return RefillActions();
}

void Replay::StopPlaying()
{
  ASSERT(!is_recorder);

  if (replay_state != PLAYING)
    return;

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
