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
#include <WORMUX_debug.h>

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

SDL_Thread* NetworkThread::thread = NULL;
bool NetworkThread::stop_thread = false;

int NetworkThread::ThreadRun(void* /*no_param*/)
{
  MSG_DEBUG("network", "Thread created: %u", SDL_ThreadID());
  Network::GetInstance()->ReceiveActions();
  return 0;
}

void NetworkThread::Start()
{
  thread = SDL_CreateThread(NetworkThread::ThreadRun, NULL);
  stop_thread = false;
}

void NetworkThread::Stop()
{
  stop_thread = true;
}

bool NetworkThread::Continue()
{
  return !stop_thread;
}

void NetworkThread::Wait()
{
  if (thread != NULL && SDL_ThreadID() != SDL_GetThreadID(thread)) {
    SDL_WaitThread(thread, NULL);
  }

  thread = NULL;
  stop_thread = false;
}

//-----------------------------------------------------------------------------

int  Network::num_objects = 0;

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
  game_master_player(false),
  state(NO_NETWORK),// useless value at beginning
  socket_set(NULL),
#ifdef LOG_NETWORK
  fout(0),
  fin(0),
#endif
  network_menu(NULL),
  cpu(),
  sync_lock(false)
{
  player.SetNickname(GetDefaultNickname());
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

//-----------------------------------------------------------------------------

void Network::ReceiveActions()
{
  char* buffer;
  size_t packet_size;
  std::list<DistantComputer*>::iterator dst_cpu;

  while (NetworkThread::Continue()) // While the connection is up
  {
    if (state == NETWORK_PLAYING && cpu.empty())
    {
      // If while playing everybody disconnected, just quit
      break;
    }

    //Loop while nothing is received
    while (NetworkThread::Continue())
    {
      WaitActionSleep();

      // Check forced disconnections
      for (dst_cpu = cpu.begin();
           NetworkThread::Continue() && dst_cpu != cpu.end();
           dst_cpu++)
      {
	// Disconnection is in 2 phases to be handled by one thread
        if ((*dst_cpu)->MustBeDisconnected()) {
          dst_cpu = CloseConnection(dst_cpu);
        }
      }

      // List is now maybe empty
      if (cpu.empty()) {
        if (IsClient()) {
          fprintf(stderr, "you are alone!\n");
	  NetworkThread::Stop();
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
         NetworkThread::Continue() && dst_cpu != cpu.end();
         dst_cpu++)
    {
      if((*dst_cpu)->SocketReady()) // Check if this socket contains data to receive
      {

    	  if (!(*dst_cpu)->ReceiveDatas(reinterpret_cast<void* &>(buffer), packet_size)) {

	  // An error occured during the reception
          dst_cpu = CloseConnection(dst_cpu);

          // Please Visual Studio that in debug mode has trouble with continuing
          if (cpu.empty()) {
            if (IsClient()) {
              fprintf(stderr, "you are alone!\n");
	      NetworkThread::Stop();
              return; // We really don't need to go through the loops
            }
            break;
          }
          continue;
        }

#ifdef LOG_NETWORK
        if (fin != 0) {
          int tmp = 0xFFFFFFFF;
          write(fin, &packet_size, 4);
          write(fin, buffer, packet_size);
          write(fin, &tmp, 4);
        }
#endif

        Action* a = new Action(buffer, (*dst_cpu));
        free(buffer);

	MSG_DEBUG("network.traffic", "Received action %s",
		  ActionHandler::GetInstance()->GetActionName(a->GetType()).c_str());
	HandleAction(a, *dst_cpu);

        if (cpu.empty()) {
          if (IsClient()) {
            fprintf(stderr, "you are alone!\n");
            NetworkThread::Stop();
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
    NetworkThread::Stop();
    singleton->DisconnectNetwork();
    delete singleton;
    ChatLogger::CloseIfOpen();
  }
}

// Protected method for client and server
void Network::DisconnectNetwork()
{
  NetworkThread::Wait();

  DistantComputer* tmp;
  std::list<DistantComputer*>::iterator client = cpu.begin();

  while (client != cpu.end()) {
    tmp = (*client);
    client = cpu.erase(client);
    delete tmp;
  }

  if (socket_set != NULL) {
    delete socket_set;
  }
}

//-----------------------------------------------------------------------------
//----------------       Action handling methods   ----------------------------
//-----------------------------------------------------------------------------

// Send Messages
void Network::SendActionToAll(const Action& a) const
{
  MSG_DEBUG("network.traffic","Send action %s to all remote computers",
            ActionHandler::GetInstance()->GetActionName(a.GetType()).c_str());

  SendAction(a, NULL, false);
}

void Network::SendActionToOne(const Action& a, DistantComputer* client) const
{
  MSG_DEBUG("network.traffic","Send action %s to %s (%s)",
            ActionHandler::GetInstance()->GetActionName(a.GetType()).c_str(),
	    client->ToString().c_str());

  SendAction(a, client, true);
}

void Network::SendActionToAllExceptOne(const Action& a, DistantComputer* client) const
{
  MSG_DEBUG("network.traffic","Send action %s to all EXCEPT %s",
            ActionHandler::GetInstance()->GetActionName(a.GetType()).c_str(),
	    client->ToString().c_str());

  SendAction(a, client, false);
}

// if (client == NULL) sending to every clients
// if (clt_as_rcver) sending only to client 'client'
// if (!clt_as_rcver) sending to all EXCEPT client 'client'
void Network::SendAction(const Action& a, DistantComputer* client, bool clt_as_rcver) const
{
  char* packet;
  int size;

  a.WriteToPacket(packet, size);
  ASSERT(packet != NULL);

#ifdef LOG_NETWORK
  if (fout != 0) {
    int tmp = 0xFFFFFFFF;
    write(fout, &size, 4);
    write(fout, packet, size);
    write(fout, &tmp, 4);
  }
#endif

  if (clt_as_rcver) {
    ASSERT(client);
    client->SendData(packet, size);
  } else {

    for (std::list<DistantComputer*>::const_iterator it = cpu.begin();
	 it != cpu.end(); it++) {

      if ((*it) != client) {
	(*it)->SendData(packet, size);
      }
    }
  }

  free(packet);
}

//-----------------------------------------------------------------------------

// Static method
bool Network::IsConnected()
{
  return (!GetInstance()->IsLocal() && NetworkThread::Continue());
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
  const connection_state_t error = net->ClientConnect(host, port);

  if (error != CONNECTED) {
    // revert change if connection failed
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
  const connection_state_t error = net->ServerStart(port, GameMode::GetInstance()->max_teams);

  if (error != CONNECTED) {
    // revert change
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

void Network::SendNetworkState()
{
  ASSERT(!IsLocal());

  if (IsGameMaster()) {
    Action a(Action::ACTION_NETWORK_MASTER_CHANGE_STATE);
    a.Push(state);
    SendActionToAll(a);
  } else {
    Action a(Action::ACTION_NETWORK_CLIENT_CHANGE_STATE);
    a.Push(state);
    SendActionToAll(a);
  }
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

Player& Network::GetPlayer()
{
  return player;
}

//-----------------------------------------------------------------------------

uint Network::GetNbConnectedPlayers() const
{
  ASSERT(IsGameMaster());

  return cpu.size() + 1;
}

uint Network::GetNbInitializedPlayers() const
{
  ASSERT(IsGameMaster());
  uint r = 0;

  for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
       client != cpu.end();
       client++) {
    if ((*client)->GetState() == DistantComputer::STATE_INITIALIZED)
      r++;
  }

  return r;
}

uint Network::GetNbReadyPlayers() const
{
  ASSERT(IsGameMaster());
  uint r = 0;

  for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
       client != cpu.end();
       client++) {
    if ((*client)->GetState() == DistantComputer::STATE_READY)
      r++;
  }

  return r;
}

uint Network::GetNbCheckedPlayers() const
{
  ASSERT(IsGameMaster());
  uint r = 0;

  for (std::list<DistantComputer*>::const_iterator client = cpu.begin();
       client != cpu.end();
       client++) {
    if ((*client)->GetState() == DistantComputer::STATE_CHECKED)
      r++;
  }

  return r;
}
