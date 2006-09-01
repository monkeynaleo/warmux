/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 * Network layer for Wormux.
 *****************************************************************************/

#ifndef NETWORK_H 
#define NETWORK_H
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <SDL_thread.h>
#include "../include/base.h" 
#include <list>
#include <string>
#include "../include/action.h"
#include "distant_cpu.h"
//-----------------------------------------------------------------------------
class Network
{
  friend class DistantComputer;

  bool inited;
public:
  typedef enum
    {
      NETWORK_NOT_CONNECTED,
      NETWORK_OPTION_SCREEN,
      NETWORK_INIT_GAME,
      NETWORK_READY_TO_PLAY,
      NETWORK_PLAYING
    } network_state_t;
  network_state_t state;
		
protected:
  bool m_is_connected;
  bool m_is_server;
  bool m_is_client;

  TCPsocket server_socket; // Wait for incoming connections on this socket
  SDL_Thread* thread; // network thread, where we receive data from network
  SDLNet_SocketSet socket_set;
  IPaddress ip; // for server : store listening port
                // for client : store server address/port

public:
  std::list<DistantComputer*> cpu; // list of the connected computer
  uint max_player_number;
  uint connected_player;
  uint client_inited;
  bool sync_lock;

  Network();
  ~Network();
  void Init();
  
  const bool IsConnected() const;
  const bool IsLocal() const;
  const bool IsServer() const;
  const bool IsClient() const;
  
  // Network functions common to client and server
  void Disconnect();

  // Action handling
  void SendAction(Action* action);
  void SendPacket(char* packet, int size);
  void ReceiveActions();

  // Client specific
  void ClientConnect(const std::string &host, const std::string &port);

  // Serveur specific methods
  void ServerStart(const std::string &port);
  void AcceptIncoming();
  void RejectIncoming();
  std::list<DistantComputer*>::iterator CloseConnection(std::list<DistantComputer*>::iterator closed);

};

extern Network network;
//-----------------------------------------------------------------------------
#endif
