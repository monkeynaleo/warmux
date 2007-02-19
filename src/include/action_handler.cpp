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
 * Wormux action handler.
 *****************************************************************************/

#include "action_handler.h"
#include "action.h"
#include "../character/body.h"
#include "../character/move.h"
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../game/game.h"
#include "../game/time.h"
#include "../include/constant.h"
#include "../network/network.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../map/maps_list.h"
#include "../map/wind.h"
#include "../menu/network_menu.h"
#include "../network/randomsync.h"
#include "../network/network.h"
#include "../team/macro.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
#include "../tool/vector2.h"
#include "../weapon/construct.h"
#include "../weapon/launcher.h"
#include "../weapon/supertux.h"
#include "../weapon/weapon.h"
#include "../weapon/weapons_list.h"
#include "../weapon/explosion.h"

// Delta angle used to move the crosshair
#define DELTA_CROSSHAIR 0.035 /* ~1 degree */

ActionHandler * ActionHandler::singleton = NULL;

ActionHandler * ActionHandler::GetInstance()
{
  if (singleton == NULL)
    singleton = new ActionHandler();
  return singleton;
}

// ########################################################
// ########################################################

void Action_Nickname(Action *a)
{

}

void Action_Network_ChangeState (Action *a)
{
  MSG_DEBUG("action.handler", "ChangeState");

  if(network.IsServer())
  {
    switch(network.state)
    {
    case Network::NETWORK_OPTION_SCREEN:
      // State is changed when server clicks on the launch game button
      network.client_inited++;
      break;
    case Network::NETWORK_INIT_GAME:
      // One more client is ready to play
      network.client_inited++;
      if(network.client_inited == network.connected_player)
        network.state = Network::NETWORK_READY_TO_PLAY;
      break;
    default:
      assert(false);
      break;
    }
  }

  if(network.IsClient())
  {
    switch(network.state)
    {
    case Network::NETWORK_OPTION_SCREEN:
      network.state = Network::NETWORK_INIT_GAME;
      break;
    case Network::NETWORK_INIT_GAME:
      network.state = Network::NETWORK_READY_TO_PLAY;
      break;
    case Network::NETWORK_READY_TO_PLAY:
      network.state = Network::NETWORK_PLAYING;
      break;
    default:
       assert(false);
    }
  }
}

// ########################################################

void Action_Player_ChangeWeapon (Action *a)
{
  ActiveTeam().SetWeapon(static_cast<Weapon::Weapon_type>(a->PopInt()));
}

void Action_Player_NextCharacter (Action *a)
{
  a->RetrieveCharacter();       // Retrieve current character's informations
  a->RetrieveCharacter();       // Retrieve next character information
  camera.FollowObject(&ActiveCharacter(), true, true);
}

void Action_GameLoop_ChangeCharacter (Action *a)
{
  a->RetrieveCharacter();
  camera.FollowObject(&ActiveCharacter(), true, true);
}

void Action_GameLoop_NextTeam (Action *a)
{
  teams_list.SetActive (a->PopString());
  ActiveTeam().PrepareTurn();
  assert (!ActiveCharacter().IsDead());
}

// ########################################################

void Action_Rules_SetGameMode (Action *a)
{
  assert(network.IsClient());
  GameMode::GetInstance()->LoadFromString(a->PopString());

//   GameMode::GetInstance()->max_characters = a->PopInt();
//   GameMode::GetInstance()->max_teams = a->PopInt();
//   GameMode::GetInstance()->duration_turn = a->PopInt();
//   GameMode::GetInstance()->duration_exchange_player = a->PopInt();
//   GameMode::GetInstance()->duration_before_death_mode = a->PopInt();
//   GameMode::GetInstance()->gravity = a->PopDouble();
//   GameMode::GetInstance()->safe_fall = a->PopDouble();
//   GameMode::GetInstance()->damage_per_fall_unit = a->PopDouble();
//   GameMode::GetInstance()->duration_move_player = a->PopInt();
//   GameMode::GetInstance()->allow_character_selection = a->PopInt();
//   GameMode::GetInstance()->character.init_energy = a->PopInt();
//   GameMode::GetInstance()->character.max_energy = a->PopInt();
//   GameMode::GetInstance()->character.mass = a->PopInt();
//   GameMode::GetInstance()->character.air_resist_factor = a->PopDouble();
//   GameMode::GetInstance()->character.jump_strength = a->PopInt();
//   GameMode::GetInstance()->character.jump_angle = a->PopDouble();
//   GameMode::GetInstance()->character.super_jump_strength = a->PopInt();
//   GameMode::GetInstance()->character.super_jump_angle = a->PopDouble();
//   GameMode::GetInstance()->character.back_jump_strength = a->PopInt();
//   GameMode::GetInstance()->character.back_jump_angle = a->PopDouble();
}

