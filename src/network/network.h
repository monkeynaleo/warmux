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
//-----------------------------------------------------------------------------

class Network
{
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
  std::list<TCPsocket> conn;
		
private:
	
  bool m_is_connected;
  bool m_is_server;
  bool m_is_client;

  TCPsocket server_socket; // Wait for incoming connections on this socket
  SDL_Thread* thread; // network thread, where we receive data from network
  SDLNet_SocketSet socket_set;
  IPaddress ip; // for server : store listening port
                // for client : store server address/port

  Action* make_action(Uint32* packet);

public:
  uint max_player_number;
  uint connected_player;
  uint client_inited;

	Network();
	~Network();
	void Init();
	
	bool is_connected();
	bool is_local();
	bool is_server();
	bool is_client();
	
	void disconnect();
	void client_connect(const std::string &host, const std::string &port);
	void server_start(const std::string &port);

  void SendAction(const Action &action);
  void ReceiveActions();

  void AcceptIncoming();
  void RejectIncoming();
  std::list<TCPsocket>::iterator CloseConnection(std::list<TCPsocket>::iterator closed);
};

extern Network network;
//-----------------------------------------------------------------------------
#endif
