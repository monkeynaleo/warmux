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
 * Game menu
 *****************************************************************************/

#include "network_menu.h"

#include "../game/game.h"
#include "../game/config.h"
#include "../game/game_mode.h"
#include "../graphic/video.h"
#include "../graphic/font.h"
#include "../map/maps_list.h"
#include "../network/network.h"
#include "../include/app.h"
#include "../include/action_handler.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../tool/string_tools.h"

const uint TEAMS_Y = 20;
const uint TEAMS_W = 160;
const uint TEAMS_H = 160;
const uint TEAM_LOGO_Y = 290;
const uint TEAM_LOGO_H = 48;

const uint MAPS_X = 20;
const uint MAPS_W = 160;
 
const uint MAP_PREVIEW_W = 300;

#define WORMUX_NETWORK_PORT "9999"

NetworkMenu::NetworkMenu() :
  Menu("menu/bg_play")
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml",false);
  Rectanglei rectZero(0, 0, 0, 0);
  
  normal_font = Font::GetInstance(Font::FONT_NORMAL);

  // Connection related widgets
  connection_box = new VBox(Rectanglei( 475 + 30 + 5, TEAMS_Y, 800-475-40, 1));
  connection_box->AddWidget(new Label(_("Server adress:"), rectZero, *normal_font));

  server_adress = new TextBox("127.0.0.1", rectZero, *normal_font);
  connection_box->AddWidget(server_adress);

  start_client = new ButtonText( Point2i(400,30),
                        res, "main_menu/button",
                        _("Connect to game"),
                        normal_font);
  start_client->SetSizePosition(Rectanglei(600,30, 150, 30));
  connection_box->AddWidget(start_client);

  start_server = new ButtonText( Point2i(400,30),
                        res, "main_menu/button",
                        _("Host a game"),
                        normal_font);
  start_server->SetSizePosition(Rectanglei(600,90, 150, 30));
  connection_box->AddWidget(start_server);

  widgets.AddWidget(connection_box);

  // Game options widgets
  player_number = new SpinButton(_("Max number of players:"), rectZero, GameMode::GetInstance()->max_teams, 1, 2, GameMode::GetInstance()->max_teams);
  options_box = new VBox(Rectanglei( 475 + 30 + 5, connection_box->GetSizeY() + 2 * TEAMS_Y, 800-475-40, 1));
  options_box->AddWidget(new Label(_("Game options:"),rectZero, *normal_font));
  options_box->AddWidget(player_number);
  connected_players = new Label((std::string)"0" + _(" players connected"), rectZero, *normal_font);
  inited_players = new Label((std::string)"0" + _(" players ready"), rectZero, *normal_font);
  options_box->AddWidget(connected_players);
  options_box->AddWidget(inited_players);
  options_box->enabled = false;
  widgets.AddWidget(options_box);

  msg_box = new MessageBox(10, Rectanglei( 475 + 30 + 5, options_box->GetPositionY() + options_box->GetSizeY() + TEAMS_Y, 800-475-40, 1), normal_font);
  widgets.AddWidget(msg_box);

  // Center the boxes!
  uint x = 30;

  /* Choose the teams !! */
  team_box = new VBox(Rectanglei( x, TEAMS_Y, 475, 1));
  team_box->AddWidget(new Label(_("Select the teams:"), rectZero, *normal_font));

  Box * tmp_box = new HBox( Rectanglei(0,0, 1, TEAMS_H), false);
  tmp_box->SetMargin(10);
  tmp_box->SetBorder( Point2i(0,0) );

  lbox_all_teams = new ListBox( Rectanglei( 0, 0, TEAMS_W, TEAMS_H - TEAM_LOGO_H - 5 ));
  lbox_all_teams->always_one_selected = false;
  tmp_box->AddWidget(lbox_all_teams);

  Box * buttons_tmp_box = new VBox(Rectanglei(0, 0, 68, 1), false);

  bt_add_team = new Button( Rectanglei(0, 0, 48, 48) ,res,"menu/arrow-right");
  buttons_tmp_box->AddWidget(bt_add_team);
  
  bt_remove_team = new Button( Rectanglei( 0, 0, 48, 48 ),res,"menu/arrow-left");
  buttons_tmp_box->AddWidget(bt_remove_team);

  space_for_logo = new NullWidget( Rectanglei(0,0,48,48) );
  buttons_tmp_box->AddWidget(space_for_logo);

  tmp_box->AddWidget(buttons_tmp_box);
  
  lbox_selected_teams = new ListBox( Rectanglei(0, 0, TEAMS_W, TEAMS_H - TEAM_LOGO_H - 5 )); 
  lbox_selected_teams->always_one_selected = false;
  tmp_box->AddWidget(lbox_selected_teams);

  team_box->AddWidget(tmp_box);
  team_box->enabled = false;
  widgets.AddWidget(team_box);

  /* Choose the map !! */
  tmp_box = new HBox( Rectanglei(0, 0, 1, MAP_PREVIEW_W - 25 ), false);
  tmp_box->SetMargin(0);
  tmp_box->SetBorder( Point2i(0,0) );

  lbox_maps = new ListBox( Rectanglei(0, 0, MAPS_W, MAP_PREVIEW_W-25 ));
  tmp_box->AddWidget(lbox_maps);
  tmp_box->AddWidget(new NullWidget( Rectanglei(0, 0, MAP_PREVIEW_W+5, MAP_PREVIEW_W)));
  
  map_box = new VBox( Rectanglei(x, team_box->GetPositionY()+team_box->GetSizeY()+20, 475, 1) );
  map_box->AddWidget(new Label(_("Select the world:"), rectZero, *normal_font));
  map_box->AddWidget(tmp_box);
  map_box->enabled = false;
  widgets.AddWidget(map_box);

  // Values initialization

  // Load Maps' list
  std::sort(lst_terrain.liste.begin(), lst_terrain.liste.end(), compareMaps);

  ListeTerrain::iterator
    terrain=lst_terrain.liste.begin(),
    fin_terrain=lst_terrain.liste.end();
  for (; terrain != fin_terrain; ++terrain)
    lbox_maps->AddItem (false, terrain -> name, terrain -> name);
  lbox_maps->Select(0);

  // Load Teams' list
  teams_list.full_list.sort(compareTeams);

  TeamsList::full_iterator
    it=teams_list.full_list.begin(), 
    end=teams_list.full_list.end();

  // No selected team by default
  for (; it != end; ++it)
    lbox_all_teams->AddItem (false, (*it).GetName(), (*it).GetId());

  terrain_init = false;
  b_ok->enabled = false;

  teams_list.Clear();
  Reset();
}

