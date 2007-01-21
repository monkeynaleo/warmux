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
 * Keyboard managment.
 *****************************************************************************/

#include "keyboard.h"
#include <sstream>
#include <iostream>
#include "cursor.h"
#include "game_msg.h"
#include "interface.h"
#include "../include/action.h"
#include "../include/app.h"
#include "../game/config.h"
#include "../game/game.h"
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../game/time.h"
#include "../graphic/video.h"
#include "../include/action_handler.h"
#include "../include/constant.h"
#include "../map/camera.h"
#include "../team/macro.h"
#include "../character/move.h"
#include "../tool/i18n.h"
#include "../tool/math_tools.h"
#include "../sound/jukebox.h"
#include "../map/camera.h"
#include "../weapon/weapon.h"
#include "../weapon/weapons_list.h"
#include "../network/network.h"

// Vitesse du definalement au clavier
#define SCROLL_CLAVIER 20 // ms

Keyboard * Keyboard::singleton = NULL;

Keyboard * Keyboard::GetInstance() {
  if (singleton == NULL) {
    singleton = new Keyboard();
  }
  return singleton;
}

Keyboard::Keyboard()
{
  //Disable repeated events when a key is kept down
  SDL_EnableKeyRepeat(0,0);

  SetKeyAction(SDLK_LEFT,      Action::ACTION_MOVE_LEFT);
  SetKeyAction(SDLK_RIGHT,     Action::ACTION_MOVE_RIGHT);
  SetKeyAction(SDLK_UP,        Action::ACTION_UP);
  SetKeyAction(SDLK_DOWN,      Action::ACTION_DOWN);
  SetKeyAction(SDLK_RETURN,    Action::ACTION_JUMP);
  SetKeyAction(SDLK_BACKSPACE, Action::ACTION_HIGH_JUMP);
  SetKeyAction(SDLK_b,         Action::ACTION_BACK_JUMP);
  SetKeyAction(SDLK_SPACE,     Action::ACTION_SHOOT);
  SetKeyAction(SDLK_TAB,       Action::ACTION_NEXT_CHARACTER);
  SetKeyAction(SDLK_ESCAPE,    Action::ACTION_QUIT);
  SetKeyAction(SDLK_p,         Action::ACTION_PAUSE);
  SetKeyAction(SDLK_F10,       Action::ACTION_FULLSCREEN);
  SetKeyAction(SDLK_F9,        Action::ACTION_TOGGLE_INTERFACE);
  SetKeyAction(SDLK_F1,        Action::ACTION_WEAPONS1);
  SetKeyAction(SDLK_F2,        Action::ACTION_WEAPONS2);
  SetKeyAction(SDLK_F3,        Action::ACTION_WEAPONS3);
  SetKeyAction(SDLK_F4,        Action::ACTION_WEAPONS4);
  SetKeyAction(SDLK_F5,        Action::ACTION_WEAPONS5);
  SetKeyAction(SDLK_F6,        Action::ACTION_WEAPONS6);
  SetKeyAction(SDLK_F7,        Action::ACTION_WEAPONS7);
  SetKeyAction(SDLK_F8,        Action::ACTION_WEAPONS8);
  SetKeyAction(SDLK_c,         Action::ACTION_CENTER);
  SetKeyAction(SDLK_1,         Action::ACTION_WEAPON_1);
  SetKeyAction(SDLK_2,         Action::ACTION_WEAPON_2);
  SetKeyAction(SDLK_3,         Action::ACTION_WEAPON_3);
  SetKeyAction(SDLK_4,         Action::ACTION_WEAPON_4);
  SetKeyAction(SDLK_5,         Action::ACTION_WEAPON_5);
  SetKeyAction(SDLK_6,         Action::ACTION_WEAPON_6);
  SetKeyAction(SDLK_7,         Action::ACTION_WEAPON_7);
  SetKeyAction(SDLK_8,         Action::ACTION_WEAPON_8);
  SetKeyAction(SDLK_9,         Action::ACTION_WEAPON_9);
  SetKeyAction(SDLK_PAGEUP,    Action::ACTION_WEAPON_MORE);
  SetKeyAction(SDLK_PAGEDOWN,  Action::ACTION_WEAPON_LESS);
  SetKeyAction(SDLK_s,         Action::ACTION_CHAT);
}

void Keyboard::Reset()
{
  for (int i = Action::ACTION_FIRST; i != Action::ACTION_LAST; i++)
    PressedKeys[i] = false ;
}

void Keyboard::SetKeyAction(int key, Action::Action_t at)
{
  layout[key] = at;
}

// Get the key associated to an action.
int Keyboard::GetKeyAssociatedToAction(Action::Action_t at)
{
  std::map<int, Action::Action_t>::iterator it;
  for (it= layout.begin(); it != layout.end(); it++) {
    if (it->second == at) {
      return it->first;
    }
  }
  return 0;
}


