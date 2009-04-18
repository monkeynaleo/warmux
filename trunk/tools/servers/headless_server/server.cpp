/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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

#include <WORMUX_error.h>
#include <WORMUX_action.h>
#include <server.h>

NetworkGame::NetworkGame(const std::string& _game_name, const std::string& _password) :
  game_name(_game_name), password(_password), game_started(false)
{
}

const std::string& NetworkGame::GetName() const
{
  return game_name;
}

const std::string& NetworkGame::GetPassword() const
{
  return password;
}

void NetworkGame::AddCpu(DistantComputer* cpu)
{
  cpulist.push_back(cpu);
}

std::list<DistantComputer*>& NetworkGame::GetCpus()
{
  return cpulist;
}

const std::list<DistantComputer*>& NetworkGame::GetCpus() const
{
  return cpulist;
}

bool NetworkGame::AcceptNewComputers() const
{
  if (game_started || cpulist.size() >= 4) {
    DPRINT(INFO, "Game %s denies connexion", game_name.c_str());
    return false;
  }

  return true;
}

std::list<DistantComputer*>::iterator
NetworkGame::CloseConnection(std::list<DistantComputer*>::iterator closed)
{
  std::list<DistantComputer*>::iterator it;

  it = cpulist.erase(closed);
  delete *closed;


  return it;
}

uint NetworkGame::NextPlayerId() const
{
  uint player_id = 1;

  std::list<DistantComputer*>::const_iterator it;
  std::list<Player>::const_iterator player;
  for (it = cpulist.begin(); it != cpulist.end(); it++) {

    const std::list<Player>& players = (*it)->GetPlayers();

    for (player = players.begin(); player != players.end(); player++) {
      if (player_id <= player->GetId()) {
	player_id = player->GetId() + 1;
      }
    }
  }
  return player_id;
}

void NetworkGame::ElectGameMaster()
{
  if (cpulist.empty())
    return;

  DistantComputer* host = cpulist.front();

  DPRINT(INFO, "New game master: %s", host->GetAddress().c_str());

  Action a(Action::ACTION_NETWORK_SET_GAME_MASTER);
  SendActionToOne(a, host);
}

// Send Messages
void NetworkGame::SendActionToAll(const Action& a) const
{
  SendAction(a, NULL, false);
}

void NetworkGame::SendActionToOne(const Action& a, DistantComputer* client) const
{
  SendAction(a, client, true);
}

void NetworkGame::SendActionToAllExceptOne(const Action& a, DistantComputer* client) const
{
  SendAction(a, client, false);
}

// if (client == NULL) sending to every clients
// if (clt_as_rcver) sending only to client 'client'
// if (!clt_as_rcver) sending to all EXCEPT client 'client'
void NetworkGame::SendAction(const Action& a, DistantComputer* client, bool clt_as_rcver) const
{
  char* packet;
  int size;

  a.WriteToPacket(packet, size);
  ASSERT(packet != NULL);

  if (clt_as_rcver) {
    ASSERT(client);
    client->SendData(packet, size);
  } else {

    for (std::list<DistantComputer*>::const_iterator it = cpulist.begin();
	 it != cpulist.end(); it++) {

      if ((*it) != client) {
	(*it)->SendData(packet, size);
      }
    }
  }

  free(packet);
}

void NetworkGame::ForwardPacket(void * buffer, size_t len, DistantComputer* sender)
{
  std::list<DistantComputer*>::iterator it;

  for (it = cpulist.begin(); it != cpulist.end(); it++) {

    if ((*it) != sender) {
      (*it)->SendData(buffer, len);
    }
  }

  if (sender == cpulist.front()) {
    Action a(reinterpret_cast<const char*>(buffer), sender);
    if (a.GetType() == Action::ACTION_NETWORK_MASTER_CHANGE_STATE) {
      int net_state = a.PopInt();
      if (net_state == WNet::NETWORK_LOADING_DATA)
	game_started = true;
      else if (net_state == WNet::NETWORK_NEXT_GAME)
	game_started = false;
    }
  }
}

GameServer::GameServer() :
  game_name("dedicated"),
  password(""),
  port(0),
  clients_socket_set(NULL)
{
  CreateGame(1);
}

void GameServer::CreateGame(uint game_id)
{
  char gamename_c_str[32];
  snprintf(gamename_c_str, 32, "%s-%d", game_name.c_str(), game_id);
  std::string gamename_str(gamename_c_str);

  NetworkGame netgame(gamename_str, password);
  games.insert(std::make_pair(game_id, netgame));

  DPRINT(INFO, "Game - %s - created", gamename_str.c_str());
}

void GameServer::DeleteGame(std::map<uint, NetworkGame>::iterator gamelst_it)
{
  DPRINT(INFO, "Game - %s - deleted", gamelst_it->second.GetName().c_str());
  games.erase(gamelst_it);
}

const NetworkGame& GameServer::GetGame(uint game_id) const
{
  std::map<uint, NetworkGame>::const_iterator gamelst_it;
  gamelst_it = games.find(game_id);

  if (gamelst_it == games.end()) {
    ASSERT(false);
  }
  return (gamelst_it->second);
}

NetworkGame& GameServer::GetGame(uint game_id)
{
  std::map<uint, NetworkGame>::iterator gamelst_it;
  gamelst_it = games.find(game_id);

  if (gamelst_it == games.end()) {
    ASSERT(false);
  }
  return (gamelst_it->second);
}

const std::list<DistantComputer*>& GameServer::GetCpus(uint game_id) const
{
  return GetGame(game_id).GetCpus();
}

std::list<DistantComputer*>& GameServer::GetCpus(uint game_id)
{
  return GetGame(game_id).GetCpus();
}

