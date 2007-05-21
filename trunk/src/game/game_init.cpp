/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Game loop : drawing and data handling
 *****************************************************************************/

#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <iostream>
#include "game_init.h"
#include "game_loop.h"
#include "config.h"
#include "game.h"
#include "game_mode.h"
#include "time.h"
#include "ai/ai_engine.h"
#include "graphic/video.h"
#include "include/action_handler.h"
#include "include/app.h"
#include "include/constant.h"
#include "interface/cursor.h"
#include "interface/game_msg.h"
#include "interface/interface.h"
#include "interface/keyboard.h"
#include "interface/loading_screen.h"
#include "interface/mouse.h"
#include "map/camera.h"
#include "map/map.h"
#include "map/maps_list.h"
#include "map/wind.h"
#include "network/network.h"
#include "network/network_server.h"
#include "network/randomsync.h"
#include "object/bonus_box.h"
#include "object/medkit.h"
#include "object/objects_list.h"
#include "particles/particle.h"
#include "sound/jukebox.h"
#include "team/macro.h"
#include "tool/debug.h"
#include "tool/i18n.h"
#include "tool/stats.h"
#include "weapon/weapons_list.h"

void GameInit::InitGameData_NetServer()
{
  Network::GetInstanceServer()->RejectIncoming();

  randomSync.Init();

  SendGameMode();

  Action a_change_state(Action::ACTION_NETWORK_CHANGE_STATE);
  Network::GetInstance()->SendAction ( &a_change_state );
  Network::GetInstance()->state = Network::NETWORK_INIT_GAME;

  GameMode::GetInstance()->Load();

  // Load maps
  InitMap();

  // Load teams
  InitTeams();

  // Tells all clients that the server is ready to play
  Network::GetInstance()->SendAction ( &a_change_state );

  // Wait for all clients to be ready to play
  while (Network::GetInstance()->state != Network::NETWORK_READY_TO_PLAY 
	 && Network::IsConnected())
  {
    ActionHandler::GetInstance()->ExecActions();
    SDL_Delay(200);
  }
  Network::GetInstance()->SendAction ( &a_change_state );
  Network::GetInstance()->state = Network::NETWORK_PLAYING;
}

void GameInit::InitGameData_NetClient()
{
  //GameMode::GetInstance()->Load(); : done by the action handler

  // Loading map
  InitMap();

  // Loading teams
  InitTeams();

  // Tells server that client is ready
  Action a_change_state(Action::ACTION_NETWORK_CHANGE_STATE);

  Network::GetInstance()->SendAction (&a_change_state);
  while (Network::GetInstance()->state != Network::NETWORK_READY_TO_PLAY
	 && Network::IsConnected())
  {
    // The server is placing characters on the map
    // We can receive new team / map selection
    ActionHandler::GetInstance()->ExecActions();
    SDL_Delay(100);
  }

  // Waiting for other clients
  std::cout << Network::GetInstance()->state << " : Waiting for people over the network" << std::endl;
  while (Network::GetInstance()->state != Network::NETWORK_PLAYING)
  {
    // The server waits for everybody to be ready to start
    ActionHandler::GetInstance()->ExecActions();
    SDL_Delay(100);
  }
  std::cout << Network::GetInstance()->state << " : Run game !" << std::endl;
}

void GameInit::InitGameData_Local()
{
  // GameMode::GetInstance()->Load(); : done in the game menu to adjust some parameters

  // Load the map
  InitMap();

  // Init teams
  InitTeams();
}

void GameInit::InitMap()
{
  std::cout << "o " << _("Initialise map") << std::endl;

  LoadingScreen::GetInstance()->StartLoading(1, "map_icon", _("Maps"));
  world.Reset();
  MapsList::GetInstance()->ActiveMap().FreeData();

  lst_objects.PlaceBarrels();
}

void GameInit::InitTeams()
{
  std::cout << "o " << _("Initialise teams") << std::endl;

  LoadingScreen::GetInstance()->StartLoading(2, "team_icon", _("Teams"));

  // Check the number of teams
  if (teams_list.playing_list.size() < 2)
    Error(_("You need at least two valid teams !"));
  assert (teams_list.playing_list.size() <= GameMode::GetInstance()->max_teams);

  // Load the teams
  teams_list.LoadGamingData();

  // Initialization of teams' energy
  LoadingScreen::GetInstance()->StartLoading(3, "weapon_icon", _("Weapons")); // use fake message...
  teams_list.InitEnergy();

  lst_objects.PlaceMines();
}

void GameInit::InitSounds()
{
  std::cout << "o " << _("Initialise sounds") << std::endl;

  // Load teams' sound profiles
  LoadingScreen::GetInstance()->StartLoading(4, "sound_icon", _("Sounds"));

  jukebox.LoadXML("default");
  FOR_EACH_TEAM(team)
    if ( (**team).GetSoundProfile() != "default" )
      jukebox.LoadXML((**team).GetSoundProfile()) ;
}

void GameInit::InitData()
{
  std::cout << "o " << _("Initialisation") << std::endl;
  Time::GetInstance()->Reset();

  // initialize gaming data
  if (Network::GetInstance()->IsServer())
    InitGameData_NetServer();
  else if (Network::GetInstance()->IsClient())
    InitGameData_NetClient();
  else
    InitGameData_Local();

  InitSounds();
}

void GameInit::Init()
{
  // Disable sound during the loading of data
  bool enable_sound = jukebox.UseEffects();
  jukebox.ActiveEffects(false);

  // Display Loading screen
  LoadingScreen::GetInstance()->DrawBackground();
  Mouse::GetInstance()->Hide();

  Game::GetInstance()->MessageLoading();

  // Init all needed data
  InitData();

  CharacterCursor::GetInstance()->Reset();
  Keyboard::GetInstance()->Reset();

  Interface::GetInstance()->Reset();
  GameMessages::GetInstance()->Reset();

  ParticleEngine::Load();

  Mouse::GetInstance()->SetPointer(Mouse::POINTER_SELECT);

  // First "selection" of a weapon -> fix bug 6576
  ActiveTeam().AccessWeapon().Select();

  // Loading is finished, sound effects can be enabled again
  jukebox.ActiveEffects(enable_sound);

  GameLoop::GetInstance()->Init();
}

