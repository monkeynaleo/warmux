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
 * Network connection menu: this menu allows the user to choose between
 * hosting a game or connecting to a server.
 *****************************************************************************/


#include "network_connection_menu.h"
#include "network_menu.h"
#include "internet_menu.h"

#include "../game/game.h"
#include "../game/config.h"
#include "../game/game_mode.h"
#include "../graphic/video.h"
#include "../graphic/font.h"
#include "../map/maps_list.h"
#include "../network/network.h"
#include "../network/top_server.h"
#include "../include/app.h"
#include "../include/action_handler.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../tool/string_tools.h"

NetworkConnectionMenu::NetworkConnectionMenu() :
  Menu("menu/bg_network", vCancel)
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml",false);
  Rectanglei rectZero(0, 0, 0, 0);
  
  normal_font = Font::GetInstance(Font::FONT_NORMAL);
  big_font = Font::GetInstance(Font::FONT_BIG);

  Rectanglei stdRect(0, 0, 300, 64);

  uint x_button = AppWormux::GetInstance()->video.window.GetWidth()/2 - stdRect.GetSizeX()/2;
  uint y_box = AppWormux::GetInstance()->video.window.GetHeight()/2 - 200;

  // Connection related widgets
  connection_box = new VBox(Rectanglei( x_button, y_box, stdRect.GetSizeX(), 1), false);
  connection_box->SetBorder(Point2i(0,0));
  connection_box->AddWidget(new Label(_("Server adress:"), rectZero, *normal_font));

  server_adress = new TextBox("localhost", rectZero, *normal_font);
  connection_box->AddWidget(server_adress);

  start_client = new ButtonText( Point2i(0,0),
				 res, "main_menu/button",
				 _("Connect to game"),
				 big_font);
  start_client->SetSizePosition( stdRect );
  connection_box->AddWidget(start_client);

  start_server = new ButtonText( Point2i(0,0),
				 res, "main_menu/button",
				 _("Host a game"),
				 big_font);
  start_server->SetSizePosition( stdRect );
  connection_box->AddWidget(start_server);

  internet_server = new PictureTextCBox(_("Server available on Internet"),
				"menu/fullscreen",
				stdRect);
  internet_server->SetValue( true );
  connection_box->AddWidget(internet_server);

  internet_client = new ButtonText( Point2i(0,0),
				 res, "main_menu/button",
				 _("Connect to an internet game"),
				 big_font);
  internet_client->SetSizePosition( stdRect );
  connection_box->AddWidget(internet_client);

  widgets.AddWidget(connection_box);

  // Warning about experimental networking
  msg_box = new MessageBox(7, 
			   Rectanglei( AppWormux::GetInstance()->video.window.GetWidth()/2 - 300,
				       y_box+connection_box->GetSizeY() + 30, 
				       600, 1), 
			   Font::GetInstance(Font::FONT_SMALL));
  widgets.AddWidget(msg_box);

  msg_box->NewMessage("WARNING!! Network is still under developement and therefore a little experimental.");
  msg_box->NewMessage(""); // Skip a line
  msg_box->NewMessage("Some weapons are disabled, because of known bugs");
  msg_box->NewMessage("(ninjarope, airhammer, blowtorch, submachine gun)");
  msg_box->NewMessage("and surely many other things don't work either!");
  msg_box->NewMessage(""); // Skip a line
  msg_box->NewMessage("Have a good game!");

  resource_manager.UnLoadXMLProfile(res);
}

NetworkConnectionMenu::~NetworkConnectionMenu()
{
}

void NetworkConnectionMenu::OnClic(const Point2i &mousePosition, int button)
{
  Widget* w = widgets.Clic(mousePosition, button);
  
  if (w == start_client)
  {
    network.Init();
    network.ClientConnect(server_adress->GetText(),WORMUX_NETWORK_PORT);
    if(network.IsConnected())
    {
      msg_box->NewMessage(_("Connected to ") + server_adress->GetText());
      msg_box->NewMessage("Click the green check when you are ready to");
      msg_box->NewMessage("play!");
    }
    else
      msg_box->NewMessage(_("Unable to connect"));
  }

  if (w == start_server)
  {
    if( !internet_server->GetValue() )
      top_server.SetHiddenServer();

    if( !top_server.Connect() )
      return;

    network.Init();
    network.ServerStart(WORMUX_NETWORK_PORT);

    top_server.SendServerStatus();

    if(network.IsConnected())
    {
      network.client_inited = 1;
      msg_box->NewMessage(_("Server started"));
    }
    else
      msg_box->NewMessage(_("Unable to start server"));
  }

  if (w == internet_client)
  {
    if( !top_server.Connect() )
      return;

    InternetMenu im;
    im.Run();

    top_server.Disconnect();
  }

  if (network.IsConnected()) {
    // run the network menu ! :-)
    NetworkMenu nm;
    network.network_menu = &nm;
    nm.Run();
    network.network_menu = NULL;
    top_server.Disconnect();

    // for the moment, it's just for test...
    close_menu = true;
    sig_ok();
  }
}

void NetworkConnectionMenu::Draw(const Point2i &mousePosition){}

void NetworkConnectionMenu::__sig_ok()
{
  network.Disconnect();
}

void NetworkConnectionMenu::__sig_cancel()
{
  network.Disconnect();
}

