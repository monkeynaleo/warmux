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
 * Wormux action handler.
 *****************************************************************************/

#include "action_handler.h"
#include "action.h"
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../include/constant.h"
#include "../network/network.h"
#include "../map/map.h"
#include "../map/maps_list.h"
#include "../map/wind.h"
#include "../network/randomsync.h"
#include "../network/network.h"
#include "../team/macro.h"
#include "../team/move.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
#include "../weapon/weapon.h"
#include "../weapon/weapons_list.h"
#include "../weapon/explosion.h"

// Delta appliqu��l'angle du viseur
#define DELTA_CROSSHAIR 2

ActionHandler * ActionHandler::singleton = NULL;

ActionHandler * ActionHandler::GetInstance()
{
  if (singleton == NULL)
    singleton = new ActionHandler();
  return singleton;
}

void Action_Walk (Action *a)
{
  assert(false);
  MoveCharacter (ActiveCharacter());
}

void Action_MoveRight (Action *a)
{
  assert(false);
  MoveCharacterRight (ActiveCharacter());
}

void Action_MoveLeft (Action *a)
{
  assert(false);
  MoveCharacterLeft (ActiveCharacter());
}

void Action_Jump (Action *a)
{
  GameLoop::GetInstance()->character_already_chosen = true;
  ActiveCharacter().Jump(); 
}

void Action_HighJump (Action *a)
{
  GameLoop::GetInstance()->character_already_chosen = true;
  ActiveCharacter().HighJump();
}

void Action_Up (Action *a)
{
  ActiveTeam().crosshair.ChangeAngle (-DELTA_CROSSHAIR);
}

void Action_Down (Action *a)
{
  ActiveTeam().crosshair.ChangeAngle (DELTA_CROSSHAIR);
}

void Action_ChangeWeapon (Action *a)
{
  ActionInt* ai = dynamic_cast<ActionInt*>(a);
  ActiveTeam().SetWeapon((Weapon_type)ai->GetValue());
}

void Action_ChangeCharacter (Action *a)
{
  ActiveTeam().NextCharacter();
}

void Action_Shoot (Action *action)
{
  ActionDoubleInt* a = dynamic_cast<ActionDoubleInt*>(action);
  ActiveTeam().AccessWeapon().Shoot(a->GetValue1(), a->GetValue2());
}

void Action_Wind (Action *a)
{
  ActionInt* ai = dynamic_cast<ActionInt*>(a);
  wind.SetVal (ai->GetValue());
}

void Action_MoveCharacter (Action *a)
{
  ActionInt2* ap = dynamic_cast<ActionInt2*>(a);
  ActiveCharacter().SetXY (Point2i(ap->GetValue1(), ap->GetValue2()));
}

void Action_SetCharacterSpeed (Action *a)
{
  ActionDouble2* ap = dynamic_cast<ActionDouble2*>(a);
  ActiveCharacter().SetSpeedXY (Point2d(ap->GetValue1(), ap->GetValue2()));
}

void Action_SetFrame (Action *a)
{
  //Set the frame of the walking skin, to get the position of the hand synced
  ActionInt* ai = dynamic_cast<ActionInt*>(a);
  if (!ActiveTeam().is_local || network.state != Network::NETWORK_PLAYING)
  {
    ActiveTeam().ActiveCharacter().body->SetFrame((uint)ai->GetValue());
  }
}

void Action_SetClothe (Action *a)
{
  ActionString* action = dynamic_cast<ActionString*>(a);
  if (!ActiveTeam().is_local || network.state != Network::NETWORK_PLAYING)
  {
    ActiveTeam().ActiveCharacter().SetClothe(action->GetValue());
  }
}

void Action_SetMovement (Action *a)
{
  ActionString* action = dynamic_cast<ActionString*>(a);
  if (!ActiveTeam().is_local || network.state != Network::NETWORK_PLAYING)
  {
    ActiveTeam().ActiveCharacter().SetMovement(action->GetValue());
  }
}

