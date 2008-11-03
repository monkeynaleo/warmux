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
 * Notify an index server of an opened wormux server
 * Obtain information about running games from an index server
 *****************************************************************************/

#include <assert.h>
#include <SDL_net.h>
#include "network/download.h"
#include "game/config.h"
#include "graphic/video.h"
#include "include/app.h"
#include "include/constant.h"
#include "network/index_server.h"
#include "network/index_svr_msg.h"
#include "network/network.h"
#include "tool/debug.h"
#include "tool/random.h"

IndexServer::IndexServer():
  used(0),
  server_lst(),
  first_server(server_lst.end()),
  current_server(server_lst.end()),
  hidden_server(false)
{
}

IndexServer::~IndexServer()
{
  server_lst.clear();
  if (IsConnected())
    Disconnect();
}

bool IndexServer::IsConnected()
{
  return socket.IsConnected();
}


/*************  Connection  /  Disconnection  ******************/
connection_state_t IndexServer::Connect()
{
  connection_state_t r = CONN_REJECTED;

  MSG_DEBUG("index_server", "Connecting..");
  ASSERT(!IsConnected());

  if (hidden_server)
    return CONNECTED;

  // Download the server if it's empty
  if (server_lst.size() == 0) {
    server_lst = Downloader::GetInstance()->GetServerList("server_list");
    first_server = server_lst.end();
    current_server = server_lst.end();
  }

  // If it's still empty, then something went wrong when downloading it
  if (server_lst.size() == 0)
    return CONN_BAD_HOST;

  std::string addr;
  int port;
  uint nb_servers_tried = 0; // how many servers have we tried to connect ?

  // Cycle through the list of server
  // Until we find one running
  while (GetServerAddress(addr, port, nb_servers_tried))
  {
    r = ConnectTo(addr, port);
    if (r == CONNECTED)
      return r;
  }

  // Undo what was done
  Disconnect();

  return r;
}

connection_state_t IndexServer::ConnectTo(const std::string & address, const int & port)
{
  connection_state_t r;

  MSG_DEBUG("index_server", "Connecting to %s %i", address.c_str(), port);

  r = socket.ConnectTo(address, port);
  if (r != CONNECTED) {
    goto error;
  }

  if (!socket.AddToTmpSocketSet()) {
    r = CONN_REJECTED;
    goto error;
  }

  r = HandShake();
  if (r != CONNECTED)
    goto error;

  return r;

 error:
  socket.Disconnect();
  return r;
}

void IndexServer::Disconnect()
{
  if (hidden_server)
  {
    hidden_server = false;
    return;
  }

  first_server = server_lst.end();
  current_server = server_lst.end();

  if (!IsConnected())
    return;

  MSG_DEBUG("index_server", "Closing connection");

  socket.Disconnect();
}

bool IndexServer::GetServerAddress( std::string & address, int & port, uint & nb_servers_tried)
{
  // have we already tried all servers ?
  if (server_lst.size() == nb_servers_tried) {
    return false;
  }
  nb_servers_tried++;

  MSG_DEBUG("index_server", "Trying a new server");
  // Cycle through the server list to find the first one
  // accepting connection
  if (first_server == server_lst.end())
    {
      // First try :
      // Randomly select a server in the list
      int nbr = RandomLocal().GetLong( 0, server_lst.size()-1 );
      first_server = server_lst.begin();
      while(nbr--)
	++first_server;

      ASSERT(first_server != server_lst.end());

      current_server = first_server;

      address = current_server->first;
      port = current_server->second;
      return true;
    }

  ++current_server;
  if (current_server == server_lst.end())
    current_server = server_lst.begin();

  address = current_server->first;
  port = current_server->second;

  return (current_server != first_server);
}

/*************  Basic transmissions  ******************/
void IndexServer::NewMsg(IndexServerMsg msg_id)
{
  assert(used == 0);
  Batch((int)msg_id);
  // Reserve 4 bytes for the total message length.
  used += 4;
}

void IndexServer::Batch(const int& nbr)
{
  assert(used+4 < INDEX_SERVER_BUFFER_LENGTH);
  used += WNet::Batch(buffer+used, nbr);
}

void IndexServer::Batch(const std::string &str)
{
  assert(used+4+str.size() < INDEX_SERVER_BUFFER_LENGTH);
  used += WNet::Batch(buffer+used, str);
}

bool IndexServer::SendMsg()
{
  WNet::FinalizeBatch(buffer, used);

  bool r = socket.SendBuffer(buffer, used);
  used = 0;
  return r;
}

