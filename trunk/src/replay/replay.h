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
 * Replay engine for Warmux parading as network layer
 * @todo Evaluate rate generation and determine wheter gziping is interesting
 *****************************************************************************/

#ifndef REPLAY_H
#define REPLAY_H

#include <vector>
#include <WARMUX_action.h>
#include <WARMUX_singleton.h>

#include "game_mode_info.h"

class Team;

class Replay : public Singleton<Replay>
{
public:
  typedef enum
  {
    WAIT_FOR_SOURCE,
    WAIT_FOR_SINK,
    WAIT_NOT
  } wait_state_t;
  typedef enum
  {
    NOTHING,
    PLAYING,
    PAUSED_PLAY,
    RECORDING,
    PAUSED_RECORD
  } replay_state_t;

 private:
  Uint32         *buf;
  Uint32         *ptr;
  Uint32         bufsize;
  Uint32         total_time;
  Uint32         start_time;

  // State
  wait_state_t   wait_state;
  replay_state_t replay_state;
  bool           is_recorder;

  // For both
  Uint32         old_time;
  Sint32         duration;

  // For replaying
  SDL_TimerID    id;
  Action         *current_action;

  // For replaying: GameMode save
  bool           config_loaded;
  GameModeInfo   mode_info;

  // For replaying: save current team list
  std::vector<Team*> backup_list;

  void ChangeBufsize(Uint32 n);
  Uint32 MemUsed() { return ptr - buf; };
  static bool CheckReturn(const char*msg, bool a)
  {
    //std::cout << msg << ": replied state is: " << a << "\n";
    return a;
  }

 public:
  Replay();
  ~Replay();
  void Init(bool is_rec);
  void DeInit();
  Uint32 GetTotalTime() const { return total_time; }
  Uint32 GetStartTime() const { return start_time; }

  // Recorder
  bool StartRecording();
  void StopRecording() { replay_state = NOTHING; };
  void StoreAction(Action* a);
  bool IsRecording() const { return CheckReturn("IsRecording()", is_recorder && replay_state==RECORDING); };
  bool SaveReplay(const std::string& name, const char *comment = NULL);

  // Synching - no much need to have mutex here
  void SetWaitState(wait_state_t w) { wait_state = w; }
  wait_state_t GetWaitState() const { return wait_state; }

  // Replayer
  bool  LoadReplay(const std::string& name);
  Action* GetAction(Sint32* tick_time);
  bool StartPlaying();
  Uint32 PlayOneAction();
  void StopPlaying();
  bool IsPlaying() const { return CheckReturn("IsPlaying()", !is_recorder && replay_state==PLAYING); };
};

#endif //REPLAY_H
