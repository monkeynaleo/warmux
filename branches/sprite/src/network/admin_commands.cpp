/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2007 Jon de Andres
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
 * Administration commands
 *****************************************************************************/

#include <string>
#include <WORMUX_distant_cpu.h>
#include "graphic/colors.h"
#include "include/app.h"
#include "network/admin_commands.h"
#include "network/network.h"

static void PrintHelp()
{
  std::string msg = "help: " + std::string(_("Displays this message"));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg, light_gray_color);
  msg = "kick <nickname>: " + std::string(_("Kicks the player designated by <nickname> out of the game"));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg, light_gray_color);
  msg = "list: " + std::string(_("Lists the connected players"));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg, light_gray_color);
  msg = "address: " + std::string(_("Shows the designated player address"));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg, light_gray_color);
}

typedef enum
{
  USER_KICK,
  USER_ADDRESS
} UserCommandType;

static void UserCommand(const std::string& nick, UserCommandType type)
{
  bool found = false;
  std::string msg;
  std::list<DistantComputer*>& hosts = Network::GetInstance()->LockRemoteHosts();

  for (std::list<DistantComputer*>::iterator cpu = hosts.begin();
       cpu != hosts.end();
       ++cpu) {

    if ((*cpu)->GetNicknames() == nick) {
      found = true;

      switch (type) {
      case USER_KICK:
        (*cpu)->ForceDisconnection();
        msg = std::string(Format(_("%s kicked from game"), nick.c_str()));
        break;
      case USER_ADDRESS:
        msg = std::string(Format(_("%s has address %s"), nick.c_str(), (*cpu)->GetAddress().c_str()));
        break;
      default:
        msg = std::string(_("Unknown command of type %i"), type);
        break;
      }

      break;
    }
  }

  if (!found) {
    msg = std::string(Format(_("%s: no such nickame"), nick.c_str()));
  }

  Network::GetInstance()->UnlockRemoteHosts();

  AppWormux::GetInstance()->ReceiveMsgCallback(msg, primary_red_color);
}

static void ListPlayers()
{
  if (Network::GetInstance()->GetNbPlayersConnected() == 0) {
    AppWormux::GetInstance()->ReceiveMsgCallback(_("No player connected"), primary_red_color);
    return;
  }

  AppWormux::GetInstance()->ReceiveMsgCallback(_("Connected players: "), primary_red_color);

  std::list<DistantComputer*>& hosts = Network::GetInstance()->LockRemoteHosts();

  for (std::list<DistantComputer*>::iterator cpu = hosts.begin();
      cpu != hosts.end();
      ++cpu) {
    std::string msg = std::string(Format("%s (%s)", (*cpu)->GetNicknames().c_str(), (*cpu)->GetAddress().c_str()));
    AppWormux::GetInstance()->ReceiveMsgCallback(msg, primary_red_color);
  }

  Network::GetInstance()->UnlockRemoteHosts();
}

void ProcessCommand(const std::string & cmd)
{
  if (cmd == "/help") {
    PrintHelp();
  } else if (cmd.substr(0, 6) == "/kick ") {
    std::string nick = cmd.substr(6, cmd.size() - 6);
    UserCommand(nick, USER_KICK);
  } else if (cmd.substr(0, 9) == "/address ") {
    std::string nick = cmd.substr(9, cmd.size() - 9);
    UserCommand(nick, USER_ADDRESS);
  } else if (cmd.substr(0, 5) == "/list") {
    ListPlayers();
  } else {
    AppWormux::GetInstance()->ReceiveMsgCallback(_("Unknown command"), primary_red_color);
    PrintHelp();
  }
}