NetworkMenu::~NetworkMenu()
{
}

void NetworkMenu::OnClic(const Point2i &mousePosition, int button)
{     
  ActionHandler * action_handler = ActionHandler::GetInstance();
  Widget* w = widgets.Clic(mousePosition, button);
  if (w == start_client)
  {
    network.Init();
    network.ClientConnect(server_adress->GetText(),WORMUX_NETWORK_PORT);
    if(network.IsConnected())
    {
      team_box->enabled = true;
      map_box->enabled = true;
      lbox_maps->enabled = false;
      connection_box->enabled = false;
      b_ok->enabled = true;
      msg_box->NewMessage(_("Connected to ") + server_adress->GetText());
      msg_box->NewMessage(_("Click the green check when"));
      msg_box->NewMessage(_("you are ready to play!"));
    }
    else
      msg_box->NewMessage(_("Unable to connect"));
  }

  if (w == start_server)
  {
    network.Init();
    network.ServerStart(WORMUX_NETWORK_PORT);
    if(network.IsConnected())
    {
      network.client_inited = 1;
      team_box->enabled = true;
      map_box->enabled = true;
      options_box->enabled = true;
      connection_box->enabled = false;
      msg_box->NewMessage(_("Server started"));
    }
    else
      msg_box->NewMessage(_("Unable to start server"));
  }

  if (w == lbox_maps)
  {
    ChangeMap();
    action_handler->NewAction (new ActionString(ACTION_SET_MAP, TerrainActif().name));
  }

  if (w == bt_add_team) {
    if (lbox_selected_teams->GetItemsList()->size() < GameMode::GetInstance()->max_teams)
    {
      int index = -1;
      teams_list.FindById(lbox_all_teams->ReadValue(),index)->is_local = true;
      std::string team_id = teams_list.FindById(lbox_all_teams->ReadValue(),index)->GetId();
      action_handler->NewAction (new ActionString(ACTION_NEW_TEAM, team_id));
      MoveTeams(lbox_all_teams, lbox_selected_teams, false);
    }
  }
  if (w == bt_remove_team) {
    int index = -1;
    if(teams_list.FindById(lbox_selected_teams->ReadValue(),index)->is_local)
    {
      std::string team_id = teams_list.FindById(lbox_selected_teams->ReadValue(),index)->GetId();
      action_handler->NewAction (new ActionString(ACTION_DEL_TEAM, team_id));
      MoveTeams(lbox_selected_teams, lbox_all_teams, true);
    }
  }
  if(w == player_number)
  {
    network.max_player_number = player_number->GetValue();
  }
}