void SendGameMode()
{
  assert(network.IsServer());
  Action a(Action::ACTION_RULES_SET_GAME_MODE);
  
  std::string contents;
  GameMode::GetInstance()->ExportToString(contents);
  
  a.Push(contents);
//   a.Push((int)GameMode::GetInstance()->max_characters);
//   a.Push((int)GameMode::GetInstance()->max_teams);
//   a.Push((int)GameMode::GetInstance()->duration_turn);
//   a.Push((int)GameMode::GetInstance()->duration_exchange_player);
//   a.Push((int)GameMode::GetInstance()->duration_before_death_mode);
//   a.Push(GameMode::GetInstance()->gravity);
//   a.Push(GameMode::GetInstance()->safe_fall);
//   a.Push(GameMode::GetInstance()->damage_per_fall_unit);
//   a.Push((int)GameMode::GetInstance()->duration_move_player);
//   a.Push(GameMode::GetInstance()->allow_character_selection);
//   a.Push((int)GameMode::GetInstance()->character.init_energy);
//   a.Push((int)GameMode::GetInstance()->character.max_energy);
//   a.Push((int)GameMode::GetInstance()->character.mass);
//   a.Push(GameMode::GetInstance()->character.air_resist_factor);
//   a.Push((int)GameMode::GetInstance()->character.jump_strength);
//   a.Push(GameMode::GetInstance()->character.jump_angle);
//   a.Push((int)GameMode::GetInstance()->character.super_jump_strength);
//   a.Push(GameMode::GetInstance()->character.super_jump_angle);
//   a.Push((int)GameMode::GetInstance()->character.back_jump_strength);
//   a.Push(GameMode::GetInstance()->character.back_jump_angle);
  network.SendAction(&a);
}

void Action_Rules_AskVersion (Action *a)
{
  if (!network.IsClient()) return;
  ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_RULES_SEND_VERSION, Constants::VERSION));
}

void Action_Rules_SendVersion (Action *a)
{
  if (!network.IsServer()) return;
  std::string version= a->PopString();
  if (version != Constants::VERSION)
  {
    Error(Format(_("Wormux versions are differents : client=%s, server=%s."),
    version.c_str(), Constants::VERSION.c_str()));
  }
}

// ########################################################

// TODO: Move this into network/distant_cpu.cpp
void Action_ChatMessage (Action *a)
{
  if(Game::GetInstance()->IsGameLaunched())
    //Add message to chat session in Game
    //    GameLoop::GetInstance()->chatsession.chat->AddText(a->PopString());
    GameLoop::GetInstance()->chatsession.NewMessage(a->PopString());
  else
    //Network Menu
    network.network_menu->ReceiveMsgCallback(a->PopString());
}

void Action_Menu_SetMap (Action *a)
{
  if (!network.IsClient()) return;
  MapsList::GetInstance()->SelectMapByName(a->PopString());
  network.network_menu->ChangeMapCallback();
}

// TODO: Move this into network/distant_cpu.cpp
void Action_Menu_AddTeam (Action *a)
{
  ConfigTeam the_team;

  the_team.id = a->PopString();
  the_team.player_name = a->PopString();
  the_team.nb_characters = uint(a->PopInt());

  teams_list.AddTeam (the_team);

  network.network_menu->AddTeamCallback(the_team.id);
}

void Action_Menu_UpdateTeam (Action *a)
{
  ConfigTeam the_team;

  the_team.id = a->PopString();
  the_team.player_name = a->PopString();
  the_team.nb_characters = uint(a->PopInt());

  teams_list.UpdateTeam (the_team);

  network.network_menu->UpdateTeamCallback(the_team.id);
}

// TODO: Move this into network/distant_cpu.cpp
void Action_Menu_DelTeam (Action *a)
{
  std::string team = a->PopString();
  teams_list.DelTeam (team);
  network.network_menu->DelTeamCallback(team);
}

// ########################################################

