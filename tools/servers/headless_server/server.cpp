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

#include <WORMUX_error.h>
#include <server.h>

GameServer::GameServer() :
  password(""),
  port(0),
  clients_socket_set(NULL)
{
  std::list<DistantComputer *> dst_cpus;
  cpu.insert(std::make_pair(0, dst_cpus));
}

const std::list<DistantComputer*>& GameServer::GetCpus(uint game_id) const
{
  std::map<uint, std::list<DistantComputer*> >::const_iterator cpulst_it;
  cpulst_it = cpu.find(game_id);

  if (cpulst_it == cpu.end()) {
    ASSERT(false);
  }

  return (cpulst_it->second);
}

std::list<DistantComputer*>& GameServer::GetCpus(uint game_id)
{
  std::map<uint, std::list<DistantComputer*> >::iterator cpulst_it;
  cpulst_it = cpu.find(game_id);

  if (cpulst_it == cpu.end()) {
    ASSERT(false);
  }

  return (cpulst_it->second);
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

// Send Messages
void GameServer::SendActionToAll(uint game_id, const Action& a) const
{
  SendAction(game_id, a, NULL, false);
}

void GameServer::SendActionToOne(uint game_id, const Action& a, DistantComputer* client) const
{
  SendAction(game_id, a, client, true);
}

void GameServer::SendActionToAllExceptOne(uint game_id, const Action& a, DistantComputer* client) const
{
  SendAction(game_id, a, client, false);
}

// if (client == NULL) sending to every clients
// if (clt_as_rcver) sending only to client 'client'
// if (!clt_as_rcver) sending to all EXCEPT client 'client'
void GameServer::SendAction(uint game_id, const Action& a, DistantComputer* client, bool clt_as_rcver) const
{
  char* packet;
  int size;

  a.WriteToPacket(packet, size);
  ASSERT(packet != NULL);

  if (clt_as_rcver) {
    ASSERT(client);
    client->SendData(packet, size);
  } else {

    for (std::list<DistantComputer*>::const_iterator it = GetCpus(game_id).begin();
	 it != GetCpus(game_id).end(); it++) {

      if ((*it) != client) {
	(*it)->SendData(packet, size);
      }
    }
  }

  free(packet);
}

uint GameServer::NextPlayerId(uint game_id) const
{
  uint player_id = 1;

  std::list<DistantComputer*>::const_iterator it;
  std::list<Player>::const_iterator player;
  for (it = GetCpus(game_id).begin(); it != GetCpus(game_id).end(); it++) {

    const std::list<Player>& players = (*it)->GetPlayers();

    for (player = players.begin(); player != players.end(); player++) {
      if (player_id <= player->GetId()) {
	player_id = player->GetId() + 1;
      }
    }
  }
  return player_id;
}

bool GameServer::HandShake(WSocket& client_socket, std::string& nickname, uint player_id)
{
  bool client_will_be_master = false;
  if (clients_socket_set->NbSockets() == 0)
    client_will_be_master = true;

  DPRINT(INFO, "%s will be master ? %d", client_socket.GetAddress().c_str(), client_will_be_master);
  return WNet::Server_HandShake(client_socket, game_name, password, nickname, player_id, client_will_be_master);
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
      uint player_id = NextPlayerId(0);

      if (!HandShake(*incoming, client_nickname, player_id))
 	return;

      clients_socket_set->AddSocket(incoming);

      uint game_id = 0;
      std::map<uint, std::list<DistantComputer*> >::const_reverse_iterator cpulst_it;
      cpulst_it = cpu.rbegin();

      if (cpulst_it != cpu.rend()) {
	game_id = cpulst_it->first;

	if (cpulst_it->second.size() >= 2) {
	  game_id = cpulst_it->first + 1;
	}
      }

      DistantComputer* client = new DistantComputer(incoming, client_nickname, player_id);
      cpu[game_id].push_back(client);

      if (clients_socket_set->NbSockets() == clients_socket_set->MaxNbSockets())
	RejectIncoming();
    }
    SDL_Delay(100);
  }
}

std::list<DistantComputer*>::iterator
GameServer::CloseConnection(uint game_id, std::list<DistantComputer*>::iterator closed)
{
  std::list<DistantComputer*>::iterator it;

  it = cpu[game_id].erase(closed);
  delete *closed;

  if (clients_socket_set->NbSockets() + 1 == clients_socket_set->MaxNbSockets()) {
    // A new player will be able to connect, so we reopen the socket
    // For incoming connections
    DPRINT(INFO, "Allowing new connections (%d/%d)",
	   clients_socket_set->NbSockets(), clients_socket_set->MaxNbSockets());
    server_socket.AcceptIncoming(port);
  }

  return it;
}

void GameServer::ForwardPacket(uint game_id, void * buffer, size_t len, const DistantComputer* sender)
{
  std::list<DistantComputer*>::iterator it;

  for (it = GetCpus(game_id).begin(); it != GetCpus(game_id).end(); it++) {

    if ((*it) != sender) {
      (*it)->SendData(buffer, len);
    }
  }
}

void GameServer::ElectGameMaster(uint game_id)
{
  if (GetCpus(game_id).empty())
    return;

  DistantComputer* host = GetCpus(game_id).front();

  DPRINT(INFO, "New game master: %s", host->GetAddress().c_str());

  Action a(Action::ACTION_NETWORK_SET_GAME_MASTER);
  GameServer::GetInstance()->SendActionToOne(game_id, a, host);
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

    std::map<uint, std::list<DistantComputer*> >::iterator cpulst_it;
    std::list<DistantComputer*>::iterator dst_cpu;

    for (cpulst_it = cpu.begin(); cpulst_it != cpu.end(); cpulst_it++) {

      for (dst_cpu = cpulst_it->second.begin();
	   dst_cpu != cpulst_it->second.end();
	   dst_cpu++) {

	if ((*dst_cpu)->SocketReady()) {// Check if this socket contains data to receive

	  if (!(*dst_cpu)->ReceiveData(reinterpret_cast<void* &>(buffer), packet_size)) {
	    // An error occured during the reception

	    bool turn_master_lost = (dst_cpu == cpulst_it->second.begin());
	    dst_cpu = CloseConnection(cpulst_it->first, dst_cpu);

	    if (turn_master_lost) {
	      ElectGameMaster(cpulst_it->first);
	    }

	  } else {

	    ForwardPacket(cpulst_it->first, buffer, packet_size, *dst_cpu);
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

  GameServer::GetInstance()->SendActionToAllExceptOne(host.GetGameId(), a, &host);
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
  GameServer::GetInstance()->SendActionToAll(host.GetGameId(), a); // host is already removed from the list
}

void WORMUX_DisconnectPlayer(Player& /*player*/)
{

}

