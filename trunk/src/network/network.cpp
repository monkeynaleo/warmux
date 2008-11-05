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
 * Network layer for Wormux.
 *****************************************************************************/

#include <SDL_thread.h>
#include <SDL_timer.h>

#include "network/network.h"
#include "network/network_local.h"
#include "network/network_client.h"
#include "network/network_server.h"
#include "network/distant_cpu.h"
#include "network/chatlogger.h"
//-----------------------------------------------------------------------------
#include "game/game_mode.h"
#include "game/game.h"
#include "graphic/video.h"
#include "include/action.h"
#include "include/action_handler.h"
#include "include/app.h"
#include "include/constant.h"
#include "tool/debug.h"

#include <sys/types.h>
#ifdef LOG_NETWORK
#  include <sys/stat.h>
#  include <fcntl.h>
#  ifdef WIN32
#    include <io.h>
#  endif
#endif
//-----------------------------------------------------------------------------

// Standard header, only needed for the following method
#ifdef WIN32
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <arpa/nameser.h>
#  include <resolv.h>
#  include <errno.h>
#  include <unistd.h>
#endif

#include "team/team.h"
#include "team/teams_list.h"

//-----------------------------------------------------------------------------

int  Network::num_objects = 0;
bool Network::stop_thread = true;

Network * Network::GetInstance()
{
  if (singleton == NULL) {
    singleton = new NetworkLocal();
    MSG_DEBUG("singleton", "Created singleton %p of type 'NetworkLocal'\n", singleton);
  }
  return singleton;
}

NetworkServer * Network::GetInstanceServer()
{
  if (singleton == NULL || !singleton->IsServer()) {
    return NULL;
  }
  return (NetworkServer*)singleton;
}

Network::Network(const std::string& passwd):
  password(passwd),
  turn_master_player(false),
  state(NO_NETWORK),// useless value at beginning
  thread(NULL),
  socket_set(NULL),
#ifdef LOG_NETWORK
  fout(0),
  fin(0),
#endif
  network_menu(NULL),
  cpu(),
  sync_lock(false)
{
  nickname = GetDefaultNickname();
  num_objects++;
}
//-----------------------------------------------------------------------------

Network::~Network()
{
#ifdef LOG_NETWORK
  if (fin != 0)
    close(fin);
  if (fout != 0)
    close(fout);
#endif

  num_objects--;
  if (num_objects == 0) {
    WNet::Quit();
  }
}

//-----------------------------------------------------------------------------

std::string Network::GetDefaultNickname() const
{
  std::string s_nick;
  const char *nick = NULL;
#ifdef WIN32
  char  buffer[32];
  DWORD size = 32;
  if (GetUserName(buffer, &size))
    nick = buffer;
#else
  nick = getenv("USER");
#endif
  s_nick = (nick) ? nick : _("Unnamed");
  return s_nick;
}

void Network::SetNickname(const std::string& _nickname)
{
  nickname = _nickname;
}

const std::string& Network::GetNickname() const
{
  return nickname;
}

//-----------------------------------------------------------------------------

bool Network::ThreadToContinue() const
{
  return !stop_thread;
}

int Network::ThreadRun(void*/*no_param*/)
{
  MSG_DEBUG("network", "Thread created: %u", SDL_ThreadID());
  GetInstance()->ReceiveActions();
  return 1;
}

