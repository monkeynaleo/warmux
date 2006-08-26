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
 * Network layer for Wormux.
 *****************************************************************************/

#include "network.h"
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <SDL_thread.h>
#include "../game/game_mode.h"
#include "../gui/question.h"
#include "../include/action_handler.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
//-----------------------------------------------------------------------------
const uint packet_max_size = 100; // in bytes

//-----------------------------------------------------------------------------
Network network;

//-----------------------------------------------------------------------------

Network::Network()
{
  max_player_number = 0;
  m_is_connected = false;
  m_is_server = false;
  m_is_client = false;
  state = NETWORK_NOT_CONNECTED;
  inited = false;
  sync_lock = false;
}

//-----------------------------------------------------------------------------
int net_thread_func(void* no_param)
{
  network.ReceiveActions();
  return 1;
}

//-----------------------------------------------------------------------------

void Network::Init()
{
  if(inited) return;
  if (SDLNet_Init()) {
      Error(_("Failed to initialize network library!"));
  }
  inited = true;
  max_player_number = GameMode::GetInstance()->max_teams;
  connected_player = 0;
}

//-----------------------------------------------------------------------------

Network::~Network() 
{
  Disconnect();
  if(inited)
    SDLNet_Quit();
}

//-----------------------------------------------------------------------------

void Network::Disconnect() 
{
  if(!m_is_connected) return;

  m_is_connected = false; // To make the threads terminate

  SDL_WaitThread(thread,NULL);
  printf("Network thread finished\n");
  for(std::list<TCPsocket>::iterator client=conn.begin();
      client != conn.end();
      client++)
  {
    SDLNet_TCP_DelSocket(socket_set,*client);
    SDLNet_TCP_Close(*client);
  }
  conn.clear();
  SDLNet_FreeSocketSet(socket_set);

  if(m_is_server)
    SDLNet_TCP_Close(server_socket);

  m_is_server = false;
  m_is_client = false;
}

