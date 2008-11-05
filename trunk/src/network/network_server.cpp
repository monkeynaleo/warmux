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
 * Network server layer for Wormux.
 *****************************************************************************/

#include "network/network_server.h"
//-----------------------------------------------------------------------------
#include <WORMUX_socket.h>
#include <SDL_thread.h>
#include "include/action_handler.h"
#include "network/distant_cpu.h"
#include "tool/string_tools.h"

#include <sys/types.h>
#ifdef LOG_NETWORK
#  include <sys/stat.h>
#  include <fcntl.h>
#  ifdef WIN32
#    include <io.h>
#  endif
#endif

//-----------------------------------------------------------------------------

NetworkServer::NetworkServer(const std::string& password) :
  Network(password),
  port(0)
{
#ifdef LOG_NETWORK
  fin = open("./network_server.in", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
  fout = open("./network_server.out", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
#endif
}

NetworkServer::~NetworkServer()
{
}

void NetworkServer::HandleAction(Action* a, DistantComputer* sender) const
{
  // Repeat the packet to other clients:
  if (a->GetType() != Action::ACTION_NETWORK_CHANGE_STATE
      && a->GetType() != Action::ACTION_NETWORK_CHECK_PHASE2)
  {
    char* packet;
    int packet_size;
    a->WriteToPacket(packet, packet_size);

    for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
         client != cpu.end();
         client++)
      if (*client != sender)
      {
        (*client)->SendDatas(packet, packet_size);
      }
    free(packet);
  }

  ActionHandler::GetInstance()->NewAction(a, false);
}

bool NetworkServer::HandShake(WSocket& client_socket) const
{
  return WNet::Server_HandShake(client_socket, GetPassword());
}

void NetworkServer::WaitActionSleep()
{
  if (server_socket.IsConnected()) {

    // Check for an incoming connection
    WSocket* incoming = server_socket.LookForClient();
    if (incoming) {

      if (!HandShake(*incoming))
 	return;

      socket_set->AddSocket(incoming);

      DistantComputer* client = new DistantComputer(incoming);
      cpu.push_back(client);

      ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_NETWORK_CONNECT,
							 client->GetAddress()));
      printf("New client connected\n");
      if (GetNbConnectedPlayers() >= max_nb_players)
        RejectIncoming();
    }
    SDL_Delay(100);
  }
}

void NetworkServer::RejectIncoming()
{
  server_socket.Disconnect();
}

//-----------------------------------------------------------------------------

connection_state_t NetworkServer::ServerStart(const std::string &net_port, uint _max_nb_players)
{
  WNet::Init();

  // The server starts listening for clients
  printf("o Starting server on port %s...\n", net_port.c_str());

  cpu.clear();
  // Convert port number (std::string port) into SDL port number format:
  if (!str2int(net_port, port)) {
    return CONN_BAD_PORT;
  }

  // Open the port to listen to
  if (!server_socket.AcceptIncoming(port)) {
    return CONN_BAD_PORT;
  }

  printf("o Server successfully started\n");
  max_nb_players = _max_nb_players;
  socket_set = WSocketSet::GetSocketSet(max_nb_players);
  if (!socket_set) {
    return CONN_REJECTED;
  }
  thread = SDL_CreateThread(Network::ThreadRun, NULL);

  return CONNECTED;
}

std::list<DistantComputer*>::iterator
NetworkServer::CloseConnection(std::list<DistantComputer*>::iterator closed)
{
  std::list<DistantComputer*>::iterator it;

  printf("- client disconnected: %s(%s)\n", (*closed)->GetAddress().c_str(), (*closed)->GetNickname().c_str());

  it = cpu.erase(closed);
  delete *closed;

  if (GetNbConnectedPlayers() == max_nb_players)
  {
    // A new player will be able to connect, so we reopen the socket
    // For incoming connections
    printf("Allowing new connections\n");
    server_socket.AcceptIncoming(port);
  }

  return it;
}

void NetworkServer::SetMaxNumberOfPlayers(uint _max_nb_players)
{
  max_nb_players = _max_nb_players;
}

uint NetworkServer::GetNbConnectedPlayers() const
{
  return cpu.size() + 1; // WARNING: is the server playing ?
}

uint NetworkServer::GetNbInitializedPlayers() const
{
  uint r = 0;

  for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
       client != cpu.end();
       client++) {
    if ((*client)->GetState() == DistantComputer::STATE_INITIALIZED)
      r++;
  }

  return r;
}

uint NetworkServer::GetNbReadyPlayers() const
{
  uint r = 0;

  for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
       client != cpu.end();
       client++) {
    if ((*client)->GetState() == DistantComputer::STATE_READY)
      r++;
  }

  return r;
}

uint NetworkServer::GetNbCheckedPlayers() const
{
  uint r = 0;

  for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
       client != cpu.end();
       client++) {
    if ((*client)->GetState() == DistantComputer::STATE_CHECKED)
      r++;
  }

  return r;
}