void Action_SetCharacterDirection (Action *a)
{
  ActionInt* ai = dynamic_cast<ActionInt*>(a);
  ActiveCharacter().SetDirection (ai->GetValue());
}

void Action_SetMap (Action *a)
{
  ActionString* action = dynamic_cast<ActionString*>(a);
  MSG_DEBUG("action.handler", "SetMap : %s", action->GetValue());
  if (!network.IsClient()) return;
  lst_terrain.ChangeTerrainNom (action->GetValue());
}

void Action_ClearTeams (Action *a)
{
  MSG_DEBUG("action.handler", "ClearTeams");
  if (!network.IsClient()) return;
  teams_list.Clear();
}

void Action_ChangeState (Action *a)
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

void Action_SetGameMode (Action *a)
{
  ActionString* action = dynamic_cast<ActionString*>(a);	
  MSG_DEBUG("action.handler", "SetGameMode : %s", action->GetValue());
  GameMode::GetInstance()->Load (action->GetValue());
}


void Action_NewTeam (Action *a)
{
  ActionString* action = dynamic_cast<ActionString*>(a);
  MSG_DEBUG("action.handler", "NewTeam : %s", action->GetValue());

  teams_list.AddTeam (action->GetValue());
}

void Action_DelTeam (Action *a)
{
  ActionString* action = dynamic_cast<ActionString*>(a);
  MSG_DEBUG("action.handler", "DelTeam : %s", action->GetValue());

  teams_list.DelTeam (action->GetValue());
}

void Action_ChangeTeam (Action *a)
{
  ActionString* action = dynamic_cast<ActionString*>(a);
  MSG_DEBUG("action.handler", "ChangeTeam : %s", action->GetValue());
//  if (!network.IsClient()) return;
  teams_list.SetActive (std::string(action->GetValue()));
  ActiveTeam().PrepareTurn();
  assert (!ActiveCharacter().IsDead());
}

void Action_AskVersion (Action *a)
{
  if (!network.IsClient()) return;
  ActionHandler::GetInstance()->NewAction(new ActionString(ACTION_SEND_VERSION, Constants::VERSION));
}

void Action_SendVersion (Action *a)
{
  if (!network.IsServer()) return;
  ActionString* action = dynamic_cast<ActionString*>(a);
  if (action->GetValue() != Constants::VERSION)
  {
    Error(Format(_("Wormux versions are differents : client=%s, server=%s."),
    action->GetValue(), Constants::VERSION.c_str()));
  }
}

void Action_SendRandom (Action *a)
{
  if (!network.IsClient()) return;
  ActionDouble* action = dynamic_cast<ActionDouble*>(a);
  randomSync.AddToTable(action->GetValue());
}

void Action_SyncBegin (Action *a)
{
  MSG_DEBUG("action.handler", "ChangeState");
  assert(!network.sync_lock);
  network.sync_lock = true;
}

void Action_SyncEnd (Action *a)
{
  MSG_DEBUG("action.handler", "ChangeState");
  assert(network.sync_lock);
  network.sync_lock = false;
}

void Action_Explosion (Action *a)
{
  MSG_DEBUG("action.handler", "Explosion !");

  Point2i pos;
  ExplosiveWeaponConfig config;

  pos.x = a->PopInt();
  pos.y = a->PopInt();
  config.explosion_range = a->PopInt();
  config.damage = a->PopInt();
  config.blast_range = a->PopDouble();
  config.blast_force = a->PopDouble();
  std::string son = a->PopString();
  bool fire_particle = a->PopInt();
  ParticleEngine::ESmokeStyle smoke = (ParticleEngine::ESmokeStyle)a->PopInt();

  ApplyExplosion_common(pos, config, son, fire_particle, smoke);
}

