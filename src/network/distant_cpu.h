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
 * Handle distant computers
 *****************************************************************************/

#ifndef DISTANT_CPU_H
#define DISTANT_CPU_H
//-----------------------------------------------------------------------------
#include "include/base.h"
#include "network/player.h"
#include <map>
#include <string>
//-----------------------------------------------------------------------------

class WSocket;

class DistantComputer
{
 public:
  typedef enum {
    STATE_ERROR,
    STATE_INITIALIZED,
    STATE_READY,
    STATE_CHECKED,
    STATE_NEXT_GAME
  } state_t;

 private:
  /* If you need this, implement it (correctly)*/
  DistantComputer(const DistantComputer&);
  const DistantComputer& operator=(const DistantComputer&);
  /*********************************************/

  WSocket* sock;
  DistantComputer::state_t state;
  bool force_disconnect;

  Player player;

public:
  DistantComputer(WSocket* new_sock);
  ~DistantComputer();

  bool SocketReady() const;
  bool ReceiveDatas(void* & data, size_t& len);
  bool SendDatas(const void* data, size_t len);

  std::string GetAddress();

  Player& GetPlayer();

  void SetState(DistantComputer::state_t _state);
  DistantComputer::state_t GetState() const;

  void ForceDisconnection();
  bool MustBeDisconnected();
};

#endif

