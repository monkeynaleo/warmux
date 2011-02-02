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
#include "replay_info.h"
#include "game/game_mode.h"
#include "game/config.h"
#include "map/maps_list.h"
#include "include/action_handler.h"
#include "include/constant.h"
#include "game/game_time.h"
#include "team/teams_list.h"

//#define REPLAY_TRACE

Replay replay;
static const uint max_packet_size = 100;

Uint32 DoAction(Uint32 interval, void *param)
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
#ifdef REPLAY_TRACE
    printf("Corrected next playing time to %i\n", next);
#endif
  } while (next <= 0);

  // Compute 
  return next;
}

Replay::Replay()
  : buf(NULL)
  , ptr(NULL)
  , bufsize(0)
  , is_recorder(true)
{
  DeInit();
  //Stores (4*96)*1024 bytes or around 32 minutes of gameplay
  ChangeBufsize(96*1024);
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

  if (current_action) delete current_action;
  current_action = NULL;

  replay_state = NOTHING;
}

void Replay::ChangeBufsize(Uint32 n)
{
  if (n <= bufsize)
    return;

  // All data is supposed to be consumed
  buf = (Uint32*)realloc(buf, n*sizeof(Uint32));
  bufsize = n*sizeof(Uint32);
  ptr = buf;
}

bool Replay::StartRecording()
{
#ifdef REPLAY_TRACE
  printf("Asked to start recording\n");
#endif
  assert(is_recorder && replay_state == PAUSED_RECORD);

  replay_state = RECORDING;
  wait_state   = WAIT_NOT;
  total_time   = 0;
  start_time   = GameTime::GetInstance()->Read();
  old_time     = 0;

  return true;
}

bool Replay::SaveReplay(const std::string& name, const char *comment)
{
  assert (is_recorder);

  std::ofstream out(name.c_str(), std::ofstream::binary);
  if (!out)
  {
    Error(Format(_("Couldn't open %s\n"), name.c_str()));
    return false;
  }

  // Generate replay info and dump it to file
  ReplayInfo *info = ReplayInfo::ReplayInfoFromCurrent(total_time, comment);
  info->DumpToFile(out);
  delete info;

  // Flush actions recorded
#ifdef REPLAY_TRACE
  Uint32 pos = out.tellp();
  printf("Actions stored at %u on %u bytes\n", pos, MemUsed());
#endif
  out.write((char*)buf, MemUsed());
  out.close();

  // @fixme Return length actually written
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

  assert(is_recorder && replay_state==RECORDING);

  // Enlarge buffer if can't contain max packet size
  if (ptr-buf > (int)(bufsize-max_packet_size-3)*4)
    ChangeBufsize(2*bufsize);
  
  // Time - and try avoiding drift
  total_time = GameTime::GetInstance()->Read() - start_time;
  duration = (Sint32)total_time-old_time;
  if (duration<0) duration=0; //Shouldn't happen but...
  old_time = total_time;
  SDLNet_Write32(duration, ptr); ptr++;

  // Packet body
  a->Write((char*)ptr);
  size = 2+SDLNet_Read32(ptr+1); // type+size+sizeof(data)
  if (size > max_packet_size) {
    Error(Format("Bad packet, stream was: %l08X %l08X %l08X\n",
                 ptr[0], ptr[1], ptr[2]));
  }
  ptr += size;

#ifdef REPLAY_TRACE
  Action_t      type = a->GetType();
  ActionHandler *ah = ActionHandler::GetInstance();
  printf("Storing action %s: time=%u type=%i length=%i\n", 
         ah->GetActionName(type).c_str(), duration, type, size*sizeof(Uint32));
#endif
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
    goto err;
  }

  info = ReplayInfo::ReplayInfoFromFile(in);
  if (!info->IsValid())
    goto err;

  if (info->GetVersion() != Constants::WARMUX_VERSION) {
    Error(Format(_("Bad version: %s != %s"),
                 info->GetVersion().c_str(),
                 Constants::WARMUX_VERSION.c_str()));
    goto err;
  }

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

#ifdef REPLAY_TRACE
  printf("Game mode: turn=%us move_player=%u max_energy=%u init_energy=%u\n",
         game_mode->duration_turn, game_mode->duration_move_player,
         game_mode->character.max_energy, game_mode->character.init_energy);
#endif

  // All of the above could be avoided through a GameMode::Load
  config_loaded = true;

  // Get remaining data
  pos = in.tellg();
  in.seekg(0, std::fstream::end);
  bufsize = in.tellg()-pos;
  in.seekg(pos);
#ifdef REPLAY_TRACE
  printf("Allocated %ib for actions found at %i\n", int(bufsize), int(pos));
#endif

  if (bufsize%4) {
    // Make it fatal
    Error(Format(_("Warning, malformed replay with data of size %u"), bufsize));
    return false;
  }

  ChangeBufsize(bufsize/4);

  in.read((char*)buf, bufsize);
  if (!!in) status = true;

 err:
  in.close();
  if (info) delete info;
  return status;
}

// Only use is internal, but let those parameters be available
Action* Replay::GetAction(Sint32 *tick_time)
{
  Action::Action_t   type;
  Uint16             size;
  Action             *a;

  ASSERT(!is_recorder && replay_state == PLAYING);
  
  // Does it contain the 3 elements needed to decode at least
  // action header?
  if (MemUsed() > bufsize-3*4) {
    StopPlaying();
    return NULL;
  }

  *tick_time = SDLNet_Read32(ptr); ptr++;
  type = (Action::Action_t)SDLNet_Read32(ptr);
  if (type > Action::ACTION_TIME_VERIFY_SYNC) {
    Error(Format(_("Malformed replay: action with unknow type %08X"), type));
    StopPlaying();
    return NULL;
  }

  // @fixme Is first element in data always size in Uint32 ?
  size = SDLNet_Read32(ptr+1)+2;

  if (size > max_packet_size || MemUsed() > bufsize-size*4) {
    Error(Format(_("Malformed replay: action with datasize=%u"), size));
    StopPlaying();
    return NULL;
  }

  // @fixme We may read beyond end of buffer if malformed replay
  a = Action::FromMem(type, ptr+1, size);
  ptr += size;

#ifdef REPLAY_TRACE
  ActionHandler *ah = ActionHandler::GetInstance();
  printf("Read action %s: type=%i time=%i length=%i\n",
         ah->GetActionName(type).c_str(), type, *tick_time, size*4);
#endif

  return a;
}

bool Replay::StartPlaying()
{
  assert(!is_recorder && replay_state == PAUSED_PLAY);

  // Check GameMode
  GameMode * game_mode = GameMode::GetInstance();
  printf("Game mode: turn=%us move_player=%u max_nrg=%u init_nrg=%u\n",
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
        printf("Started playing\n");
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
    printf("Nothing to do\n");
    StopPlaying();
    return 0;
  }

  // Wait for Sink
  wait_time = GameTime::GetInstance()->Read();
  while (wait_state == WAIT_FOR_SINK) {
#ifdef REPLAY_TRACE
    printf("Waiting for sink...\n");
#endif
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
  if (id) SDL_RemoveTimer(id);
  id = 0;

  // Only replay seems to use this, so we can quit it now
  SDL_QuitSubSystem(SDL_INIT_TIMER);
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
