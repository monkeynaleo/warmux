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
 
#include <SDL_net.h>
#include "top_server.h"
#include "network.h"
#include "gui/question.h"
#include "tool/debug.h"
#include "tool/i18n.h"


TopServer top_server;


TopServer::TopServer()
{
  hidden_server = false;
  connected = false;
}

TopServer::~TopServer()
{
  if(connected)
    Disconnect();
}

bool TopServer::Connect()
{
  assert(!connected);

  if( hidden_server )
    return true;

  network.Init();

  MSG_DEBUG("top_server", "Opening connection");

  if(SDLNet_ResolveHost(&ip, "localhost" , 1234)==-1)
  {
    Question question;
    question.Set(_("Invalid top server adress!"),1,0);
    question.Ask();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return false;
  }

  socket = SDLNet_TCP_Open(&ip);

  if(!socket)
  {
    Question question;
    question.Set(_("Unable to contact top server!"),1,0);
    question.Ask();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return false;
  }

  connected = true;
  return true;
}

void TopServer::Disconnect()
{
  if( hidden_server )
  {
    hidden_server = false;
    return;
  }

  if( !connected )
    return;

  SDLNet_TCP_Close(socket);
  connected = false;
}