void NetworkMenu::Reset()
{
  //If we are a client and the server disconnected:
  map_box->enabled = false;
  team_box->enabled = false;
  connection_box->enabled = true;
  lbox_maps->enabled = true;

  // Remove selected teams from the list
  while(lbox_selected_teams->GetItemsList()->size()!=0)
  {
    lbox_selected_teams->Select(0);
    MoveTeams(lbox_selected_teams, lbox_all_teams,true);
  }
  teams_list.Clear();
}

void NetworkMenu::SaveOptions()
{
  teams_list.Clear();
  // teams
  std::vector<list_box_item_t> * 
    selected_teams = lbox_selected_teams->GetItemsList();

  if (selected_teams->size() > 1) {
    std::list<uint> selection;

    std::vector<list_box_item_t>::iterator 
      it = selected_teams->begin(), 
      end = selected_teams->end();

    int index = -1;
    for (; it != end; ++it) {
      teams_list.FindById(it->value, index);
      if (index > -1)
	selection.push_back(uint(index));
    }
    teams_list.ChangeSelection (selection);

  }

  //Save options in XML
//  Config::GetInstance()->Save();
}

void NetworkMenu::__sig_ok()
{
  if(network.IsClient())
  {
    // Wait for the server, and stay in the menu map / team can still be changed
    Action a(ACTION_CHANGE_STATE);
    network.SendAction(&a);
    b_ok->enabled = false;
    b_cancel->enabled = false;
    team_box->enabled = false;
    while(network.state != Network::NETWORK_INIT_GAME)
    {
      Display(Point2i(-1,-1));
    }
  }

  SaveOptions();
  Game::GetInstance()->Start();
}

void NetworkMenu::__sig_cancel()
{
  network.Disconnect();
}

void NetworkMenu::ChangeMap()
{
  std::string map_id = lbox_maps->ReadLabel();
  uint map = lst_terrain.FindMapById(map_id);
  lst_terrain.ChangeTerrainNom(map_id);
  if(terrain_init)
    delete map_preview;
  map_preview = new Sprite(lst_terrain.liste[map].preview);
  float scale = std::min( float(MAP_PREVIEW_W)/map_preview->GetHeight(), 
                          float(MAP_PREVIEW_W)/map_preview->GetWidth() ) ;
  map_preview->Scale (scale, scale);
  terrain_init = true;
}

void NetworkMenu::MoveTeams(ListBox * from, ListBox * to, bool sort)
{
  if (from->GetSelectedItem() != -1) {
    to->AddItem (false, 
		 from->ReadLabel(),
		 from->ReadValue());
    to->Deselect();
    if (sort) to->Sort();
    
    from->RemoveSelected();
  }
}

void NetworkMenu::MoveDisableTeams(ListBox * from, ListBox * to, bool sort)
{
  if (from->GetSelectedItem() != -1) {
    to->AddItem (false, 
		 from->ReadLabel(),
		 from->ReadValue(),
       false);
    to->Deselect();
    if (sort) to->Sort();
    
    from->RemoveSelected();
  }
}

