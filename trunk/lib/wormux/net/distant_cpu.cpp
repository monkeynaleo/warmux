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
#include <WORMUX_socket.h>

static const int MAX_PACKET_SIZE = 250*1024;

DistantComputer::DistantComputer(WSocket* new_sock, const std::string& nickname) :
  sock(new_sock),
  state(DistantComputer::STATE_NOT_INITIALIZED)
{
  player.SetNickname(nickname);
}

DistantComputer::~DistantComputer()
{
  WORMUX_DisconnectHost(*this);

  player.Disconnect();

  delete sock;
}

Player& DistantComputer::GetPlayer()
{
  return player;
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

void DistantComputer::SetState(DistantComputer::state_t _state)
{
  state = _state;
}

DistantComputer::state_t DistantComputer::GetState() const
{
  return state;
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
  std::string str = GetAddress() + std::string(" (") + player.GetNickname() + std::string(" )");
  return str;
}
