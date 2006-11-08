/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
 * Notify a top server of an opened wormux server
 * Obtain information about running games from a top server
 *****************************************************************************/
 
#ifndef TOP_SERVER_H
#define TOP_SERVER_H
//-----------------------------------------------------------------------------
#include <SDL_net.h>

class TopServer
{
  TCPsocket socket;
  IPaddress ip;

  // If the server is visible on internet
  bool hidden_server;

  bool connected;
public:
  TopServer();
  ~TopServer();

  bool Connect();
  void SetHiddenServer() { hidden_server = true; };
  void Disconnect();
};

extern TopServer top_server;

#endif
