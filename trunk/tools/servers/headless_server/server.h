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
 ******************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <string>

#include <WORMUX_network.h>
#include <WORMUX_socket.h>
#include <WORMUX_types.h>
#include <WSERVER_config.h>
#include <WSERVER_debug.h>

class GameServer
{
private:
  std::string password;
  uint port;

  WSocket server_socket;
  WSocketSet* clients_socket_set;

  bool HandShake(WSocket& client_socket, std::string& client_nickname);
  void WaitClients();
  void RejectIncoming();
  std::list<WSocket*>::iterator CloseConnection(std::list<WSocket*>::iterator closed);

  void ForwardPacket(void * buffer, size_t len, const WSocket* sender);

public:
  GameServer();

  bool ServerStart(uint port, uint max_nb_clients, std::string& password);
  void RunLoop();
};

#endif
