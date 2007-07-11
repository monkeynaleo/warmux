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
 * Virtual class to handle man/machine interaction
 *****************************************************************************/

#include "man_machine_interface.h"
#include "cursor.h"
#include "interface.h"
#include "character/character.h"
#include "ai/ai_engine_stupid.h"
#include "game/game.h"
#include "game/game_loop.h"
#include "game/game_mode.h"
#include "include/app.h"
#include "include/action_handler.h"
#include "map/camera.h"
#include "network/network.h"
#include "object/objbox.h"
#include "team/teams_list.h"
#include "team/team.h"
#include "weapon/weapons_list.h"

#define SCROLL_KEYBOARD 20 // pixel

ManMachineInterface::ManMachineInterface()
{
  SetDefaultConfig();
}

ManMachineInterface::~ManMachineInterface()
{
}

void ManMachineInterface::SetDefaultConfig()
{
}

void ManMachineInterface::Reset()
{
  for (int i = 0; i != 256; i++)
    PressedKeys[i] = false ;
}

void ManMachineInterface::RegisterEvent(uint8 event_type)
{
  registred_event.push_back(event_type);
}

bool ManMachineInterface::IsRegistredEvent(uint8 event_type)
{
  std::list<uint8>::iterator it;
  for(it = registred_event.begin(); it != registred_event.end(); it ++) {
    if(event_type == (*it))
      return true;
  }
  return false;
}

void ManMachineInterface::SetKeyAction(int key, Key_t at)
{
  layout[key] = at;
}

// Get the key associated to an action.
int ManMachineInterface::GetKeyAssociatedToAction(Key_t at)
{
  std::map<int, Key_t>::iterator it;
  for (it= layout.begin(); it != layout.end(); it++) {
    if (it->second == at) {
      return it->first;
    }
  }
  return 0;
}

bool ManMachineInterface::MoveCamera(const Key_t &key)
{
  bool r = true;

  switch(key) {
  case KEY_MOVE_RIGHT:
    camera.SetXY(Point2i(SCROLL_KEYBOARD, 0));
    break;
  case KEY_MOVE_LEFT:
    camera.SetXY(Point2i(-SCROLL_KEYBOARD, 0));
    break;
  case KEY_UP:
    camera.SetXY(Point2i(0, -SCROLL_KEYBOARD));
    break;
  case KEY_DOWN:
    camera.SetXY(Point2i(0, SCROLL_KEYBOARD));
    break;
  default:
    r = false;
    break;
  }

  if (r)
    camera.SetAutoCrop(false);

  return r;
}

// Handle a pressed key
void ManMachineInterface::HandleKeyPressed(const Key_t &key)
{
  SDLMod mod = SDL_GetModState();
  if (mod & KMOD_CTRL) {
    if (MoveCamera(key)) {
      PressedKeys[key] = true;
      return;
    }
  }

  // Managing keys related to character moves
  // Available only when local
  if (!ActiveTeam().IsLocal()) return;
  if (GameLoop::GetInstance()->ReadState() == GameLoop::END_TURN) return;
  if (ActiveCharacter().IsDead()) return;

  if (GameLoop::GetInstance()->ReadState() == GameLoop::HAS_PLAYED) {
    switch (key) {

    case KEY_MOVE_RIGHT:
      ActiveCharacter().HandleKeyPressed_MoveRight();
      break;
    case KEY_MOVE_LEFT:
      ActiveCharacter().HandleKeyPressed_MoveLeft();
      break;
    case KEY_UP:
      ActiveCharacter().HandleKeyPressed_Up();
      break;
    case KEY_DOWN:
      ActiveCharacter().HandleKeyPressed_Down();
      break;
    case KEY_JUMP:
      ActiveCharacter().HandleKeyPressed_Jump();
      break;
    case KEY_HIGH_JUMP:
      ActiveCharacter().HandleKeyPressed_HighJump();
      break;
    case KEY_BACK_JUMP:
      ActiveCharacter().HandleKeyPressed_BackJump();
      break;
    case KEY_SHOOT:
      // Shoot key is not accepted in HAS_PLAYED state
      return;
    default:
      // key not supported
      return;
    }
  } else if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING) {

    // Movements are managed by weapons because sometimes it overrides the keys
    switch (key) {

    case KEY_MOVE_RIGHT:
      ActiveTeam().AccessWeapon().HandleKeyPressed_MoveRight();
      break;
    case KEY_MOVE_LEFT:
      ActiveTeam().AccessWeapon().HandleKeyPressed_MoveLeft();
      break;
    case KEY_UP:
      ActiveTeam().AccessWeapon().HandleKeyPressed_Up();
      break;
    case KEY_DOWN:
      ActiveTeam().AccessWeapon().HandleKeyPressed_Down();
      break;
    case KEY_JUMP:
      ActiveTeam().AccessWeapon().HandleKeyPressed_Jump();
      break;
    case KEY_HIGH_JUMP:
      ActiveTeam().AccessWeapon().HandleKeyPressed_HighJump();
      break;
    case KEY_BACK_JUMP:
      ActiveTeam().AccessWeapon().HandleKeyPressed_BackJump();
      break;
    case KEY_SHOOT:
      if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING) {
	ActiveTeam().AccessWeapon().HandleKeyPressed_Shoot();
	break;
      }
    default:
      // key not supported
      return;
    }
  }

  PressedKeys[key] = true ;
}

