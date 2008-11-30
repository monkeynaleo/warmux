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

#include <WORMUX_action.h>
#include <WORMUX_distant_cpu.h>
#include <WORMUX_network.h>
#include <WORMUX_socket.h>
#include <WORMUX_singleton.h>
#include <WORMUX_types.h>
#include <WSERVER_config.h>
#include <WSERVER_debug.h>

class GameServer : public Singleton<GameServer>
{
private:
  friend class Singleton<GameServer>;
  GameServer();

  std::string game_name;
  std::string password;
  uint port;

  WSocket server_socket;
  WSocketSet* clients_socket_set;
  std::list<DistantComputer*> cpu; // list of the connected computer

  uint NextPlayerId() const;
  bool HandShake(WSocket& client_socket, std::string& client_nickname, uint player_id);
  void WaitClients();
  void RejectIncoming();
  std::list<DistantComputer*>::iterator CloseConnection(std::list<DistantComputer*>::iterator closed);
  void ElectGameMaster();

  void ForwardPacket(void * buffer, size_t len, const DistantComputer* sender);
  void SendAction(const Action& a, DistantComputer* client, bool clt_as_rcver) const;
public:
  bool ServerStart(uint port, uint max_nb_clients, const std::string& game_name, std::string& password);
  void RunLoop();

  // Action handling
  void SendActionToAll(const Action& action) const;
  void SendActionToOne(const Action& action, DistantComputer* client) const;
  void SendActionToAllExceptOne(const Action& action, DistantComputer* client) const;
};

uint Action_TimeStamp();
void WORMUX_ConnectHost(DistantComputer& host);
void WORMUX_DisconnectHost(DistantComputer& host);
void WORMUX_DisconnectPlayer(Player& player);

#endif
