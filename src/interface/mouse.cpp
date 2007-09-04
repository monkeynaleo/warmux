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
 * Mouse management
 *****************************************************************************/

#include "interface/mouse.h"

#include "interface/cursor.h"
#include "interface/interface.h"
#include "character/character.h"
#include "game/config.h"
#include "game/game_mode.h"
#include "game/game.h"
#include "graphic/video.h"
#include "include/app.h"
#include "include/action_handler.h"
#include "map/camera.h"
#include "map/map.h"
#include "team/macro.h"
#include "team/team.h"
#include "tool/point.h"
#include "tool/resource_manager.h"
#include "weapon/weapon.h"

Mouse * Mouse::singleton = NULL;

Mouse * Mouse::GetInstance() {
  if (singleton == NULL) {
    singleton = new Mouse();
  }
  return singleton;
}

Mouse::Mouse(){
  visible = MOUSE_VISIBLE;
  // Load the different pointers
  Profile *res = resource_manager.LoadXMLProfile("graphism.xml", false);
  pointer_select = resource_manager.LoadImage(res, "mouse/pointer_select");
  pointer_move = resource_manager.LoadImage(res, "mouse/pointer_move");
  pointer_arrow_up = resource_manager.LoadImage(res, "mouse/pointer_arrow_up");
  pointer_arrow_up_right = resource_manager.LoadImage(res, "mouse/pointer_arrow_up_right");
  pointer_arrow_up_left = resource_manager.LoadImage(res, "mouse/pointer_arrow_up_left");
  pointer_arrow_down = resource_manager.LoadImage(res, "mouse/pointer_arrow_down");
  pointer_arrow_down_right = resource_manager.LoadImage(res, "mouse/pointer_arrow_down_right");
  pointer_arrow_down_left = resource_manager.LoadImage(res, "mouse/pointer_arrow_down_left");
  pointer_arrow_right = resource_manager.LoadImage(res, "mouse/pointer_arrow_right");
  pointer_arrow_left = resource_manager.LoadImage(res, "mouse/pointer_arrow_left");
  pointer_aim = resource_manager.LoadImage(res, "mouse/pointer_aim");
  pointer_fire_right = resource_manager.LoadImage(res, "mouse/pointer_fire_right");
  pointer_fire_left = resource_manager.LoadImage(res, "mouse/pointer_fire_left");
  current_pointer = POINTER_STANDARD;
  resource_manager.UnLoadXMLProfile(res);
}

void Mouse::ActionLeftClic(bool) const
{
  const Point2i pos_monde = GetWorldPosition();

  // Action in weapon menu ?
  if( Interface::GetInstance()->weapons_menu.ActionClic( GetPosition() ) )
    return;

  //Change character by mouse click only if the choosen weapon allows it
  if( GameMode::GetInstance()->AllowCharacterSelection() &&
      ActiveTeam().GetWeapon().mouse_character_selection) {

    // Choose a character of our own team
    bool character_found=false;
    Team::iterator it=ActiveTeam().begin(),
                         fin=ActiveTeam().end();

    for( ; it != fin; ++it) {
      if( &(*it) != &ActiveCharacter()
        && !it -> IsDead()
        && it->GetRect().Contains( pos_monde ) ){

        character_found = true;
        break;
      }
    }

    if (character_found) {
      Action * next_character = new Action(Action::ACTION_PLAYER_NEXT_CHARACTER);
      next_character->StoreActiveCharacter();

      while ( &(*it) != &ActiveCharacter() )
        ActiveTeam().NextCharacter ();

      next_character->StoreActiveCharacter();
      ActionHandler::GetInstance()->NewAction(next_character);

      return;
    }

    if( ActiveCharacter().GetRect().Contains( pos_monde ) ){
      CharacterCursor::GetInstance()->FollowActiveCharacter();
      return;
    }
  }

  // Choosing target for a weapon, many posibilities :
  // - Do nothing
  // - Choose a target but don't fire
  // - Choose a target and fire it !
  Action* a = new Action(Action::ACTION_WEAPON_SET_TARGET);
  a->Push(GetWorldPosition());
  ActionHandler::GetInstance()->NewAction (a);
}


void Mouse::ActionRightClic(bool) const
{
  Interface::GetInstance()->weapons_menu.SwitchDisplay();
}

void Mouse::ActionWheelUp(bool shift) const
{
  ActiveTeam().AccessWeapon().HandleMouseWheelUp(shift);
}

void Mouse::ActionWheelDown(bool shift) const
{
  ActiveTeam().AccessWeapon().HandleMouseWheelDown(shift);
}

bool Mouse::HandleClic (const SDL_Event& event) const
{
  if ( event.type != SDL_MOUSEBUTTONDOWN &&
       event.type != SDL_MOUSEBUTTONUP ) {
    return false ;
  }

  if (Game::GetInstance()->ReadState() != Game::PLAYING)
    return true;

  if(!ActiveTeam().IsLocal())
    return true;

  if( event.type == SDL_MOUSEBUTTONDOWN ){
    bool shift = !!(SDL_GetModState() & KMOD_SHIFT);
    switch (event.button.button) {
    case SDL_BUTTON_RIGHT:
      ActionRightClic(shift);
      break;
    case SDL_BUTTON_LEFT:
      ActionLeftClic(shift);
      break;
    case SDL_BUTTON_WHEELDOWN:
      ActionWheelDown(shift);
      break;
    case SDL_BUTTON_WHEELUP:
      ActionWheelUp(shift);
      break;
    default:
      break;
    }
  }
  return true;
}