void NetworkMenu::Draw(const Point2i &mousePosition)
{
  if(network.IsConnected())
  {
    Team* last_team = teams_list.FindByIndex(0);
    // Display the flags of the team
    int t = lbox_all_teams->MouseIsOnWhichItem(mousePosition);
    if (t != -1) {
      int index = -1;
      Team * new_team = teams_list.FindById(lbox_all_teams->ReadValue(t), index);
      if (new_team!=NULL) last_team = new_team;
    } else {
      t = lbox_selected_teams->MouseIsOnWhichItem(mousePosition);
      if (t != -1) {
        int index = -1;
        Team * new_team = teams_list.FindById(lbox_selected_teams->ReadValue(t), index);
        if (new_team!=NULL) last_team = new_team;
      }
    }

    AppWormux * app = AppWormux::GetInstance();
    app->video.window.Blit( last_team->flag, space_for_logo->GetPosition() );
  
    // Display the map preview
    if (!terrain_init){
        ChangeMap();
        terrain_init = true;
    }
  
    map_box->Draw(mousePosition);
    map_preview->Blit ( app->video.window, 
		      map_box->GetPositionX()+MAPS_W+10, 
		      map_box->GetPositionY()+map_box->GetSizeY()/2-map_preview->GetHeight()/2);

    //Refresh the number of connected players:
    char nbr[3];
    sprintf(nbr,"%i",network.connected_player);
    std::string pl = (std::string)nbr + _(" players connected");
    if(connected_players->GetText() != pl)
      connected_players->SetText(pl);
    //Refresh the number of players ready:
    sprintf(nbr,"%i",network.client_inited);
    pl = (std::string)nbr + _(" players ready");
    if(inited_players->GetText() != pl)
      inited_players->SetText(pl);

    if(network.IsServer())
    {
      if(network.connected_player > 1 && network.client_inited == network.connected_player)
        b_ok->enabled = true;
      else
        b_ok->enabled = false;
      // Check for newly connected client:
      for(std::list<TCPsocket>::iterator adr=network.conn.begin();
          adr != network.conn.end();
          adr++)
      {
        IPaddress* ip = SDLNet_TCP_GetPeerAddress(*adr);
        std::string address = SDLNet_ResolveIP(ip);
        if(std::find(connected_client.begin(),connected_client.end(),address) == connected_client.end())
        {
          msg_box->NewMessage(address + _(" has joined the party"));
          connected_client.push_back(address);
        }
      }
      // Check for disconnected client:
      std::list<std::string>::iterator str=connected_client.begin();
      while(str != connected_client.end())
      {
        bool found = false;
        for(std::list<TCPsocket>::iterator adr=network.conn.begin();
            adr != network.conn.end();
            adr++)
        {
          IPaddress* ip = SDLNet_TCP_GetPeerAddress(*adr);
          std::string address = SDLNet_ResolveIP(ip);
          if(address == *str)
          {
            found = true;
            break;
          }
        }
        if(!found)
        {
          msg_box->NewMessage(*str + _(" just left the party"));
          str = connected_client.erase(str);
        }
        else
          str++;
      }
    }
    ActionHandler * action_handler = ActionHandler::GetInstance();
    action_handler->ExecActions();

    if(network.IsClient())
    {
      //Check for changes sent by the server
      if(network.conn.size()==0)
      {
        network.Disconnect();
        Reset();
      }
      // Check map changement
      if( TerrainActif().name != lbox_maps->ReadLabel() )
      {
        msg_box->NewMessage(_("Chosen map : ") + TerrainActif().name);
        lbox_maps->Select(TerrainActif().name);
        ChangeMap();
      }
    }
    //Check for team addition:
    for(TeamsList::iterator it=teams_list.playing_list.begin();
        it != teams_list.playing_list.end();
        it++)
    {
      bool found = false;
      std::string id = (*it)->GetId();
      for(std::vector<list_box_item_t>::iterator lst_it = lbox_selected_teams->GetItemsList()->begin();
          lst_it != lbox_selected_teams->GetItemsList()->end();
          lst_it++)
      {
        if(lst_it->value == id)
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        (*it)->is_local = false;
        lbox_all_teams->Select((*it)->GetName());
        msg_box->NewMessage((*it)->GetName() + " selected");
        MoveDisableTeams(lbox_all_teams, lbox_selected_teams, false);        
      }
    }
    //Check for team removal:
    for(std::vector<list_box_item_t>::iterator lst_it = lbox_selected_teams->GetItemsList()->begin();
        lst_it != lbox_selected_teams->GetItemsList()->end();
        lst_it++)
    {
      bool found = false;
      for(TeamsList::iterator it=teams_list.playing_list.begin();
          it != teams_list.playing_list.end();
          it++)
      {
        std::string id = (*it)->GetId();
        if(lst_it->value == id)
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        lbox_selected_teams->Select(lst_it->label);
        msg_box->NewMessage(lst_it->label + " unselected");
        MoveTeams(lbox_selected_teams, lbox_all_teams, true);
      }
    }
  }
}
