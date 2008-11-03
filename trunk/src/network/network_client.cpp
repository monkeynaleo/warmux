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
 * Network client layer for Wormux.
 *****************************************************************************/

#include "network/network_client.h"
//-----------------------------------------------------------------------------
#include <WORMUX_socket.h>
#include <SDL_thread.h>
#include "include/action_handler.h"
#include "include/app.h"
#include "include/constant.h"
#include "game/game_mode.h"
#include "network/distant_cpu.h"
#include "network/net_error_msg.h"
#include "tool/debug.h"
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

NetworkClient::NetworkClient(const std::string& password) : Network(password)
{
#ifdef LOG_NETWORK
  fin = open("./network_client.in", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
  fout = open("./network_client.out", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP);
#endif
}

NetworkClient::~NetworkClient()
{
}

std::list<DistantComputer*>::iterator NetworkClient::CloseConnection(std::list<DistantComputer*>::iterator closed)
{
  printf("Client disconnected\n");
  delete *closed;

  return cpu.erase(closed);
}

void NetworkClient::HandleAction(Action* a, DistantComputer* /*sender*/) const
{
  ActionHandler::GetInstance()->NewAction(a, false);
}

//-----------------------------------------------------------------------------

connection_state_t NetworkClient::HandShake(WSocket& server_socket) const
{
  int ack;
  connection_state_t ret = CONN_REJECTED;
  std::string version;

  MSG_DEBUG("network", "Client: Handshake !");

  // Adding the socket to a temporary socket set
  if (!server_socket.AddToTmpSocketSet())
    goto error_no_socket_set;

  // 1) Send the version number
  MSG_DEBUG("network", "Client: sending version number");

  if (!server_socket.SendStr(Constants::WORMUX_VERSION))
    goto error;

  // is it ok ?
  if (!server_socket.ReceiveStr(version, 40))
    goto error;

  MSG_DEBUG("network", "Client: server version number is %s", version.c_str());

  if (Constants::WORMUX_VERSION != version) {
    std::string str = Format(_("The client and server versions are incompatible "
			       "(local=%s, server=%s). Please try another server."),
			     Constants::WORMUX_VERSION.c_str(), version.c_str());
    AppWormux::DisplayError(str);
    goto error;
  }

  // 2) Send the password

  MSG_DEBUG("network", "Client: sending password");
  if (!server_socket.SendStr(GetPassword()))
    goto error;

  // is it ok ?
  if (!server_socket.ReceiveInt(ack))
    goto error;

  if (ack) {
    ret = CONN_WRONG_PASSWORD;
    goto error;
  }

  MSG_DEBUG("network", "Client: Handshake done successfully :)");
  ret = CONNECTED;

 error:
  server_socket.RemoveFromTmpSocketSet();

 error_no_socket_set:
  if (ret != CONNECTED)
    std::cerr << "Client: HandShake with server has failed!" << std::endl;
  return ret;
}

connection_state_t
NetworkClient::ClientConnect(const std::string &host, const std::string& port)
{
  connection_state_t r;
  WSocket* socket;
  DistantComputer* server;
  Action a(Action::ACTION_NICKNAME, GetNickname());
  int prt;

  MSG_DEBUG("network", "Client connect to %s:%s", host.c_str(), port.c_str());

  if (!str2int(port, prt)) {
    r = CONN_BAD_PORT;
    goto error;
  }

  socket = new WSocket();
  r = socket->ConnectTo(host, prt);
  if (r != CONNECTED)
    goto error;

  r = HandShake(*socket);
  if (r != CONNECTED)
    goto error;

  socket_set = SDLNet_AllocSocketSet(1);
  if (!socket_set) {
    r = CONN_REJECTED;
    goto error;
  }

  socket->AddToSocketSet(socket_set);
  server = new DistantComputer(socket);

  cpu.push_back(server);

  //Send nickname to server
  SendAction(a);

  //Control to net_thread_func
  thread = SDL_CreateThread(Network::ThreadRun, NULL);
  return CONNECTED;

 error:
  delete socket;
  return r;
}
