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
 * Network connection menu: this menu allows the user to choose between
 * hosting a game or connecting to a server.
 *****************************************************************************/


#include "menu/internet_menu.h"

#include "include/app.h"
#include "gui/button.h"
#include "gui/label.h"
#include "gui/box.h"
#include "gui/button_text.h"
#include "gui/list_box.h"
#include "gui/question.h"
#include "graphic/video.h"
#include "network/net_error_msg.h"
#include "network/index_server.h"
#include "network/irc.h"
#include "tool/i18n.h"
#include "tool/resource_manager.h"

InternetMenu::InternetMenu() :
  Menu("menu/bg_network", vCancel)
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml",false);
  Rectanglei rectZero(0, 0, 0, 0);

  Rectanglei stdRect(0, 0, 405, 64);

  connection_box = new VBox(stdRect.GetSizeX(), false);
  connection_box->SetBorder(Point2i(0,0));

  connect_lst = new ListBox( Point2i(stdRect.GetSizeX(), 300), false);
  connection_box->AddWidget(connect_lst);

  refresh = new ButtonText( res, "main_menu/button",
                            _("Refresh"), // Refresh the list of available hosts
                            Font::FONT_BIG,
                            Font::FONT_NORMAL);

  refresh->SetSizePosition( stdRect );
  connection_box->AddWidget(refresh);

  connect = new ButtonText( res, "main_menu/button",
                            _("Connect !"),
                            Font::FONT_BIG,
                            Font::FONT_NORMAL);

  connect->SetSizePosition( stdRect );
  connection_box->AddWidget(connect);

  connection_box->SetXY(AppWormux::GetInstance()->video->window.GetWidth()/2 - stdRect.GetSizeX()/2,
			AppWormux::GetInstance()->video->window.GetHeight()/2 - 200);
  widgets.AddWidget(connection_box);

  resource_manager.UnLoadXMLProfile(res);
  RefreshList(false);

  //if there is a running server, preselect it
  if (connect_lst->Size() > 0)
    connect_lst->Select(0);

  irc.Connect();
}

InternetMenu::~InternetMenu()
{
}

void InternetMenu::OnClickUp(const Point2i &mousePosition, int button)
{
  Widget* w = widgets.ClickUp(mousePosition, button);

  if (w == refresh)
    RefreshList(true);
  else
  if (w == connect && connect_lst->GetSelectedItem() != -1)
  {
    char c_address[32];
    char c_port[16];
    sscanf(connect_lst->ReadValue().c_str(),"%[^:]:%s", c_address, c_port);

    std::string address = std::string(c_address);
    std::string port = std::string(c_port);

    connection_state_t conn = Network::ClientStart(address, port);
    if ( Network::IsConnected() && conn == CONNECTED )
    {
      close_menu = true;
      Menu::mouse_ok();
    }
    else
    {
      Menu::DisplayError(NetworkErrorToString(conn));
    }
  }
}

void InternetMenu::OnClick(const Point2i &mousePosition, int button)
{
  widgets.Click(mousePosition, button);
}

void InternetMenu::RefreshList(bool warning_if_empty)
{
  // Save the currently selected address
  int current = connect_lst->GetSelectedItem();

  // Empty the list:
  while (connect_lst->Size() != 0)
  {
    connect_lst->Select(0);
    connect_lst->RemoveSelected();
  }

  std::list<GameServerInfo> lst = IndexServer::GetInstance()->GetHostList();

  if (warning_if_empty && lst.size() == 0) {
    Menu::DisplayError(_("Sorry, currently, no game is waiting for players"));
    return;
  }

  for (std::list<GameServerInfo>::iterator game_server_info_it = lst.begin();
       game_server_info_it != lst.end();
       ++game_server_info_it) {
    std::string display_str = game_server_info_it->ip_address + ":";
    display_str += game_server_info_it->port + " - ";
    display_str += game_server_info_it->dns_address + " - ";
    display_str += game_server_info_it->game_name;

    std::string connect_str = game_server_info_it->ip_address + ":";
    connect_str += game_server_info_it->port;
    connect_lst->AddItem(false, display_str, connect_str);
  }

  if (current != -1 && connect_lst->Size() != 0)
    connect_lst->Select( current );
}

void InternetMenu::Draw(const Point2i &/*mousePosition*/)
{
  IndexServer::GetInstance()->Refresh();
}

bool InternetMenu::signal_ok()
{
  return true;
}

bool InternetMenu::signal_cancel()
{
  return true;
}
