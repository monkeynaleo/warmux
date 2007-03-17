/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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

#include "network.h"
//-----------------------------------------------------------------------------
#include "../game/game_mode.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
#include "distant_cpu.h"

#if defined(DEBUG) && not defined(WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
//-----------------------------------------------------------------------------

// Standard header, only needed for the following method
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#else
#include <winsock.h>
#endif
//-----------------------------------------------------------------------------

ConnectionState Network::CheckHost(const std::string &host, const std::string& port)
{
  MSG_DEBUG("network", "Checking connection to %s:%s", host.c_str(), port.c_str());
  int prt=0;
  sscanf(port.c_str(),"%i",&prt);

  struct hostent* hostinfo;
  hostinfo = gethostbyname(host.c_str());
  if( ! hostinfo )
    return CONN_BAD_HOST;

#ifndef WIN32
  int fd = socket(AF_INET, SOCK_STREAM, 0); 
  if( fd == -1 )
    return CONN_BAD_SOCKET;

#else
  SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
  if( fd == INVALID_SOCKET )
    return CONN_BAD_SOCKET;
#endif

  // Set the timeout
  struct timeval timeout;
  memset(&timeout, 0, sizeof(timeout));
  timeout.tv_sec = 5; // 5seconds timeout
#ifndef WIN32
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(timeout));
  setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout, sizeof(timeout));
#else
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
  setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
#endif

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
#ifndef WIN32
  addr.sin_addr.s_addr = *(in_addr_t*)*hostinfo->h_addr_list;
#else
  addr.sin_addr.s_addr = inet_addr(*hostinfo->h_addr_list);
#endif

  addr.sin_port = htons(prt);

#ifndef WIN32
  if( connect(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1 )
  {
    switch(errno)
    {
    case ECONNREFUSED: return CONN_REJECTED;
    case EINPROGRESS:
    case ETIMEDOUT: return CONN_TIMEOUT;
    default: return CONN_BAD_SOCKET;
    }
  }
  close(fd);
#else
  if( connect(fd, (struct sockaddr*) &addr, sizeof(addr)) != 0 )
  {
    switch(WSAGetLastError())
    {
    case WSAECONNREFUSED: return CONN_REJECTED;
    case WSAEINPROGRESS:
    case WSAETIMEDOUT: return CONN_TIMEOUT;
    default: return CONN_BAD_SOCKET;
    }
  }
  closesocket(fd);
#endif
  return CONNECTED;
}

ConnectionState Network::ClientConnect(const std::string &host, const std::string& port)
{
  MSG_DEBUG("network", "Client connect to %s:%s", host.c_str(), port.c_str());

  int prt=0;
  sscanf(port.c_str(),"%i",&prt);

  if( CheckHost(host, port) == CONN_TIMEOUT)
    return CONN_TIMEOUT;

  if(SDLNet_ResolveHost(&ip,host.c_str(),(Uint16)prt)==-1)
  {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return CONN_BAD_HOST;
  }

  TCPsocket socket = SDLNet_TCP_Open(&ip);

  if(!socket)
  {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return CONN_REJECTED;
  }

  m_connection = NETWORK_CLIENT;

  socket_set = SDLNet_AllocSocketSet(1);
  cpu.push_back(new DistantComputer(socket));
  //Send nickname to server
  Action a(Action::ACTION_NICKNAME, nickname);
  network.SendAction(&a);

  //Control to net_thread_func
  thread = SDL_CreateThread(net_thread_func,NULL);
  return CONNECTED;
}