//-----------------------------------------------------------------------------
//----------------       Client specific methods   ----------------------------
//-----------------------------------------------------------------------------
void Network::ClientConnect(const std::string &host, const std::string& port) 
{
  MSG_DEBUG("network", "Client connect to %s:%s", host.c_str(), port.c_str());

  int prt=0;
  IPaddress ip;
  sscanf(port.c_str(),"%i",&prt);

  if(SDLNet_ResolveHost(&ip,host.c_str(),prt)==-1)
  {
    Question question;
    question.Set(_("Invalid server adress!"),1,0);
    question.AskQuestion();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  TCPsocket socket = SDLNet_TCP_Open(&ip);

  if(!socket)
  {
    Question question;
    question.Set(_("Unable to contact server!"),1,0);
    question.AskQuestion();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  m_is_client = true;
  m_is_server = false;
  state = NETWORK_OPTION_SCREEN;
  m_is_connected = true;

  socket_set = SDLNet_AllocSocketSet(1);
  SDLNet_TCP_AddSocket(socket_set, socket);
  connected_player = 1;
  conn.push_back(socket);
  thread = SDL_CreateThread(net_thread_func,NULL);
}

//-----------------------------------------------------------------------------
//----------------       Server specific methods   ----------------------------
//-----------------------------------------------------------------------------

void Network::ServerStart(const std::string &port) 
{
  // The server starts listening for clients
  MSG_DEBUG("network", "Start server on port %s", port.c_str());

  conn.clear();
  // Convert port number (std::string port) into SDL port number format:
  int prt;
  sscanf(port.c_str(),"%i",&prt);

  if(SDLNet_ResolveHost(&ip,NULL,prt)==-1)
  {
    Question question;
    question.Set(_("Invalid port!"),1,0);
    question.AskQuestion();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  m_is_server = true;
  m_is_client = false;
  m_is_connected = true;

  // Open the port to listen to
  AcceptIncoming();
  connected_player = 1;
  printf("\nConnected\n");
  state = NETWORK_OPTION_SCREEN;
  socket_set = SDLNet_AllocSocketSet(GameMode::GetInstance()->max_teams);
  thread = SDL_CreateThread(net_thread_func,NULL);
}

std::list<TCPsocket>::iterator Network::CloseConnection(std::list<TCPsocket>::iterator closed)
{
  printf("Client disconnected\n");
  SDLNet_TCP_DelSocket(socket_set,*closed);
  SDLNet_TCP_Close(*closed);
  if(m_is_server && connected_player == max_player_number)
  {
    // A new player will be able to connect, so we reopen the socket
    // For incoming connections
    AcceptIncoming();
  }

  connected_player--;
  return conn.erase(closed);
}

void Network::AcceptIncoming()
{
  assert(m_is_server);
  if(state == NETWORK_PLAYING) return;

  server_socket = SDLNet_TCP_Open(&ip);
  if(!server_socket)
  {
    Question question;
    question.Set(_("Unable to listen for client!"),1,0);
    question.AskQuestion();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }
  printf("\nStart listening");
}

void Network::RejectIncoming()
{
  assert(m_is_server);
  if(!server_socket) return;
  SDLNet_TCP_Close(server_socket);
  server_socket = NULL;
  printf("\nStop listening");
}

//-----------------------------------------------------------------------------
//----------------       Action handling methods   ----------------------------
//-----------------------------------------------------------------------------
void Network::ReceiveActions()
{
  char packet[packet_max_size];

  uint i;
  int received;
  Uint32 packet_size = 0;

  while(m_is_connected && (conn.size()==1 || m_is_server))
  {
    packet_size = 0;
    i = 0;
    while(SDLNet_CheckSockets(socket_set, 10) == 0 && m_is_connected) //Loop while nothing is received
    if(m_is_server && server_socket)
    {
      // Check for an incoming connection
      TCPsocket incoming;
      incoming = SDLNet_TCP_Accept(server_socket);
      if(incoming)
      {
        SDLNet_TCP_AddSocket(socket_set, incoming);
        conn.push_back(incoming);
        connected_player++;
        printf("New client connected\n");
        if(connected_player >= max_player_number)
          RejectIncoming();
        ActionHandler::GetInstance()->NewAction(new Action(ACTION_ASK_VERSION));
      }
    }

    std::list<TCPsocket>::iterator sock=conn.begin();
    while(sock != conn.end() && m_is_connected)
    {
      if(SDLNet_SocketReady(*sock)) // Check if this socket contains data to receive
      {
        // Read the size of the packet
        Uint32 net_size=0;
        if(SDLNet_TCP_Recv(*sock, &net_size, 4) <= 0)
        {
          sock = CloseConnection(sock);
          continue;
        }
        packet_size = SDLNet_Read32(&net_size);
        assert(packet_size > 0);


        // Fill the packet while it didn't reached its size
        memset(packet,0, packet_max_size);
        while(packet_size != i)
        {
          received = SDLNet_TCP_Recv(*sock, packet + i, packet_size - i);
          if(received > 0)
            i += received;
          if(received < 0)
            std::cerr << "Malformed packet" << std::endl;
        }

        Action* a = make_action(packet);
        MSG_DEBUG("network.traffic","Received action %s",
                ActionHandler::GetInstance()->GetActionName(a->GetType()).c_str());

        ActionHandler::GetInstance()->NewAction(a, false);

        // Repeat the packet to other clients:
        if(a->GetType() != ACTION_SEND_VERSION
        && a->GetType() != ACTION_CHANGE_STATE)
        for(std::list<TCPsocket>::iterator client = conn.begin();
            client != conn.end();
            client++)
        if(client != sock)
        {
          SDLNet_TCP_Send(*client,&packet_size,1);
          SDLNet_TCP_Send(*client,packet,packet_max_size);
        }
      }
      sock++;
    }
  }
}

// Send Messages
void Network::SendAction(Action* a)
{
  if (!m_is_connected) return;

  MSG_DEBUG("network.traffic","Send action %s",
        ActionHandler::GetInstance()->GetActionName(a->GetType()).c_str());

  Uint32 size;
  SDLNet_Write32(packet_max_size, &size);
  char packet[packet_max_size];
  memset(packet,0,packet_max_size);
  a->Write(packet);

  packet[packet_max_size - 1] = 0xFF;

  assert(*((int*)packet) != 0 );

  for(std::list<TCPsocket>::iterator client = conn.begin();
      client != conn.end();
      client++)
  {
    SDLNet_TCP_Send(*client,&size,4);
    SDLNet_TCP_Send(*client,packet,packet_max_size);
  }
}

//-----------------------------------------------------------------------------

const bool Network::IsConnected() const { return m_is_connected; }
const bool Network::IsLocal() const { return !m_is_server && !m_is_client; }
const bool Network::IsServer() const { return m_is_server; }
const bool Network::IsClient() const { return m_is_client; }

//-----------------------------------------------------------------------------

Action* Network::make_action(char* packet)
{ 
  Action_t type = (Action_t)SDLNet_Read32(packet);
  return new Action(type);
}

//-----------------------------------------------------------------------------
