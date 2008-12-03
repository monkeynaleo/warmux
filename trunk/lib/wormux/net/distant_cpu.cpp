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
 * Distant Computer handling
 *****************************************************************************/

#include <algorithm>  //std::find
#include <SDL_thread.h>
#include <WORMUX_distant_cpu.h>
#include <WORMUX_error.h>
#include <WORMUX_i18n.h>
#include <WORMUX_socket.h>

static const int MAX_PACKET_SIZE = 250*1024;

DistantComputer::DistantComputer(WSocket* new_sock, const std::string& nickname, uint _game_id, uint initial_player_id) :
  sock(new_sock),
  state(DistantComputer::STATE_NOT_INITIALIZED),
  game_id(_game_id)
{
  Player theplayer(initial_player_id, nickname);
  players.push_back(theplayer);

  WORMUX_ConnectHost(*this);
}

DistantComputer::DistantComputer(WSocket* new_sock, const std::string& nickname, uint initial_player_id) :
  sock(new_sock),
  state(DistantComputer::STATE_NOT_INITIALIZED),
  game_id(0)
{
  Player theplayer(initial_player_id, nickname);
  players.push_back(theplayer);

  WORMUX_ConnectHost(*this);
}

DistantComputer::~DistantComputer()
{
  WORMUX_DisconnectHost(*this);

  // This will call the needed player->Disconnect() for each player
  players.clear();

  delete sock;
}

void DistantComputer::AddPlayer(uint player_id)
{
  Player theplayer(player_id, _("Unnamed"));
  players.push_back(theplayer);
}

void DistantComputer::DelPlayer(uint player_id)
{
  std::list<Player>::iterator player_it;
  for (player_it = players.begin(); player_it != players.end(); player_it++) {
    if (player_it->GetId() == player_id) {
      players.erase(player_it);
      return;
    }
  }
}

Player* DistantComputer::GetPlayer(uint player_id)
{
  Player* player = NULL;

  std::list<Player>::iterator player_it;
  for (player_it = players.begin(); player_it != players.end(); player_it++) {
    if (player_it->GetId() == player_id)
      return &(*player_it);
  }

  return player;
}

const std::list<Player>& DistantComputer::GetPlayers() const
{
  return players;
}

bool DistantComputer::SocketReady() const
{
  return sock->IsReady();
}

bool DistantComputer::ReceiveData(void* & data, size_t & len)
{
  return sock->ReceivePacket(data, len);
}

bool DistantComputer::SendData(const void* data, size_t len)
{
  return sock->SendPacket(data, len);
}

std::string DistantComputer::GetAddress() const
{
  return sock->GetAddress();
}

std::string DistantComputer::GetNicknames() const
{
  std::string nicknames;
  std::list<Player>::const_iterator player;

  for (player = players.begin(); player != players.end(); player++) {
    if (nicknames != "")
      nicknames += ", ";

    nicknames += player->GetNickname();
  }

  if (nicknames == "")
    nicknames = _("Unnamed");

  return nicknames;
}

void DistantComputer::SetState(DistantComputer::state_t _state)
{
  state = _state;
}

DistantComputer::state_t DistantComputer::GetState() const
{
  return state;
}

uint DistantComputer::GetGameId() const
{
  return game_id;
}

void DistantComputer::ForceDisconnection()
{
  state = STATE_ERROR;
}

bool DistantComputer::MustBeDisconnected()
{
  return (state == STATE_ERROR);
}

const std::string DistantComputer::ToString() const
{
  std::string str = GetAddress() + std::string(" (") + GetNicknames() + std::string(" )");
  return str;
}