void ActionHandler::ExecActions()
{
  assert(mutex!=NULL);
  while (queue.size() != 0)
  {
    SDL_LockMutex(mutex);
    Action *action = queue.front();
    queue.pop_front();
    SDL_UnlockMutex(mutex);
    Exec (action);
    delete action;
  }
}

void ActionHandler::NewAction(Action* a, bool repeat_to_network)
{
  assert(mutex!=NULL);
  SDL_LockMutex(mutex);
  //  MSG_DEBUG("action.handler","New action : %s",a.out());
  //  std::cout << "New action " << a ;
  queue.push_back(a);
  //  std::cout << "  queue_size " << queue.size() << std::endl;
  SDL_UnlockMutex(mutex);
  if (repeat_to_network) network.SendAction(a);
}

void ActionHandler::Register (Action_t action, 
		                      const std::string &name,callback_t fct)
{
  handler[action] = fct;
  action_name[action] = name;
}

void ActionHandler::Exec(Action *a)
{
// #ifdef DBG_ACT
  // std::cout << "Exec action " << *a << std::endl;
// #endif

  handler_it it=handler.find(a->GetType());
  assert(it != handler.end());
  (*it->second) (a);
}

std::string ActionHandler::GetActionName (Action_t action)
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
  Register (ACTION_WALK, "walk", &Action_Walk);
  Register (ACTION_MOVE_LEFT, "move_left", &Action_MoveLeft);
  Register (ACTION_MOVE_RIGHT, "move_right", &Action_MoveRight);
  Register (ACTION_UP, "up", &Action_Up);
  Register (ACTION_DOWN, "down", &Action_Down);
  Register (ACTION_JUMP, "jump", &Action_Jump);
  Register (ACTION_HIGH_JUMP, "super_jump", &Action_HighJump);
  Register (ACTION_SHOOT, "shoot", &Action_Shoot);
  Register (ACTION_CHANGE_WEAPON, "change_weapon", &Action_ChangeWeapon);
  Register (ACTION_WIND, "wind", &Action_Wind);
  Register (ACTION_CHANGE_CHARACTER, "change_character", &Action_ChangeCharacter);
  Register (ACTION_SET_GAME_MODE, "set_game_mode", &Action_SetGameMode);
  Register (ACTION_SET_MAP, "set_map", &Action_SetMap);
  Register (ACTION_CLEAR_TEAMS, "clear_teams", &Action_ClearTeams);
  Register (ACTION_NEW_TEAM, "new_team", &Action_NewTeam);
  Register (ACTION_DEL_TEAM, "del_team", &Action_DelTeam);
  Register (ACTION_CHANGE_TEAM, "change_team", &Action_ChangeTeam);
  Register (ACTION_MOVE_CHARACTER, "move_character", &Action_MoveCharacter);
  Register (ACTION_SET_CHARACTER_SPEED, "set_character_speed", &Action_SetCharacterSpeed);
  Register (ACTION_SET_MOVEMENT, "set_movement", &Action_SetMovement);
  Register (ACTION_SET_CLOTHE, "set_clothe", &Action_SetClothe);
  Register (ACTION_SET_FRAME, "set_frame", &Action_SetFrame);
  Register (ACTION_SET_CHARACTER_DIRECTION, "set_character_direction", &Action_SetCharacterDirection);
  Register (ACTION_CHANGE_STATE, "change_state", &Action_ChangeState);
  Register (ACTION_ASK_VERSION, "ask_version", &Action_AskVersion);
  Register (ACTION_SEND_VERSION, "send_version", &Action_SendVersion);
  Register (ACTION_SEND_RANDOM, "send_random", &Action_SendRandom);
  Register (ACTION_SYNC_BEGIN, "sync_begin", &Action_SyncBegin);
  Register (ACTION_SYNC_END, "sync_end", &Action_SyncEnd);
  Register (ACTION_EXPLOSION, "explosion", &Action_Explosion);
  SDL_UnlockMutex(mutex);
}
