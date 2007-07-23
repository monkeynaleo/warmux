/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 *  Handle the game time. The game can be paused.
 *****************************************************************************/

#ifndef TIME_H
#define TIME_H

#include <string>
#include "include/base.h"

class Time
{
private:
  uint current_time;
  //uint max_time;
  uint delta_t;
  bool is_game_paused;
  static Time * singleton;

  uint real_time_game_start;
  uint real_time_pause_dt;
  uint real_time_pause_begin;

private:
  Time();
public:
  static Time * GetInstance();

  void Reset();
  bool IsGamePaused() const;

  // Read the time of the game, excluding paused time
  uint ReadRealTime() const;
  uint Read() const;
  uint ReadSec() const;
  uint ReadMin() const;
  void Refresh();
  uint GetDelta() const;
  //void RefreshMaxTime(uint updated_max_time);

  // Read the clock time
  uint ClockSec() const;  // ReadSec() % 60
  uint ClockMin() const;  // ReadMin() % 60
  std::string GetString() const;

  void Pause();
  void Continue();
};

#endif