// Send information about energy and the position of every character
void SyncCharacters()
{
  assert(network.IsServer());
  ActionHandler* action_handler = ActionHandler::GetInstance();

  Action a_begin_sync(Action::ACTION_NETWORK_SYNC_BEGIN);
  network.SendAction(&a_begin_sync);
  TeamsList::iterator
    it=teams_list.playing_list.begin(),
    end=teams_list.playing_list.end();

  for (int team_no = 0; it != end; ++it, ++team_no)
  {
    Team& team = **it;
    Team::iterator
        tit = team.begin(),
        tend = team.end();

    for (int char_no = 0; tit != tend; ++tit, ++char_no)
    {
      // Sync the character's position, energy, ...
      Action * a = BuildActionSendCharacterPhysics(team_no, char_no);
      action_handler->NewAction(a);
    }
  }
  Action a_sync_end(Action::ACTION_NETWORK_SYNC_END);
  network.SendAction(&a_sync_end);
}

void Action_Character_MoveRight (Action *a)
{
  assert(false);
  MoveCharacterRight (ActiveCharacter());
}

void Action_Character_MoveLeft (Action *a)
{
  assert(false);
  MoveCharacterLeft (ActiveCharacter());
}

void Action_Character_Up (Action *a)
{
  ActiveCharacter().AddFiringAngle(-DELTA_CROSSHAIR);
}

void Action_Character_Down (Action *a)
{
  ActiveCharacter().AddFiringAngle(DELTA_CROSSHAIR);
}

void Action_Character_Jump (Action *a)
{
  GameLoop::GetInstance()->character_already_chosen = true;
  ActiveCharacter().Jump();
}

void Action_Character_HighJump (Action *a)
{
  GameLoop::GetInstance()->character_already_chosen = true;
  ActiveCharacter().HighJump();
}

void Action_Character_BackJump (Action *a)
{
  GameLoop::GetInstance()->character_already_chosen = true;
  ActiveCharacter().BackJump();
}

void Action_Character_SetPhysics (Action *a)
{
  while(!a->IsEmpty())
    a->RetrieveCharacter();
}

void Action_Character_SetSkin (Action *a)
{
  //Set the frame of the walking skin, to get the position of the hand synced
  if (!ActiveTeam().IsLocal() || network.state != Network::NETWORK_PLAYING)
  {
    ActiveTeam().ActiveCharacter().SetClothe(a->PopString());
    ActiveTeam().ActiveCharacter().SetMovement(a->PopString());
    ActiveTeam().ActiveCharacter().body->SetFrame((uint)a->PopInt());
  }
}

Action* BuildActionSendCharacterPhysics(int team_no, int char_no)
{
  Action* a = new Action(Action::ACTION_CHARACTER_SET_PHYSICS);
  a->StoreCharacter(team_no, char_no);
  return a;
}

// ########################################################

void Action_Weapon_Shoot (Action *a)
{
  double strength = a->PopDouble();
  double angle = a->PopDouble();
  a->RetrieveCharacter();
  ActiveTeam().AccessWeapon().PrepareShoot(strength, angle);
}

void Action_Weapon_StopUse(Action *a)
{
  ActiveTeam().AccessWeapon().ActionStopUse();
}

void Action_Weapon_SetTarget (Action *a)
{
  MSG_DEBUG("action.handler", "Set target by clicking");

  Point2i target;
  target.x = a->PopInt();
  target.y = a->PopInt();

  ActiveTeam().AccessWeapon().ChooseTarget (target);
}

void Action_Weapon_SetTimeout (Action *a)
{
  WeaponLauncher* launcher = dynamic_cast<WeaponLauncher*>(&(ActiveTeam().AccessWeapon()));
  assert(launcher != NULL);
  launcher->GetProjectile()->m_timeout_modifier = a->PopInt();
}

void Action_Weapon_SupertuxState (Action *a)
{
  assert(ActiveTeam().GetWeaponType() == Weapon::WEAPON_SUPERTUX);
  WeaponLauncher* launcher = static_cast<WeaponLauncher*>(&(ActiveTeam().AccessWeapon()));
  SuperTux* tux = static_cast<SuperTux*>(launcher->GetProjectile());

  double x, y;

  tux->SetAngle(a->PopDouble());
  x = a->PopDouble();
  y = a->PopDouble();
  tux->SetPhysXY(x, y);
  tux->SetSpeedXY(Point2d(0,0));
}

void Action_Weapon_ConstructionUp (Action *a)
{
  Construct* launcher = dynamic_cast<Construct*>(&(ActiveTeam().AccessWeapon()));
  assert(launcher != NULL);
  launcher->Up();
}

