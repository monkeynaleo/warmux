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
 * Socket abstraction
 *****************************************************************************/

#include <SDL_thread.h>
#include <SDL_timer.h>
#include "../include/WORMUX_error.h"
#include "../include/WORMUX_socket.h"

#include <iostream>
#include <sys/types.h>

//-----------------------------------------------------------------------------

WSocket::WSocket(TCPsocket _socket, SDLNet_SocketSet _socket_set) :
  socket(_socket),
  socket_set(_socket_set),
  lock(SDL_CreateMutex())
{
  int r;
  r = SDLNet_TCP_AddSocket(socket_set, socket);
  if (r == -1) {
    fprintf(stderr, "SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
    ASSERT(false);
  }
}

WSocket::~WSocket()
{
  SDLNet_TCP_Close(socket);
  SDLNet_TCP_DelSocket(socket_set, socket);
  SDL_DestroyMutex(lock);
}

void WSocket::Lock()
{
  SDL_LockMutex(lock);
}

void WSocket::UnLock()
{
  SDL_UnlockMutex(lock);
}


// Static methods usefull to communicate without action
// (index server, handshake, ...)

bool WSocket::SendInt_NoLock(const int& nbr)
{
  char packet[4];
  // this is not cute, but we don't want an int -> uint conversion here
  Uint32 u_nbr = *((const Uint32*)&nbr);

  SDLNet_Write32(u_nbr, packet);
  int len = SDLNet_TCP_Send(socket, packet, sizeof(packet));
  if (len < int(sizeof(packet)))
    return false;

  return true;
}

bool WSocket::SendInt(const int& nbr)
{
  bool r;

  Lock();
  r = SendInt_NoLock(nbr);
  UnLock();

  return r;
}

bool WSocket::SendStr_NoLock(const std::string &str)
{
  bool r = SendInt_NoLock(str.size());
  if (!r)
    return false;

  int len = SDLNet_TCP_Send(socket, (void*)str.c_str(), str.size());
  if (len < int(str.size()))
    return false;

  return true;
}

bool WSocket::SendStr(const std::string &str)
{
  bool r;

  Lock();
  r = SendStr_NoLock(str);
  UnLock();

  return r;
}

bool WSocket::SendBuffer_NoLock(void* data, size_t len)
{
  int size = SDLNet_TCP_Send(socket, data, len);
  if (size < int(len)) {
    return false;
  }
  return true;
}

bool WSocket::SendBuffer(void* data, size_t len)
{
  bool r;

  Lock();
  r = SendBuffer_NoLock(data, len);
  UnLock();

  return r;
}

bool WSocket::ReceiveBuffer_NoLock(void* data, size_t len)
{
  int received = 0;

  // Receive data of *exactly* length "len" bytes from the socket "socket",
  // into the memory pointed to by "data".
  // => no need to make a loop to receive all the data (see documentation)
  received = SDLNet_TCP_Recv(socket, data, len);

  return (received == int(len));
}

bool WSocket::ReceiveBuffer(void* data, size_t len)
{
  bool r;

  if (SDLNet_CheckSockets(socket_set, 5000) == 0) {
    return false;
  }

  Lock();
  r = ReceiveBuffer_NoLock(data, len);
  UnLock();

  return r;
}

bool WSocket::ReceiveInt_NoLock(int& nbr)
{
  char packet[4];
  Uint32 u_nbr;

  if (!SDLNet_SocketReady(socket)) {
    return false;
  }

  if (!ReceiveBuffer_NoLock(packet, sizeof(packet))) {
    return false;
  }

  u_nbr = SDLNet_Read32(packet);
  nbr = *((int*)&u_nbr);

  return true;
}

bool WSocket::ReceiveInt(int& nbr)
{
  bool r;

  if (SDLNet_CheckSockets(socket_set, 5000) == 0) {
    return false;
  }

  Lock();
  r = ReceiveInt_NoLock(nbr);
  UnLock();

  return r;
}

bool WSocket::ReceiveStr_NoLock(std::string &_str, size_t maxlen)
{
  bool r = true;
  unsigned int size = 0;
  char* str;

  r = ReceiveInt_NoLock((int&)size);
  if (!r) {
    goto out;
  }

  if (size == 0) {
    _str = "";
    goto out;
  }

  if (size > maxlen) {
    r = false;
    goto out;
  }

  str = new char[size+1];
  r = ReceiveBuffer_NoLock(str, size);
  if (!r) {
    goto out_delete;
  }

  str[size] = '\0';

  _str = str;

 out_delete:
  delete []str;
 out:
  return r;
}

bool WSocket::ReceiveStr(std::string &_str, size_t maxlen)
{
  bool r = false;

  if (SDLNet_CheckSockets(socket_set, 5000) == 0) {
    goto out;
  }

  Lock();
  r = ReceiveStr_NoLock(_str, maxlen);
  UnLock();

 out:
  return r;
}

std::string WSocket::GetAddress() const
{
  IPaddress* ip = SDLNet_TCP_GetPeerAddress(socket);
  std::string address;
  const char* resolved_ip = SDLNet_ResolveIP(ip);
  if (resolved_ip)
    address = resolved_ip;
  else
    return "Unresolved address";

  return address;
}
