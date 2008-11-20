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
  port(0),
  clients_socket_set(NULL)
{
}

bool GameServer::ServerStart(uint _port, uint max_nb_clients, const std::string& _game_name, std::string& _password)
{
  game_name = _game_name;
  password = _password;
  port = _port;

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

bool GameServer::HandShake(WSocket& client_socket, std::string& nickname)
{
  bool client_will_be_master = false;
  if (clients_socket_set->NbSockets() == 0)
    client_will_be_master = true;

  DPRINT(INFO, "%s will be master ? %d", client_socket.GetAddress().c_str(), client_will_be_master);
  return WNet::Server_HandShake(client_socket, game_name, password, nickname, client_will_be_master);
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
      std::string client_nickname;

      if (!HandShake(*incoming, client_nickname))
 	return;

      clients_socket_set->AddSocket(incoming);

      // ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_INFO_CLIENT_CONNECT,
      // 							 client->GetAddress()));
      DPRINT(INFO, "New client connected: %s (%s)", client_nickname.c_str(), incoming->GetAddress().c_str());

      if (clients_socket_set->NbSockets() == clients_socket_set->MaxNbSockets())
	RejectIncoming();
    }
    SDL_Delay(100);
  }
}

std::list<WSocket*>::iterator GameServer::CloseConnection(std::list<WSocket*>::iterator closed)
{
  std::list<WSocket*>::iterator it;

  DPRINT(INFO, "Client disconnected: %s", (*closed)->GetAddress().c_str());

  it = clients_socket_set->GetSockets().erase(closed);
  delete *closed;

  if (clients_socket_set->NbSockets() + 1 == clients_socket_set->MaxNbSockets()) {
    // A new player will be able to connect, so we reopen the socket
    // For incoming connections
    DPRINT(INFO, "Allowing new connections");
    server_socket.AcceptIncoming(port);
  }

  return it;
}

void GameServer::ForwardPacket(void * buffer, size_t len, const WSocket* sender)
{
  std::list<WSocket*>::iterator it;

  for (it = clients_socket_set->GetSockets().begin();
       it != clients_socket_set->GetSockets().end() ;
       it++) {

    if ((*it) != sender) {
      (*it)->SendPacket(buffer, len);
    }
  }
}

void GameServer::RunLoop()
{
  while (true) {

    WaitClients();

    int num_ready = clients_socket_set->CheckActivity(100);

    if (num_ready == -1) { // Means an error
      fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
      continue; //Or break?
    }

    void * buffer;
    size_t packet_size;
    std::list<WSocket*>::iterator it = clients_socket_set->GetSockets().begin();

    while (it != clients_socket_set->GetSockets().end()) {

      if ((*it)->IsReady()) { // Check if this socket contains data to receive

        if ((*it)->ReceivePacket(buffer, packet_size)) {
	  ForwardPacket(buffer, packet_size, *it);
	  free(buffer);
	} else {
	  it = CloseConnection(it);
	  break;
	}
      }
      it++;
    }
  }
}
