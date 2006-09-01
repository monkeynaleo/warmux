/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
 * Distant Computer handling
 *****************************************************************************/

#include "distant_cpu.h"
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <SDL_thread.h>
#include "../include/action_handler.h"
#include "../tool/debug.h"
#include "network.h"
//-----------------------------------------------------------------------------

DistantComputer::DistantComputer(TCPsocket new_sock)
  : sock(new_sock)
{
  SDLNet_TCP_AddSocket(network.socket_set, sock);
}

DistantComputer::~DistantComputer()
{
    SDLNet_TCP_DelSocket(network.socket_set, sock);
    SDLNet_TCP_Close(sock);
}

bool DistantComputer::SocketReady()
{
  return SDLNet_SocketReady(sock);
}

int DistantComputer::ReceiveDatas(char* & buf)
{
  // Firstly, we read the size of the incoming packet
  Uint32 net_size;
  if(SDLNet_TCP_Recv(sock, &net_size, 4) <= 0)
    return -1;

  int size = (int)SDLNet_Read32(&net_size);
  assert(size > 0);

  // Now we read the packet
  buf = (char*)malloc(size);

  int total_received = 0;
  while(total_received != size)
  {
    int received = SDLNet_TCP_Recv(sock, buf + total_received, size - total_received);
    if(received > 0)
      total_received += received;

    if(received < 0)
    {
      assert(false);
      std::cerr << "Malformed packet" << std::endl;
      total_received = received;
      break;
    }
  }
  return total_received;
}

void DistantComputer::SendDatas(char* packet, int size_tmp)
{
  Uint32 size;
  SDLNet_Write32(size_tmp, &size);
  SDLNet_TCP_Send(sock,&size,4);
  SDLNet_TCP_Send(sock,packet, size_tmp);
}

std::string DistantComputer::GetAdress()
{
  IPaddress* ip = SDLNet_TCP_GetPeerAddress(sock);
  std::string address = SDLNet_ResolveIP(ip);
  return address;
}
