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
 * Handle distant computers
 *****************************************************************************/

#ifndef DISTANT_CPU_H
#define DISTANT_CPU_H
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <SDL_thread.h>
#include <SDL_mutex.h>
#include "include/base.h"
#include <list>
#include <string>
#include "include/action.h"
//-----------------------------------------------------------------------------

class Action;

class DistantComputer
{
 public:
  typedef enum {
    STATE_ERROR,
    STATE_INITIALIZED,
    STATE_READY
  } state_t;

 private:
  /* If you need this, implement it (correctly)*/
  DistantComputer(const DistantComputer&);
  const DistantComputer& operator=(const DistantComputer&);
  /*********************************************/

  SDL_mutex* sock_lock;
  TCPsocket sock;
  std::list<std::string> owned_teams;

  DistantComputer::state_t state;

public:
  bool version_checked;
  bool force_disconnect;

  DistantComputer(TCPsocket new_sock);
  ~DistantComputer();

  bool SocketReady();
  int ReceiveDatas(char* & buf);
  void SendDatas(char* paket, int size);

  std::string GetAdress();
  std::string nickname;

  void ManageTeam(Action* team);
  void SendChatMessage(Action* a);

  void SetState(DistantComputer::state_t _state);
  DistantComputer::state_t GetState() const;
};

#endif

