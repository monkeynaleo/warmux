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
#include "../tool/vector2.h"
#include "../weapon/launcher.h"
#include "../weapon/supertux.h"
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
  ActiveTeam().SetWeapon((Weapon_type)a->PopInt());
}

void Action_ChangeCharacter (Action *a)
{
  ActiveTeam().NextCharacter();
}

void Action_Shoot (Action *a)
{
  double angle = a->PopDouble();
  int force = a->PopInt();
  ActiveTeam().AccessWeapon().Shoot(angle, force);
}

void Action_Wind (Action *a)
{
  wind.SetVal (a->PopInt());
}

Action* BuildActionSendCharacterPhysics(int team_no, int char_no)
{
  Action* a = new Action(ACTION_SET_CHARACTER_PHYSICS);
  Character* c = teams_list.FindPlayingByIndex(team_no)->FindByIndex(char_no);
  a->Push(team_no);
  a->Push(char_no);
  a->Push(c->GetPhysX());
  a->Push(c->GetPhysY());
  Point2d speed;
  c->GetSpeedXY(speed);
  a->Push(speed.x);
  a->Push(speed.y);
  return a;
}

void Action_SetCharacterPhysics (Action *a)
{
  int team_no, char_no;
  double x, y, s_x, s_y;

  team_no = a->PopInt();
  char_no = a->PopInt();
  Character* c = teams_list.FindPlayingByIndex(team_no)->FindByIndex(char_no);
  assert(c != NULL);

  x = a->PopDouble();
  y = a->PopDouble();
  s_x = a->PopDouble();
  s_y = a->PopDouble();
  c->SetPhysXY(x, y);
  c->SetSpeedXY(Point2d(s_x, s_y));
}

void Action_SetFrame (Action *a)
{
  //Set the frame of the walking skin, to get the position of the hand synced
  if (!ActiveTeam().is_local || network.state != Network::NETWORK_PLAYING)
  {
    ActiveTeam().ActiveCharacter().body->SetFrame((uint)a->PopInt());
  }
}

void Action_SetClothe (Action *a)
{
  if (!ActiveTeam().is_local || network.state != Network::NETWORK_PLAYING)
  {
    ActiveTeam().ActiveCharacter().SetClothe(a->PopString());
  }
}

void Action_SetMovement (Action *a)
{
  if (!ActiveTeam().is_local || network.state != Network::NETWORK_PLAYING)
  {
    ActiveTeam().ActiveCharacter().SetMovement(a->PopString());
  }
}

void Action_SetCharacterDirection (Action *a)
{
  ActiveCharacter().SetDirection (a->PopInt());
}

void Action_SetMap (Action *a)
{
  if (!network.IsClient()) return;
  lst_terrain.ChangeTerrainNom (a->PopString());
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
  GameMode::GetInstance()->Load (a->PopString());
}

void Action_NewTeam (Action *a)
{
  teams_list.AddTeam (a->PopString());
}

void Action_DelTeam (Action *a)
{
  teams_list.DelTeam (a->PopString());
}

void Action_ChangeTeam (Action *a)
{
  teams_list.SetActive (a->PopString());
  ActiveTeam().PrepareTurn();
  assert (!ActiveCharacter().IsDead());
}

void Action_AskVersion (Action *a)
{
  if (!network.IsClient()) return;
  ActionHandler::GetInstance()->NewAction(new Action(ACTION_SEND_VERSION, Constants::VERSION));
}

void Action_SendVersion (Action *a)
{
  if (!network.IsServer()) return;
  std::string version= a->PopString();
  if (version != Constants::VERSION)
  {
    Error(Format(_("Wormux versions are differents : client=%s, server=%s."),
    version.c_str(), Constants::VERSION.c_str()));
  }
}

void Action_SendRandom (Action *a)
{
  if (!network.IsClient()) return;
  randomSync.AddToTable(a->PopDouble());
}

void Action_SupertuxState (Action *a)
{
  assert(ActiveTeam().GetWeaponType() == WEAPON_SUPERTUX);
  WeaponLauncher* launcher = static_cast<WeaponLauncher*>(&(ActiveTeam().AccessWeapon()));
  SuperTux* tux = static_cast<SuperTux*>(launcher->GetProjectile());

  double x, y;

  tux->angle = a->PopDouble();
  x = a->PopDouble();
  y = a->PopDouble();
  tux->SetPhysXY(x, y);
  tux->SetSpeedXY(Point2d(0,0));
}

void Action_SyncBegin (Action *a)
{
  assert(!network.sync_lock);
  network.sync_lock = true;
}

void Action_SyncEnd (Action *a)
{
  assert(network.sync_lock);
  network.sync_lock = false;
}

void Action_Explosion (Action *a)
{
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

void Action_SetTarget (Action *a)
{
  MSG_DEBUG("action.handler", "Set target by clicking");

  Point2i target;
  target.x = a->PopInt();
  target.y = a->PopInt();

  ActiveTeam().AccessWeapon().ChooseTarget (target);
}

void Action_SetTimeout (Action *a)
{
  WeaponLauncher* launcher = dynamic_cast<WeaponLauncher*>(&(ActiveTeam().AccessWeapon()));
  assert(launcher != NULL);
  launcher->GetProjectile()->m_timeout_modifier = a->PopInt();
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
  MSG_DEBUG("action_handler", "Executing action %s",GetActionName(a->GetType()).c_str());
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
  Register (ACTION_SET_CHARACTER_PHYSICS, "set_character_physics", &Action_SetCharacterPhysics);
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
  Register (ACTION_SET_TARGET, "set_target", &Action_SetTarget);
  Register (ACTION_SUPERTUX_STATE, "supertux_state", &Action_SupertuxState);
  Register (ACTION_SET_TIMEOUT, "set_timeout", &Action_SetTimeout);
  SDL_UnlockMutex(mutex);
}