// Handle a released key
void ManMachineInterface::HandleKeyReleased(const Key_t &key)
{
  PressedKeys[key] = false;
  // Here we manage only actions which are activated on KEY_RELEASED event.

  // hack to interrupt AI
  if (ActiveTeam().IsLocalAI() && key == KEY_SHOOT)
  {
    AIStupidEngine::GetInstance()->ForceEndOfTurn();
  }

  { // Managing keys related to interface (no game interaction)
    // Always available
    switch(key){
      // Managing interface
    case KEY_QUIT:
      Game::GetInstance()->UserWantEndOfGame();
      return;
    case KEY_PAUSE:
      if (!Network::IsConnected())
	Game::GetInstance()->TogglePause();
      return;
    case KEY_FULLSCREEN:
      AppWormux::GetInstance()->video.ToggleFullscreen();
      return;
    case KEY_CHAT:
      if(Network::IsConnected())
	GameLoop::GetInstance()->chatsession.ShowInput();
      return;
    case KEY_CENTER:
      CharacterCursor::GetInstance()->FollowActiveCharacter();
      camera.FollowObject (&ActiveCharacter(), true, true, true);
      return;
    case KEY_TOGGLE_INTERFACE:
      Interface::GetInstance()->EnableDisplay (!Interface::GetInstance()->IsDisplayed());
      return;
    default:
      break;
    }
  }

  // Managing shoot key
  // Drop bonus box or medkit when outside a turn
  // Shoot when in turn
  if (key == KEY_SHOOT) {

    if (GameLoop::GetInstance()->ReadState() == GameLoop::END_TURN && 
	!Network::IsConnected()) {
      ObjBox* current_box = GameLoop::GetInstance()->GetCurrentBox();
      if (current_box != NULL) {
	current_box->DropBox();
      }
    } else if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING &&
	       ActiveTeam().IsLocal() &&
	       !ActiveCharacter().IsDead()) {
      ActiveTeam().AccessWeapon().HandleKeyReleased_Shoot();
    }
    return;
  }

  { // Managing keys related to character moves
    // Available only when local
    if (!ActiveTeam().IsLocal()) return;
    if (ActiveCharacter().IsDead()) return;
    if (GameLoop::GetInstance()->ReadState() == GameLoop::END_TURN) return;

    if (GameLoop::GetInstance()->ReadState() == GameLoop::HAS_PLAYED) {

      switch (key) {
      case KEY_MOVE_RIGHT:
	ActiveCharacter().HandleKeyReleased_MoveRight();
	return;
      case KEY_MOVE_LEFT:
	ActiveCharacter().HandleKeyReleased_MoveLeft();
	return;
      case KEY_UP:
	ActiveCharacter().HandleKeyReleased_Up();
	return;
      case KEY_DOWN:
	ActiveCharacter().HandleKeyReleased_Down();
	return;
      case KEY_JUMP:
	ActiveCharacter().HandleKeyReleased_Jump();
	return;
      case KEY_HIGH_JUMP:
	ActiveCharacter().HandleKeyReleased_HighJump();
	return;
      case KEY_BACK_JUMP:
	ActiveCharacter().HandleKeyReleased_BackJump();
	return;
      case KEY_SHOOT:
      // Shoot key is not accepted in HAS_PLAYED state
	return;
      default:
	// Key not supported
	return;
      }
    } else if  (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING) {

      // Movements are managed by weapons because sometimes it overrides the keys
      switch (key) {

      case KEY_MOVE_RIGHT:
	ActiveTeam().AccessWeapon().HandleKeyReleased_MoveRight();
	return;
      case KEY_MOVE_LEFT:
	ActiveTeam().AccessWeapon().HandleKeyReleased_MoveLeft();
	return;
      case KEY_UP:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Up();
	return;
      case KEY_DOWN:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Down();
	return;
      case KEY_JUMP:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Jump();
	return;
      case KEY_HIGH_JUMP:
	ActiveTeam().AccessWeapon().HandleKeyReleased_HighJump();
	return;
      case KEY_BACK_JUMP:
	ActiveTeam().AccessWeapon().HandleKeyReleased_BackJump();
	return;

	// Shoot key
      case KEY_SHOOT:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Shoot();
	return;

	// Other keys usefull for weapons
      case KEY_WEAPON_1:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num1();
	return;
      case KEY_WEAPON_2:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num2();
	return;
      case KEY_WEAPON_3:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num3();
	return;
      case KEY_WEAPON_4:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num4();
	return;
      case KEY_WEAPON_5:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num5();
	return;
      case KEY_WEAPON_6:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num6();
	return;
      case KEY_WEAPON_7:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num7();
	return;
      case KEY_WEAPON_8:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num8();
	return;
      case KEY_WEAPON_9:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Num9();
	return;
      case KEY_WEAPON_LESS:
	ActiveTeam().AccessWeapon().HandleKeyReleased_Less();
	return;
      case KEY_WEAPON_MORE:
	ActiveTeam().AccessWeapon().HandleKeyReleased_More();
	return;
      default:
	break;
      }
    }
  }

  { // Managing keys related to change of character or weapon

    if (GameLoop::GetInstance()->ReadState() != GameLoop::PLAYING ||
	!ActiveTeam().GetWeapon().CanChangeWeapon())
      return;

    Weapon::category_t weapon_sort = Weapon::INVALID;

    switch(key) {

    case KEY_NEXT_CHARACTER:
      {
	if (GameMode::GetInstance()->AllowCharacterSelection()) {
	  SDLMod mod = SDL_GetModState();
	  if (mod & KMOD_CTRL) {
	    Action * previous_character = new Action(Action::ACTION_PLAYER_PREVIOUS_CHARACTER);
	    previous_character->StoreActiveCharacter();
	    ActiveTeam().PreviousCharacter();
	    previous_character->StoreActiveCharacter();
	    ActionHandler::GetInstance()->NewAction(previous_character);
	  } else {
	    Action * next_character = new Action(Action::ACTION_PLAYER_NEXT_CHARACTER);
	    next_character->StoreActiveCharacter();
	    ActiveTeam().NextCharacter();
	    next_character->StoreActiveCharacter();
	    ActionHandler::GetInstance()->NewAction(next_character);
	  }
	}
      }
      return;

    case KEY_WEAPONS1:
      weapon_sort = Weapon::HEAVY;
      break;
    case KEY_WEAPONS2:
      weapon_sort = Weapon::RIFLE;
      break;
    case KEY_WEAPONS3:
      weapon_sort = Weapon::THROW;
      break;
    case KEY_WEAPONS4:
      weapon_sort = Weapon::SPECIAL;
      break;
    case KEY_WEAPONS5:
      weapon_sort = Weapon::DUEL;
      break;
    case KEY_WEAPONS6:
      weapon_sort = Weapon::MOVE;
      break;
    case KEY_WEAPONS7:
      weapon_sort = Weapon::TOOL;
      break;
    default:
      // Key not supported
      return;
    }

    if ( weapon_sort != Weapon::INVALID ) {
      Weapon::Weapon_type weapon;
      if (WeaponsList::GetInstance()->GetWeaponBySort(weapon_sort, weapon))
        {
          ASSERT (weapon >= Weapon::WEAPON_FIRST && weapon <= Weapon::WEAPON_LAST);
	  ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_PLAYER_CHANGE_WEAPON, weapon));
        }
    }
  }
}