connection_state_t IndexServer::HandShake()
{
  connection_state_t status = CONN_REJECTED;
  bool r;
  int msg;
  std::string sign;

  MSG_DEBUG("index_server", "Beginning handshake...");

  NewMsg(TS_MSG_VERSION);
  Batch(Constants::WORMUX_VERSION);

  MSG_DEBUG("index_server", "Sending information...");

  r = SendMsg();
  if (!r)
    goto error;

  MSG_DEBUG("index_server", "Receiving...");

  r = socket.ReceiveInt(msg);
  if (!r || msg != TS_MSG_VERSION)
    goto error;

  MSG_DEBUG("index_server", "Received: %d", msg);

  MSG_DEBUG("index_server", "Receiving...");
  r = socket.ReceiveStr(sign, 20);
  if (!r)
    goto error;

  MSG_DEBUG("index_server", "Received: %s", sign.c_str());

  if (sign == "Bad version") {
    status = CONN_WRONG_VERSION;
    r = socket.ReceiveStr(sign, 20);
    if (!r)
      sign = "";
    AppWormux::DisplayError(Format(_("Sorry, your version is not supported anymore. "
				     "Supported version are %s. "
				     "You can download a updated version "
				     "on http://www.wormux.org/wiki/download.php"),
				   sign.c_str()));
    goto error;
  }

  if (sign != "MassMurder!")
    goto error;

  MSG_DEBUG("index_server", "Handshake : OK");

  status = CONNECTED;
  return status;

 error:
  MSG_DEBUG("index_server", "Handshake : ERROR!");
  return status;
}

bool IndexServer::SendServerStatus(const std::string& game_name, bool pwd, int port)
{
  std::string ack;
  ASSERT(Network::GetInstance()->IsServer());

  if (hidden_server)
    return true;

  NewMsg(TS_MSG_REGISTER_GAME);
  Batch(game_name);
  Batch((int)pwd);
  SendMsg();
  NewMsg(TS_MSG_HOSTING);
  Batch(port);
  SendMsg();

  bool r = socket.ReceiveStr(ack, 5);
  if (r && ack == "OK")
    return true;

  Disconnect();
  return false;
}

std::list<GameServerInfo> IndexServer::GetHostList()
{
  std::list<GameServerInfo> lst;
  bool r;

  NewMsg(TS_MSG_GET_LIST);
  SendMsg();

  int lst_size;
  r = socket.ReceiveInt(lst_size);
  if (!r || lst_size == 0)
    return lst;

  while (lst_size--)
  {
    GameServerInfo game_server_info;
    IPaddress ip;
    int nb;
    r = socket.ReceiveInt(nb);
    if (!r)
      return lst;
    ip.host = nb;

    r = socket.ReceiveInt(nb);
    if (!r)
      return lst;
    ip.port = nb;

    r = socket.ReceiveInt(nb);
    if (!r)
      return lst;

    game_server_info.passworded = !!(nb);

    r = socket.ReceiveStr(game_server_info.game_name, 40);
    if (!r)
      return lst;

    const char* dns_addr = SDLNet_ResolveIP(&ip);
    char port[10];
    sprintf(port, "%d", ip.port);
    game_server_info.port = std::string(port);

    // We can't resolve the hostname, so just show the ip address
    unsigned char* str_ip = (unsigned char*)&ip.host;
    char formated_ip[16];
    snprintf(formated_ip, 16, "%i.%i.%i.%i", (int)str_ip[0],
	     (int)str_ip[1],
	     (int)str_ip[2],
	     (int)str_ip[3]);
    game_server_info.ip_address = std::string(formated_ip);

    if (dns_addr != NULL)
      game_server_info.dns_address = std::string(dns_addr);
    else
      game_server_info.dns_address = game_server_info.ip_address;

    MSG_DEBUG("index_server","ip: %s, port: %s, dns: %s, name: %s, pwd=%s\n",
	      game_server_info.ip_address.c_str(),
	      game_server_info.port.c_str(),
	      game_server_info.dns_address.c_str(),
	      game_server_info.game_name.c_str(),
              (game_server_info.passworded) ? "yes" : "no");

    lst.push_back(game_server_info);
  }
  return lst;
}

void IndexServer::Refresh()
{
  if (!socket.IsReady(100))
    return;

  int msg_id;
  bool r;
  r = socket.ReceiveInt(msg_id);

  if (r && msg_id == TS_MSG_PING) {
    NewMsg(TS_MSG_PONG);
    SendMsg();
    return;
  }

  Disconnect();
}