void Action_Weapon_ConstructionDown (Action *a)
{
  Construct* launcher = dynamic_cast<Construct*>(&(ActiveTeam().AccessWeapon()));
  assert(launcher != NULL);
  launcher->Down();
}

// ########################################################

void Action_Wind (Action *a)
{
  wind.SetVal (a->PopInt());
}

void Action_Network_SendRandom (Action *a)
{
  if (!network.IsClient()) return;
  randomSync.AddToTable(a->PopDouble());
}

void Action_Network_SyncBegin (Action *a)
{
  assert(!network.sync_lock);
  network.sync_lock = true;
}

void Action_Network_SyncEnd (Action *a)
{
  assert(network.sync_lock);
  network.sync_lock = false;
}

// Nothing to do here. Just for time synchronisation
void Action_Network_Ping(Action *a)
{
}

void Action_Explosion (Action *a)
{
  Point2i pos;
  ExplosiveWeaponConfig config;

  pos.x = a->PopInt();
  pos.y = a->PopInt();
  config.explosion_range = a->PopInt();
  config.particle_range = a->PopInt();
  config.damage = a->PopInt();
  config.blast_range = a->PopDouble();
  config.blast_force = a->PopDouble();
  std::string son = a->PopString();
  bool fire_particle = a->PopInt();
  ParticleEngine::ESmokeStyle smoke = (ParticleEngine::ESmokeStyle)a->PopInt();

  ApplyExplosion_common(pos, config, son, fire_particle, smoke);
}

// ########################################################
// ########################################################
// ########################################################

void ActionHandler::ExecActions()
{
  Action * a;
  std::list<Action*> to_remove;
  std::list<Action*>::iterator it;
  assert(mutex!=NULL);
  for(it = queue.begin(); it != queue.end() ; ++it)
  {
    SDL_LockMutex(mutex);
    a = (*it);
    Time::GetInstance()->RefreshMaxTime((*it)->GetTimestamp());
    // If action is in the future, wait for next refresh
    if((*it)->GetTimestamp() > Time::GetInstance()->Read()) {
      SDL_UnlockMutex(mutex);
      continue;
    }
    SDL_UnlockMutex(mutex);
    Exec ((*it));
    to_remove.push_back((*it));
  }
  while(to_remove.size() != 0)
  {
    a = to_remove.front();
    to_remove.pop_front();
    queue.remove(a);
    delete(a);
  }
}

void ActionHandler::NewAction(Action* a, bool repeat_to_network)
{
  assert(mutex!=NULL);
  SDL_LockMutex(mutex);
  //  MSG_DEBUG("action.handler","New action : %s",a.out());
  //  std::cout << "New action " << a->GetType() << std::endl ;
  queue.push_back(a);
  //  std::cout << "  queue_size " << queue.size() << std::endl;
  SDL_UnlockMutex(mutex);
  if (repeat_to_network) network.SendAction(a);
}

void ActionHandler::Register (Action::Action_t action,
		                      const std::string &name,callback_t fct)
{
  handler[action] = fct;
  action_name[action] = name;
}

void ActionHandler::Exec(Action *a)
{
  MSG_DEBUG("action_handler", "Executing action %s",GetActionName(a->GetType()).c_str());
  handler_it it=handler.find(a->GetType());
  assert(it != handler.end());
  (*it->second) (a);
}

std::string ActionHandler::GetActionName (Action::Action_t action)
{
  assert(mutex!=NULL);
  SDL_LockMutex(mutex);
  name_it it=action_name.find(action);
  assert(it != action_name.end());
  SDL_UnlockMutex(mutex);
  return it->second;
}