// Refresh keys which are still pressed.
void ManMachineInterface::Refresh()
{
  SDLMod mod = SDL_GetModState();

  //Treat KEY_REFRESH events:
  for (int i = 0; i < 256; i++) {

    if (PressedKeys[i]) {
      Key_t key = static_cast<Key_t>(i);

      if (mod & KMOD_CTRL) {
	if (MoveCamera(key))
	  return;
      }

      // Managing keys related to character moves
      // Available only when local
      if (!ActiveTeam().IsLocal()) return;
      if (ActiveCharacter().IsDead()) return;
      if (GameLoop::GetInstance()->ReadState() == GameLoop::END_TURN) return;

      // Movements are managed by weapons because sometimes it overrides the keys
      switch (key) {

      case KEY_MOVE_RIGHT:
	ActiveTeam().AccessWeapon().HandleKeyRefreshed_MoveRight();
	return;
      case KEY_MOVE_LEFT:
	ActiveTeam().AccessWeapon().HandleKeyRefreshed_MoveLeft();
	return;
      case KEY_UP:
	ActiveTeam().AccessWeapon().HandleKeyRefreshed_Up();
	return;
      case KEY_DOWN:
	ActiveTeam().AccessWeapon().HandleKeyRefreshed_Down();
	return;
      case KEY_JUMP:
	ActiveTeam().AccessWeapon().HandleKeyRefreshed_Jump();
	return;
      case KEY_HIGH_JUMP:
	ActiveTeam().AccessWeapon().HandleKeyRefreshed_HighJump();
	return;
      case KEY_BACK_JUMP:
	ActiveTeam().AccessWeapon().HandleKeyRefreshed_BackJump();
	return;
      case KEY_SHOOT:
	if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING) {
	  ActiveTeam().AccessWeapon().HandleKeyRefreshed_Shoot();
	}
	return;
      default:
	break;
      }
    }
  }
}
