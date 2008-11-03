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

#ifndef WORMUX_SOCKET_H
#define WORMUX_SOCKET_H
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <string>
#include "WORMUX_types.h"
//-----------------------------------------------------------------------------

class WSocket
{
private:
  TCPsocket socket;
  SDLNet_SocketSet socket_set;
  SDL_mutex* lock;

public:
  WSocket(TCPsocket _socket, SDLNet_SocketSet _socket_set);
  ~WSocket();

  void Lock();
  void UnLock();

  std::string GetAddress() const;
  bool IsReady() const;

  bool SendInt_NoLock(const int& nbr);
  bool SendInt(const int& nbr);

  bool SendStr_NoLock(const std::string &str);
  bool SendStr(const std::string &str);

  bool SendBuffer_NoLock(void* data, size_t len);
  bool SendBuffer(void* data, size_t len);

  bool ReceiveBuffer_NoLock(void* data, size_t len);
  bool ReceiveBuffer(void* data, size_t len);

  bool ReceiveInt_NoLock(int& nbr);
  bool ReceiveInt(int& nbr);

  bool ReceiveStr_NoLock(std::string &_str, size_t maxlen);
  bool ReceiveStr(std::string &_str, size_t maxlen);
};

//-----------------------------------------------------------------------------
#endif