ActionHandler::ActionHandler()
{
  mutex = SDL_CreateMutex();
  SDL_LockMutex(mutex);

  // ########################################################
  Register (Action::ACTION_NICKNAME, "nickname", Action_Nickname);
  Register (Action::ACTION_NETWORK_CHANGE_STATE, "NETWORK_change_state", &Action_Network_ChangeState);

  // ########################################################
  Register (Action::ACTION_PLAYER_CHANGE_WEAPON, "PLAYER_change_weapon", &Action_Player_ChangeWeapon);
  Register (Action::ACTION_PLAYER_NEXT_CHARACTER, "PLAYER_next_character", &Action_Player_NextCharacter);
  Register (Action::ACTION_GAMELOOP_CHANGE_CHARACTER, "GAMELOOP_change_character", &Action_GameLoop_ChangeCharacter);
  Register (Action::ACTION_GAMELOOP_NEXT_TEAM, "GAMELOOP_change_team", &Action_GameLoop_NextTeam);

  // ########################################################
  // To be sure that rules will be the same on each computer  
  Register (Action::ACTION_RULES_ASK_VERSION, "RULES_ask_version", &Action_Rules_AskVersion);
  Register (Action::ACTION_RULES_SEND_VERSION, "RULES_send_version", &Action_Rules_SendVersion);
  Register (Action::ACTION_RULES_SET_GAME_MODE, "RULES_set_game_mode", &Action_Rules_SetGameMode);

  // ########################################################
  // Chat message
  Register (Action::ACTION_CHAT_MESSAGE, "chat_message", Action_ChatMessage);  

  // Map selection in network menu
  Register (Action::ACTION_MENU_SET_MAP, "MENU_set_map", &Action_Menu_SetMap);

  // Teams selection in network menu
  Register (Action::ACTION_MENU_ADD_TEAM, "MENU_add_team", &Action_Menu_AddTeam);
  Register (Action::ACTION_MENU_DEL_TEAM, "MENU_del_team", &Action_Menu_DelTeam);
  Register (Action::ACTION_MENU_UPDATE_TEAM, "MENU_update_team", &Action_Menu_UpdateTeam);

  // ########################################################
  // Character's move
  Register (Action::ACTION_CHARACTER_MOVE_LEFT, "CHARACTER_move_left", &Action_Character_MoveLeft);
  Register (Action::ACTION_CHARACTER_MOVE_RIGHT, "CHARACTER_move_right", &Action_Character_MoveRight);
  Register (Action::ACTION_CHARACTER_UP, "CHARACTER_up", &Action_Character_Up);
  Register (Action::ACTION_CHARACTER_DOWN, "CHARACTER_down", &Action_Character_Down);
  Register (Action::ACTION_CHARACTER_JUMP, "CHARACTER_jump", &Action_Character_Jump);
  Register (Action::ACTION_CHARACTER_HIGH_JUMP, "CHARACTER_super_jump", &Action_Character_HighJump);
  Register (Action::ACTION_CHARACTER_BACK_JUMP, "CHARACTER_back_jump", &Action_Character_BackJump);

  Register (Action::ACTION_CHARACTER_SET_PHYSICS, "CHARACTER_set_physics", &Action_Character_SetPhysics);
  Register (Action::ACTION_CHARACTER_SET_SKIN, "CHARACTER_set_skin", &Action_Character_SetSkin);

  // ########################################################
  // Using Weapon
  Register (Action::ACTION_WEAPON_SHOOT, "WEAPON_shoot", &Action_Weapon_Shoot);
  Register (Action::ACTION_WEAPON_STOP_USE, "WEAPON_stop_use", &Action_Weapon_StopUse);

  // Quite standard weapon options
  Register (Action::ACTION_WEAPON_SET_TIMEOUT, "WEAPON_set_timeout", &Action_Weapon_SetTimeout);
  Register (Action::ACTION_WEAPON_SET_TARGET, "WEAPON_set_target", &Action_Weapon_SetTarget);

  // Special weapon options
  Register (Action::ACTION_WEAPON_SUPERTUX_STATE, "WEAPON_supertux_state", &Action_Weapon_SupertuxState);
  Register (Action::ACTION_WEAPON_CONSTRUCTION_UP, "WEAPON_construction_up", &Action_Weapon_ConstructionUp);
  Register (Action::ACTION_WEAPON_CONSTRUCTION_DOWN, "WEAPON_construction_down", &Action_Weapon_ConstructionDown);
 
  // ########################################################
  Register (Action::ACTION_NETWORK_SYNC_BEGIN, "NETWORK_sync_begin", &Action_Network_SyncBegin);
  Register (Action::ACTION_NETWORK_SYNC_END, "NETWORK_sync_end", &Action_Network_SyncEnd);
  Register (Action::ACTION_NETWORK_PING, "NETWORK_ping", &Action_Network_Ping);

  Register (Action::ACTION_EXPLOSION, "explosion", &Action_Explosion);  
  Register (Action::ACTION_WIND, "wind", &Action_Wind);
  Register (Action::ACTION_NETWORK_SEND_RANDOM, "NETWORK_send_random", &Action_Network_SendRandom);

  // ########################################################
  SDL_UnlockMutex(mutex);
}

