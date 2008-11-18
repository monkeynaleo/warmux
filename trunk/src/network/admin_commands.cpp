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
#include "include/app.h"
#include "network/admin_commands.h"
#include "network/distant_cpu.h"
#include "network/network.h"

static void PrintHelp()
{
  std::string msg = "help: " + std::string(_("Displays this message"));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg);
  msg = "kick <nickname>: " + std::string(_("Kicks the players designated by <nickname> out of the game"));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg);
  msg = "list: " + std::string(_("List the connected players"));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg);
}

static void Kick(const std::string& nick)
{
  std::string msg;
  for (std::list<DistantComputer*>::iterator cpu = Network::GetInstance()->cpu.begin();
       cpu != Network::GetInstance()->cpu.end();
       ++cpu) {

    if ((*cpu)->GetPlayer().GetNickname() == nick) {
      (*cpu)->ForceDisconnection();
      msg = std::string(Format("%s kicked", nick.c_str()));
      AppWormux::GetInstance()->ReceiveMsgCallback(msg);
      return;
    }
  }

  msg = std::string(Format("%s: no such nickame", nick.c_str()));
  AppWormux::GetInstance()->ReceiveMsgCallback(msg);
}

static void ListPlayers()
{
  if (Network::GetInstance()->cpu.empty()) {
    AppWormux::GetInstance()->ReceiveMsgCallback(_("No player connected"));
    return;
  }

  AppWormux::GetInstance()->ReceiveMsgCallback(_("Connected players: "));

  for (std::list<DistantComputer*>::iterator cpu = Network::GetInstance()->cpu.begin();
      cpu != Network::GetInstance()->cpu.end();
      ++cpu) {
    AppWormux::GetInstance()->ReceiveMsgCallback((*cpu)->GetPlayer().GetNickname());
  }

}

void ProcessCommand(const std::string & cmd)
{
  if (cmd == "/help") {
    PrintHelp();
  } else if (cmd.substr(0, 6) == "/kick ") {
    std::string nick = cmd.substr(6, cmd.size() - 6);
    Kick(nick);
  } else if (cmd.substr(0, 5) == "/list") {
    ListPlayers();
  } else {
    AppWormux::GetInstance()->ReceiveMsgCallback(_("Unknown command"));
    PrintHelp();
  }
}

