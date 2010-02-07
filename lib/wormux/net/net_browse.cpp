/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 * Wormux Network data structures and functions to browse list of
 * network games
 *****************************************************************************/

//-----------------------------------------------------------------------------
#include <SDL_thread.h>
#include <SDL_timer.h>
#include <WORMUX_error.h>
#include <WORMUX_net_browse.h>
#include <WORMUX_network.h>
//-----------------------------------------------------------------------------

SDL_Thread * Broadcaster::thread = NULL;
bool Broadcaster::running = false;

void broadcast_gameinfo(void*)
{
  Broadcaster::running = true;

  UDPSocket udpsock;
  udpsock = SDLNet_UDP_Open(WORMUX_NETWORK_PORT_INT);
  if(!udpsock) {
    fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    Broadcaster::running = false;
    return;
  }

  UDPpacket *packet;
  packet = SDLNet_AllocPacket(1024);


  do {



    SDL_Delay(200);
  } while (Broadcaster::running);

  SDLNet_UDP_Close(udpsock);
}

Broadcaster::Start(const GameServerInfo& _info)
{
  info = _info;
  thread = SDL_CreateThread(broadcast_gameinfo, NULL);
}

Broadcaster::Stop()
{
  running = false;

  if (thread) {
    int status;
    SDL_WaitThread(thread, &status);
    thread = NULL;
  }
}
