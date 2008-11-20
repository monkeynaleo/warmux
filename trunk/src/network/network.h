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

#ifndef NETWORK_H
#define NETWORK_H
//-----------------------------------------------------------------------------
#include <list>
#include <string>
#include <WORMUX_network.h>
#include <WORMUX_singleton.h>
#include "include/base.h"
#include "network/player.h"
//-----------------------------------------------------------------------------

// Use this debug to store network communication to a file
//#define LOG_NETWORK
// Factorize some declarations for Visual C++
#ifdef _MSC_VER
#  define S_IRUSR _S_IREAD
#  define S_IWUSR _S_IWRITE
#endif
#ifdef WIN32
#  define S_IRGRP 0
#  define O_SYNC  O_BINARY
#endif

// Some forward declarations
struct SDL_Thread;
class Action;
class DistantComputer;
class NetworkServer;
class NetworkMenu;
class WSocketSet;


class NetworkThread
{
private:
  static SDL_Thread* thread; // network thread, where we receive data from network
  static bool stop_thread;

  static int ThreadRun(void* no_param);
public:
  static void Start();
  static void Stop();

  static bool Continue();
  static void Wait();
};


class Network : public Singleton<Network>
{
public:
  typedef enum
    {
      NO_NETWORK,
      NETWORK_MENU_OK,
      NETWORK_LOADING_DATA,
      NETWORK_READY_TO_PLAY,
      NETWORK_PLAYING,
      NETWORK_NEXT_GAME
    } network_state_t;

private:

  Network(const Network&);
  const Network& operator=(const Network&);
  friend class DistantComputer;

  std::string password;

  static int num_objects;

  bool turn_master_player;
  Player player;

protected:
  bool game_master_player;
  network_state_t state;

  Network(const std::string& password); // pattern singleton

  WSocketSet* socket_set;

#ifdef LOG_NETWORK
  int fout;
  int fin;
#endif

  virtual void HandleAction(Action* a, DistantComputer* sender) = 0;
  virtual void WaitActionSleep() = 0;
  virtual void SendAction(const Action& a, DistantComputer* client, bool clt_as_rcver) const;

  void DisconnectNetwork();

public:
  NetworkMenu* network_menu;

  std::list<DistantComputer*> cpu; // list of the connected computer
  bool sync_lock;

  virtual ~Network();

  static Network* GetInstance();
  static NetworkServer* GetInstanceServer(); // WARNING: return NULL if not server!!

  static void Disconnect();

  static bool IsConnected();
  virtual bool IsLocal() const { return false; }
  virtual bool IsServer() const { return false; }
  virtual bool IsClient() const { return false; }

  virtual bool IsGameMaster() const { return game_master_player; }

  const std::string& GetPassword() const { return password; }
  Player& GetPlayer();

  std::string GetDefaultNickname() const;

  // Action handling
  void SendActionToAll(const Action& action) const;
  void SendActionToOne(const Action& action, DistantComputer* client) const;
  void SendActionToAllExceptOne(const Action& action, DistantComputer* client) const;

  virtual std::list<DistantComputer*>::iterator CloseConnection(std::list<DistantComputer*>::iterator closed) = 0;

  // Start a client
  static connection_state_t ClientStart(const std::string &host, const std::string &port,
					const std::string& password);

  // Start a server
  static connection_state_t ServerStart(const std::string &port,
					const std::string& password);

  // Manage network state
  void SetState(Network::network_state_t state);
  Network::network_state_t GetState() const;
  void SendNetworkState();

  void SetTurnMaster(bool master);
  bool IsTurnMaster() const;

  void ReceiveActions();

  uint GetNbConnectedPlayers() const;
  uint GetNbInitializedPlayers() const;
  uint GetNbReadyPlayers() const;
  uint GetNbCheckedPlayers() const;
};

//-----------------------------------------------------------------------------
#endif
