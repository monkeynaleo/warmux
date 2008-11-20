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
#include "map/maps_list.h"
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

NetworkServer::NetworkServer(const std::string& game_name, const std::string& password) :
  Network(game_name, password),
  port(0)
{
#ifdef LOG_NETWORK
  fin = open("./network_server.in", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
  fout = open("./network_server.out", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
#endif
  game_master_player = true;
}

NetworkServer::~NetworkServer()
{
}

void NetworkServer::HandleAction(Action* a, DistantComputer* sender)
{
  // Repeat the packet to other clients
  SendActionToAllExceptOne(*a, sender);

  ActionHandler::GetInstance()->NewAction(a, false);
}

static inline void add_team_config_to_action(Action& a, std::map<std::string, ConfigTeam>::const_iterator &team)
{
  a.Push(team->first);
  a.Push(team->second.player_name);
  a.Push(int(team->second.nb_characters));
}

void NetworkServer::SendInitialGameInfo(DistantComputer* client)
{
  // we have to tell this new computer
  // what teams / maps have already been selected

  MSG_DEBUG("network", "Server: Sending map information");

  Action a(Action::ACTION_GAME_INFO);
  MapsList::GetInstance()->FillActionMenuSetMap(a);

  MSG_DEBUG("network", "Server: Sending teams information");

  // count the number of already selected teams
  int nb_teams = 0;

  std::list<DistantComputer*>::const_iterator it;
  for (it = cpu.begin(); it != cpu.end(); it++) {
    nb_teams += (*it)->GetPlayer().GetNbTeams();
  }

  nb_teams += GetPlayer().GetNbTeams();

  a.Push(nb_teams);

  // Teams infos of already connected computers
  std::map<std::string, ConfigTeam>::const_iterator team;

  for (it = cpu.begin(); it != cpu.end(); it++) {
    for (team = (*it)->GetPlayer().GetTeams().begin();
	 team != (*it)->GetPlayer().GetTeams().end();
	 team++) {

      add_team_config_to_action(a, team);
    }
  }

  for (team = GetPlayer().GetTeams().begin(); team != GetPlayer().GetTeams().end(); team++) {
    add_team_config_to_action(a, team);
  }

  SendActionToOne(a, client);
}

bool NetworkServer::HandShake(WSocket& client_socket, std::string& nickname) const
{
  return WNet::Server_HandShake(client_socket, GetGameName(), GetPassword(), nickname, false);
}

void NetworkServer::WaitActionSleep()
{
  if (server_socket.IsConnected()) {

    // Check for an incoming connection
    WSocket* incoming = server_socket.LookForClient();
    if (incoming) {
      std::string nickname;

      if (!HandShake(*incoming, nickname))
 	return;

      socket_set->AddSocket(incoming);

      DistantComputer* client = new DistantComputer(incoming, nickname);
      SendInitialGameInfo(client);
      cpu.push_back(client);

      Action *a = new Action(Action::ACTION_INFO_CLIENT_CONNECT);
      a->Push(client->GetAddress());
      a->Push(client->GetPlayer().GetNickname());

      ActionHandler::GetInstance()->NewAction(a);

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

connection_state_t NetworkServer::StartServer(const std::string &net_port, uint _max_nb_players)
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

  NetworkThread::Start();
  return CONNECTED;
}

std::list<DistantComputer*>::iterator
NetworkServer::CloseConnection(std::list<DistantComputer*>::iterator closed)
{
  std::list<DistantComputer*>::iterator it;

  printf("- client disconnected: %s\n", (*closed)->ToString().c_str());

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
