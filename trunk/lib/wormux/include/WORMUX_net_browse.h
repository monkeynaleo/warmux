/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 * Wormux Network data structures and functions to browse list of
 * network games
 *****************************************************************************/

#ifndef WORMUX_NET_BROWSE_H
#define WORMUX_NET_BROWSE_H
//-----------------------------------------------------------------------------
#include <string>
#include <SDL_thread.h>

class GameServerInfo
{
public:
  std::string ip_address;
  std::string port;
  std::string dns_address;
  std::string game_name;
  bool        passworded;
};

class Broadcaster
{
  friend void broadcast_gameinfo(void *);

private:
  static GameServerInfo info;
  static SDL_Thread *thread;
  static bool running;

public:
  static void Start(const GameServerInfo& _info);
  static void Stop();
}

//-----------------------------------------------------------------------------
#endif