void Keyboard::HandleKeyEvent(const SDL_Event& event)
{
  // Not a keyboard event
  if ( event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
    return;
  }

  //Handle input text for Chat session in Network game
  //While player writes, it cannot control the game.
  if(GameLoop::GetInstance()->chatsession.CheckInput()){
    GameLoop::GetInstance()->chatsession.HandleKey(event);
    return;
  }

  Key_Event_t event_type;
  switch( event.type)
    {
    case SDL_KEYDOWN:
      event_type = KEY_PRESSED;
      break;
    case SDL_KEYUP:
      event_type = KEY_RELEASED;
      break;
    default:
      return;
    }

  std::map<int, Action::Action_t>::iterator it = layout.find(event.key.keysym.sym);

  if ( it == layout.end() )
    return;

  Action::Action_t action = it->second;

  //We can perform the next actions, only if the player is played localy:
  if(ActiveTeam().IsLocal())
  {
    if(event_type == KEY_PRESSED && HandleKeyPressed(action))
      return;

    if(event_type == KEY_RELEASED && HandleKeyReleased(action))
      return;

    if ((ActiveTeam().GetWeapon().override_keys &&
        ActiveTeam().GetWeapon().IsActive()) || ActiveTeam().GetWeapon().force_override_keys)
      {
        ActiveTeam().AccessWeapon().HandleKeyEvent(action, event_type);
        return ;
      }
    ActiveCharacter().HandleKeyEvent(action, event_type);
  }
  else
  {
    //Current player is on the network
    if(event_type == KEY_RELEASED)
      HandleKeyReleased(action);
  }
}

// Handle a pressed key
bool Keyboard::HandleKeyPressed (const Action::Action_t &action)
{
  PressedKeys[action] = true ;

  if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING &&
      ActiveTeam().GetWeapon().CanChangeWeapon())
    {
      int weapon_sort = -1;

      switch(action) {
        case Action::ACTION_WEAPONS1:
          weapon_sort = 1;
          break;

        case Action::ACTION_WEAPONS2:
          weapon_sort = 2;
          break;

        case Action::ACTION_WEAPONS3:
          weapon_sort = 3;
          break;

        case Action::ACTION_WEAPONS4:
          weapon_sort = 4;
          break;

        case Action::ACTION_WEAPONS5:
          weapon_sort = 5;
          break;

        case Action::ACTION_WEAPONS6:
          weapon_sort = 6;
          break;

        case Action::ACTION_WEAPONS7:
          weapon_sort = 7;
          break;

        case Action::ACTION_WEAPONS8:
          weapon_sort = 8;
          break;

        case Action::ACTION_NEXT_CHARACTER:
          if (GameMode::GetInstance()->AllowCharacterSelection()) {
            Action * next_character = new Action(Action::ACTION_NEXT_CHARACTER);
            next_character->StoreActiveCharacter();
            ActiveTeam().NextCharacter();
            next_character->StoreActiveCharacter();
            ActionHandler::GetInstance()->NewAction(next_character);
          }
          return true;

        default:
          break ;
      }

      if ( weapon_sort > 0 )
        {
          Weapon::Weapon_type weapon;
          if (WeaponsList::GetInstance()->GetWeaponBySort(weapon_sort, weapon))
            ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_CHANGE_WEAPON, weapon));

          return true;
        }
    }
  return false;
}

// Handle a released key
bool Keyboard::HandleKeyReleased (const Action::Action_t &action)
{
  PressedKeys[action] = false ;

  // We manage here only actions which are active on KEY_RELEASED event.
  Interface * interface = Interface::GetInstance();
  BonusBox * current_bbox;
  Medkit * current_mbox;

  switch(action) // Convert to int to avoid a warning
  {
    case Action::ACTION_QUIT:
      Game::GetInstance()->SetEndOfGameStatus( true );
      return true;
    case Action::ACTION_PAUSE:
      ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_PAUSE));
      return true;
    case Action::ACTION_SHOOT:
      current_bbox = GameLoop::GetInstance()->GetCurrentBonusBox();
      current_mbox = GameLoop::GetInstance()->GetCurrentMedkit();
      if (current_bbox != NULL) {
        current_bbox->DropBonusBox();
	std::cout << "MFE Drop Bonus Box !" << std::endl;
	return true;
      }
      if (current_mbox != NULL) {
        current_mbox->DropMedkit();
	std::cout << "MFE Drop Medkit !" << std::endl;
	return true;
      }
      break;
    case Action::ACTION_FULLSCREEN:
      AppWormux::GetInstance()->video.ToggleFullscreen();
      return true;
    case Action::ACTION_CHAT:
      if(network.IsConnected())
        GameLoop::GetInstance()->chatsession.ShowInput();
      return true;
    case Action::ACTION_CENTER:
      CharacterCursor::GetInstance()->FollowActiveCharacter();
      camera.FollowObject (&ActiveCharacter(), true, true, true);
      return true;
    case Action::ACTION_TOGGLE_INTERFACE:
      interface->EnableDisplay (!interface->IsDisplayed());
      return true;
    default:
      break;
  }
  return false;
}

// Refresh keys which are still pressed.
void Keyboard::Refresh()
{
  //Treat KEY_REFRESH events:
  for (int i = Action::ACTION_FIRST; i < Action::ACTION_LAST; i++)
    if(PressedKeys[i])
      {
        if (ActiveTeam().GetWeapon().override_keys &&
            ActiveTeam().GetWeapon().IsActive())
          {
            ActiveTeam().AccessWeapon().HandleKeyEvent(static_cast<Action::Action_t>(i), KEY_REFRESH);
          }
        else
          {
            ActiveCharacter().HandleKeyEvent(static_cast<Action::Action_t>(i),KEY_REFRESH);
          }
      }
}
