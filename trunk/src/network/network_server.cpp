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
#include "include/constant.h"
#include "game/game_mode.h"
#include "tool/debug.h"
#include "network/distant_cpu.h"

#include <sys/types.h>
#ifdef LOG_NETWORK
#  include <sys/stat.h>
#  include <fcntl.h>
#  ifdef WIN32
#    include <io.h>
#  endif
#endif

//-----------------------------------------------------------------------------

NetworkServer::NetworkServer(const std::string& password) : Network(password)
{
#ifdef LOG_NETWORK
  fin = open("./network_server.in", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
  fout = open("./network_server.out", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
#endif
}

NetworkServer::~NetworkServer()
{
  SDLNet_TCP_Close(server_socket);
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
  if (server_socket)
  {
    // Check for an incoming connection
    TCPsocket incoming = SDLNet_TCP_Accept(server_socket);
    if (incoming)
    {
      WSocket* socket = new WSocket(incoming);

      if (!HandShake(*socket))
 	return;

      socket->AddToSocketSet(socket_set);

      DistantComputer * client = new DistantComputer(socket);
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
//-----------------------------------------------------------------------------

connection_state_t NetworkServer::ServerStart(const std::string &port)
{
  WNet::Init();

  // The server starts listening for clients
  MSG_DEBUG("network", "Start server on port %s", port.c_str());

  cpu.clear();
  // Convert port number (std::string port) into SDL port number format:
  int prt = strtol(port.c_str(), NULL, 10);

  if (SDLNet_ResolveHost(&ip,NULL,(Uint16)prt) != 0)
  {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return CONN_BAD_PORT;
  }

  max_nb_players = GameMode::GetInstance()->max_teams;

  // Open the port to listen to
  if (!AcceptIncoming()) {
    return CONN_BAD_PORT;
  }
  printf("\nConnected\n");
  socket_set = SDLNet_AllocSocketSet(GameMode::GetInstance()->max_teams);
  thread = SDL_CreateThread(Network::ThreadRun, NULL);
  MSG_DEBUG("network", "Thread %u created by thread %u\n", SDL_GetThreadID(thread), SDL_ThreadID());
  return CONNECTED;
}

std::list<DistantComputer*>::iterator
NetworkServer::CloseConnection(std::list<DistantComputer*>::iterator closed)
{
  printf("Client disconnected\n");
  delete *closed;
  if (GetNbConnectedPlayers() == max_nb_players)
  {
    // A new player will be able to connect, so we reopen the socket
    // For incoming connections
    printf("Allowing new connections\n");
    AcceptIncoming();
  }

  return cpu.erase(closed);
}

bool NetworkServer::AcceptIncoming()
{
  server_socket = SDLNet_TCP_Open(&ip);
  if (!server_socket)
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return false;
  }
  fprintf(stderr, "\n-> Start listening");
  return true;
}

void NetworkServer::RejectIncoming()
{
  if (!server_socket)
    return;

  SDLNet_TCP_Close(server_socket);
  server_socket = NULL;
  fprintf(stderr, "\n<- Stop listening\n");
}

void NetworkServer::SetMaxNumberOfPlayers(uint _max_nb_players)
{
  if (_max_nb_players <= GameMode::GetInstance()->max_teams) {
    max_nb_players = _max_nb_players;
  } else {
    max_nb_players = GameMode::GetInstance()->max_teams;
  }
}

uint NetworkServer::GetNbConnectedPlayers() const
{
  return cpu.size() + 1;
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