void Network::ReceiveActions()
{
  char* packet;
  std::list<DistantComputer*>::iterator dst_cpu;

  while (ThreadToContinue()) // While the connection is up
  {
    if (state == NETWORK_PLAYING && cpu.empty())
    {
      // If while playing everybody disconnected, just quit
      break;
    }

    //Loop while nothing is received
    while (ThreadToContinue())
    {
      WaitActionSleep();

      // Check forced disconnections
      for (dst_cpu = cpu.begin();
           ThreadToContinue() && dst_cpu != cpu.end();
           dst_cpu++)
      {
        if((*dst_cpu)->force_disconnect)
        {
          dst_cpu = CloseConnection(dst_cpu);
          if (cpu.empty())
            break; // Let it be handled afterwards
        }
      }

      // List is now maybe empty
      if (cpu.empty()) {
        if (IsClient()) {
          fprintf(stderr, "you are alone!\n");
	  stop_thread = true;
          return; // We really don't need to go through the loops
        }
        // Even for server, as Visual Studio in debug mode has trouble with that loop
	continue;
      }
      int num_ready = socket_set->CheckActivity(100);
      // Means something is available
      if (num_ready>0)
        break;
      // Means an error
      else if (num_ready == -1)
      {
        fprintf(stderr, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        continue; //Or break?
      }
    }

    for (dst_cpu = cpu.begin();
         ThreadToContinue() && dst_cpu != cpu.end();
         dst_cpu++)
    {
      if((*dst_cpu)->SocketReady()) // Check if this socket contains data to receive
      {
        // Read the size of the packet
        int packet_size = (*dst_cpu)->ReceiveDatas(packet);
        if( packet_size == -1) { // An error occured during the reception
          dst_cpu = CloseConnection(dst_cpu);
          // Please Visual Studio that in debug mode has trouble with continuing
          if (cpu.empty()) {
            if (IsClient()) {
              fprintf(stderr, "you are alone!\n");
	      stop_thread = true;
              return; // We really don't need to go through the loops
            }
            break;
          }
          continue;
        } else
        if (packet_size == 0) // We didn't receive the full packet yet
          continue;

#ifdef LOG_NETWORK
        if (fin != 0) {
          int tmp = 0xFFFFFFFF;
          write(fin, &packet_size, 4);
          write(fin, packet, packet_size);
          write(fin, &tmp, 4);
        }
#endif

        Action* a = new Action(packet, (*dst_cpu));
        if(!a->CheckCRC()) {
          MSG_DEBUG("network.crc_bad","!!! Bad CRC for action received !!!");
          delete a;
        } else {
          MSG_DEBUG("network.traffic", "Received action %s",
                    ActionHandler::GetInstance()->GetActionName(a->GetType()).c_str());
          HandleAction(a, *dst_cpu);
        }
        free(packet);

        if (cpu.empty()) {
          if (IsClient()) {
            fprintf(stderr, "you are alone!\n");
            stop_thread = true;
            return; // We really don't need to go through the loops
          }
          break;
        }
      }
    }
  }
}
//-----------------------------------------------------------------------------

// Static method
void Network::Disconnect()
{
  // restore Windows title
  AppWormux::GetInstance()->video->SetWindowCaption( std::string("Wormux ") + Constants::WORMUX_VERSION);

  if (singleton != NULL) {
    singleton->stop_thread = true;
    singleton->DisconnectNetwork();
    delete singleton;
    ChatLogger::CloseIfOpen();
  }
}

// Protected method for client and server
void Network::DisconnectNetwork()
{
  if (thread != NULL && SDL_ThreadID() != SDL_GetThreadID(thread)) {
    SDL_WaitThread(thread, NULL);
  }

  thread = NULL;
  stop_thread = true;

  for(std::list<DistantComputer*>::iterator client = cpu.begin();
      client != cpu.end();
      client++)
  {
    delete *client;
  }
  cpu.clear();

  if (socket_set != NULL) {
    delete socket_set;
  }
}

//-----------------------------------------------------------------------------
//----------------       Action handling methods   ----------------------------
//-----------------------------------------------------------------------------

// Send Messages
void Network::SendAction(const Action& a) const
{
  MSG_DEBUG("network.traffic","Send action %s",
            ActionHandler::GetInstance()->GetActionName(a.GetType()).c_str());

  int size;
  char* packet;
  a.WriteToPacket(packet, size);

  ASSERT(packet != NULL);
  SendPacket(packet, size);

  free(packet);
}

void Network::SendPacket(char* packet, int size) const
{
#ifdef LOG_NETWORK
  if (fout != 0) {
    int tmp = 0xFFFFFFFF;
    write(fout, &size, 4);
    write(fout, packet, size);
    write(fout, &tmp, 4);
  }
#endif

  for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
       client != cpu.end();
       client++)
  {
    (*client)->SendDatas(packet, size);
  }
}

//-----------------------------------------------------------------------------

// Static method
bool Network::IsConnected()
{
  return (!GetInstance()->IsLocal() && !stop_thread);
}

//-----------------------------------------------------------------------------

// Static method
connection_state_t Network::ClientStart(const std::string& host,
                                        const std::string& port,
					const std::string& password)
{
  NetworkClient* net = new NetworkClient(password);
  MSG_DEBUG("singleton", "Created singleton %p of type 'NetworkClient'\n", net);

  // replace current singleton
  Network* prev = singleton;
  singleton = net;

  // try to connect
  stop_thread = false;
  const connection_state_t error = net->ClientConnect(host, port);

  if (error != CONNECTED) {
    // revert change if connection failed
    stop_thread = true;
    singleton = prev;
    delete net;
  } else if (prev != NULL) {
    delete prev;
  }
  AppWormux::GetInstance()->video->SetWindowCaption( std::string("Wormux ") + Constants::WORMUX_VERSION + " - Client mode");
  return error;
}

//-----------------------------------------------------------------------------

// Static method
connection_state_t Network::ServerStart(const std::string& port, const std::string& password)
{
  NetworkServer* net = new NetworkServer(password);
  MSG_DEBUG("singleton", "Created singleton %p of type 'NetworkServer'\n", net);

  // replace current singleton
  Network* prev = singleton;
  singleton = net;

  // try to connect
  stop_thread = false;
  const connection_state_t error = net->ServerStart(port, GameMode::GetInstance()->max_teams);

  if (error != CONNECTED) {
    // revert change
    stop_thread = true;
    singleton = prev;
    delete net;
  } else if (prev != NULL) {

    // that's ok
    AppWormux::GetInstance()->video->SetWindowCaption( std::string("Wormux ") + Constants::WORMUX_VERSION + " - Server mode");
    delete prev;
  }
  return error;
}

//-----------------------------------------------------------------------------

void Network::SetState(Network::network_state_t _state)
{
  state = _state;
}

Network::network_state_t Network::GetState() const
{
  return state;
}

void Network::SendNetworkState() const
{
  Action a(Action::ACTION_NETWORK_CHANGE_STATE);
  a.Push(state);
  SendAction(a);
}

void Network::SetTurnMaster(bool master)
{
  MSG_DEBUG("network.turn_master", "turn_master: %d", master);
  turn_master_player = master;
}

bool Network::IsTurnMaster() const
{
  return turn_master_player;
}