bool GameServer::ServerStart(uint _port, uint _max_nb_games, uint max_nb_clients,
			     const std::string& _game_name, std::string& _password)
{
  max_nb_games = _max_nb_games;
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


bool GameServer::HandShake(uint game_id, WSocket& client_socket, std::string& nickname, uint player_id)
{
  bool client_will_be_master = false;
  if (GetCpus(game_id).empty())
    client_will_be_master = true;

  DPRINT(INFO, "%s will be master ? %d", client_socket.GetAddress().c_str(), client_will_be_master);
  return WNet::Server_HandShake(client_socket, GetGame(game_id).GetName(), GetGame(game_id).GetPassword(),
				nickname, player_id, client_will_be_master);
}

void GameServer::RejectIncoming()
{
  server_socket.Disconnect();
}

uint GameServer::CreateGameIfNeeded()
{
  if (games.size() >= max_nb_games) {
    DPRINT(INFO, "Max number of games already reached : %d", max_nb_games);
    return 0;
  }

  uint game_id = 0;
  std::map<uint, NetworkGame>::const_iterator gamelst_it;
  for (gamelst_it = games.begin(); gamelst_it != games.end(); gamelst_it++) {

    if (gamelst_it->second.AcceptNewComputers()) {
      return gamelst_it->first;
    }

    if (gamelst_it->first > game_id)
      game_id = gamelst_it->first;
  }

  game_id++;
  CreateGame(game_id);

  return game_id;
}

void GameServer::WaitClients()
{
  if (!server_socket.IsConnected())
    return;

  // Create a new game if there is no more games accepting players
  uint game_id = CreateGameIfNeeded();
  if (!game_id)
    return;

  // Check for an incoming connection
  WSocket* incoming = server_socket.LookForClient();
  if (incoming) {

    DPRINT(INFO, "Connexion from %s\n", incoming->GetAddress().c_str());

    std::string client_nickname;
    uint player_id = GetGame(game_id).NextPlayerId();

    if (!HandShake(game_id, *incoming, client_nickname, player_id)) {
      incoming->Disconnect();
      return;
    }

    clients_socket_set->AddSocket(incoming);

    DistantComputer* client = new DistantComputer(incoming, client_nickname, game_id, player_id);
    GetGame(game_id).AddCpu(client);

    if (clients_socket_set->NbSockets() == clients_socket_set->MaxNbSockets())
      RejectIncoming();
  }
}

void GameServer::RunLoop()
{
 loop:
  while (true) {

    WaitClients();

    int num_ready = clients_socket_set->CheckActivity(100);

    if (num_ready == -1) { // Means an error
      fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
      continue; //Or break?
    }

    void * buffer;
    size_t packet_size;

    std::map<uint, NetworkGame>::iterator gamelst_it;
    std::list<DistantComputer*>::iterator dst_cpu;

    for (gamelst_it = games.begin(); gamelst_it != games.end(); gamelst_it++) {

      for (dst_cpu = gamelst_it->second.GetCpus().begin();
	   dst_cpu != gamelst_it->second.GetCpus().end();
	   dst_cpu++) {

	if ((*dst_cpu)->SocketReady()) {// Check if this socket contains data to receive

	  if (!(*dst_cpu)->ReceiveData(reinterpret_cast<void* &>(buffer), packet_size)) {
	    // An error occured during the reception

	    bool turn_master_lost = (dst_cpu == gamelst_it->second.GetCpus().begin());
	    dst_cpu = gamelst_it->second.CloseConnection(dst_cpu);

	    if (clients_socket_set->NbSockets() + 1 == clients_socket_set->MaxNbSockets()) {
	      // A new player will be able to connect, so we reopen the socket
	      // For incoming connections
	      DPRINT(INFO, "Allowing new connections (%d/%d)",
		     clients_socket_set->NbSockets(), clients_socket_set->MaxNbSockets());
	      server_socket.AcceptIncoming(port);
	    }

	    if (gamelst_it->second.GetCpus().size() != 0) {
	      if (turn_master_lost) {
		GetGame(gamelst_it->first).ElectGameMaster();
	      }
	    } else {
	      DeleteGame(gamelst_it);
	      goto loop;
	    }

	  } else {

	    GetGame(gamelst_it->first).ForwardPacket(buffer, packet_size, *dst_cpu);
	    free(buffer);
	  }
	}
      }
    } // for
  }
}

uint Action_TimeStamp()
{
  return 0;
}

void WORMUX_ConnectHost(DistantComputer& host)
{
  std::string hostname = host.GetAddress();
  std::string nicknames = host.GetNicknames();

  DPRINT(INFO, "New client connected: %s (%s)", nicknames.c_str(), hostname.c_str());

  Action a(Action::ACTION_INFO_CLIENT_CONNECT);
  a.Push(hostname);
  a.Push(nicknames);

  GameServer::GetInstance()->GetGame(host.GetGameId()).SendActionToAllExceptOne(a, &host);
}

void WORMUX_DisconnectHost(DistantComputer& host)
{
  std::string hostname = host.GetAddress();
  std::string nicknames = host.GetNicknames();

  DPRINT(INFO, "Client disconnected: %s (%s)", nicknames.c_str(), hostname.c_str());

  Action a(Action::ACTION_INFO_CLIENT_DISCONNECT);
  a.Push(hostname);
  a.Push(nicknames);
  a.Push(int(host.GetPlayers().size()));

  std::list<Player>::const_iterator player_it;
  for (player_it = host.GetPlayers().begin(); player_it != host.GetPlayers().end(); player_it++) {
    a.Push(int(player_it->GetId()));
  }
  GameServer::GetInstance()->GetGame(host.GetGameId()).SendActionToAll(a); // host is already removed from the list
}

void WORMUX_DisconnectPlayer(Player& /*player*/)
{

}

