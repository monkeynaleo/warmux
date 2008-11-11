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

#include <server.h>

GameServer::GameServer() :
  password(""),
  clients_socket_set(NULL)
{
}

bool GameServer::ServerStart(uint port, uint max_nb_clients, std::string& _password)
{
  password = _password;

  // Open the port to listen to
  if (!server_socket.AcceptIncoming(port)) {
    DPRINT(INFO, "Unable to use port %d", port);
    return false;
  }

  DPRINT(INFO, "Server successfully started on port %d", port);
  clients_socket_set = WSocketSet::GetSocketSet(max_nb_clients);
  if (!clients_socket_set) {
    DPRINT(INFO, "ERROR: Fail to allocate Socket Set");
    return false;
  }

  return true;
}

bool GameServer::HandShake(WSocket& client_socket)
{
  return WNet::Server_HandShake(client_socket, password);
}

void GameServer::RejectIncoming()
{
  server_socket.Disconnect();
}

void GameServer::WaitClients()
{
  if (server_socket.IsConnected()) {

    // Check for an incoming connection
    WSocket* incoming = server_socket.LookForClient();
    if (incoming) {

      if (!HandShake(*incoming))
 	return;

      clients_socket_set->AddSocket(incoming);


      // ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_INFO_CLIENT_CONNECT,
      // 							 client->GetAddress()));
      DPRINT(INFO, "New client connected: %s", incoming->GetAddress().c_str());

      if (clients_socket_set->NbSockets() == clients_socket_set->MaxNbSockets())
	RejectIncoming();
    }
    SDL_Delay(100);
  }
}