void Mouse::ChoixVerPointe() const
{
  if (Game::GetInstance()->ReadState() != Game::PLAYING)
    return;

  const Point2i pos_monde = GetWorldPosition();

  // Which character is pointed by the mouse ? (appart from the active one)
  Interface::GetInstance()->character_under_cursor = NULL;
  FOR_ALL_LIVING_CHARACTERS(equipe,ver){
    if ((&(*ver) != &ActiveCharacter())
       && ver->GetRect().Contains(pos_monde) ){
      Interface::GetInstance()->character_under_cursor = &(*ver);
    }
  }

  // No character is pointed... what about the active one ?
  if ((Interface::GetInstance()->character_under_cursor == NULL)
      && ActiveCharacter().GetRect().Contains( pos_monde)){
      Interface::GetInstance()->character_under_cursor = &ActiveCharacter();
  }

}

void Mouse::Refresh()
{
  static Point2i lastpos(0,0);
  /* FIXME the 200 is hardcoded because I cannot find where the main loop
   * refresh rate is set... */
  static int counter = 200;
  ChoixVerPointe();

  Point2i pos = GetPosition();

  if (lastpos != pos)
    {
      Show();
      lastpos = pos;
    }
  else if (visible == MOUSE_VISIBLE)
    {
      /* The mouse is hidden after a while when not moving */
      --counter;
      if (counter <= 0)
        {
          counter = 200;
          Hide();
        }
    }
}

Point2i Mouse::GetPosition() const
{
  int x, y;

  SDL_GetMouseState( &x, &y);
  return Point2i(x, y);
}

Point2i Mouse::GetWorldPosition() const
{
  return GetPosition() + Camera::GetInstance()->GetPosition();
}

// set the new pointer type and return the previous one
Mouse::pointer_t Mouse::SetPointer(pointer_t pointer)
{
  if (Config::GetInstance()->GetDefaultMouseCursor()) {
    Show(); // be sure cursor is visible
    return current_pointer;
  }

  if (current_pointer == pointer) return current_pointer;

  if (pointer == POINTER_STANDARD)
    SDL_ShowCursor(true);
  else
    SDL_ShowCursor(false);

  pointer_t old_pointer = current_pointer;
  current_pointer = pointer;

  return old_pointer;
}

const Surface& Mouse::GetSurfaceFromPointer(pointer_t pointer) const
{
  switch (pointer) {
  case POINTER_STANDARD:
    return pointer_select;
  case POINTER_SELECT:
    return pointer_select;
  case POINTER_MOVE:
    return pointer_move;
  case POINTER_ARROW_UP:
    return pointer_arrow_up;
  case POINTER_ARROW_UP_RIGHT:
    return pointer_arrow_up_right;
  case POINTER_ARROW_UP_LEFT:
    return pointer_arrow_up_left;
  case POINTER_ARROW_DOWN:
    return pointer_arrow_down;
  case POINTER_ARROW_DOWN_RIGHT:
    return pointer_arrow_down_right;
  case POINTER_ARROW_DOWN_LEFT:
    return pointer_arrow_down_left;
  case POINTER_ARROW_RIGHT:
    return pointer_arrow_right;
  case POINTER_ARROW_LEFT:
    return pointer_arrow_left;
  case POINTER_AIM:
    return pointer_aim;
  case POINTER_FIRE:
    if (ActiveCharacter().GetDirection() == DIRECTION_RIGHT)
      return pointer_fire_left; // left hand to shoot on the right
    else
      return pointer_fire_right;
  case POINTER_FIRE_RIGHT:
    return pointer_fire_left;
  case POINTER_FIRE_LEFT:
    return pointer_fire_right;
  }

  // to make g++ happy
  return pointer_select;
}

void Mouse::Draw() const
{
  if (visible != MOUSE_VISIBLE)
    return;

  if (current_pointer == POINTER_STANDARD)
    return; // use standard SDL cursor

  const Surface& cursor = GetSurfaceFromPointer(current_pointer);
  AppWormux::GetInstance()->video->window.Blit( cursor, GetPosition() );
  world.ToRedrawOnScreen(Rectanglei(GetPosition().x, GetPosition().y ,
                         cursor.GetWidth(), cursor.GetHeight()));
}

void Mouse::Show()
{
  visible = MOUSE_VISIBLE;

  if (Config::GetInstance()->GetDefaultMouseCursor()) {
    SDL_ShowCursor(true); // be sure cursor is visible
  }

  MSG_DEBUG("mouse", "%d, %d", GetPosition().GetX(),
            GetPosition().GetY() );
}

void Mouse::Hide()
{
  visible = MOUSE_HIDDEN;
  SDL_ShowCursor(false); // be sure cursor is invisible

  MSG_DEBUG("mouse", "%d, %d", GetPosition().GetX(),
            GetPosition().GetY() );
}

