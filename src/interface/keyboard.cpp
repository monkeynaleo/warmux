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
 * Keyboard managment. Use ClanLIB event.
 *****************************************************************************/

#include "keyboard.h"
//-----------------------------------------------------------------------------
#include <sstream>
#include "../team/macro.h"
#include "../game/time.h"

#include "../include/action_handler.h"
#include "../weapon/weapon.h"
#include "../weapon/weapons_list.h"

#include "game_msg.h"
#include "../map/camera.h"
#include "interface.h"
#include "../team/move.h"
#include "../tool/i18n.h"
#include "../tool/math_tools.h"
#include "../game/game.h"
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../sound/jukebox.h"
#include "../game/config.h"
#include "../graphic/video.h"
#include "cursor.h"
#include "../include/constant.h"
#include <iostream>
using namespace Wormux;
//-----------------------------------------------------------------------------

// Active le mode tricheur ?
#ifdef DEBUG
#  define MODE_TRICHEUR
//#  define USE_HAND_POSITION_MODIFIER
#endif

// Vitesse du definalement au clavier
#define SCROLL_CLAVIER 20 // ms

//-----------------------------------------------------------------------------
Clavier clavier;
//-----------------------------------------------------------------------------

Clavier::Clavier()
{
#ifdef CL
   pilote_installe = false;
#endif
}

//-----------------------------------------------------------------------------


void Clavier::Reset()
{
  //Disable repeated events when a key is kept down
  SDL_EnableKeyRepeat(0,0);
  for (uint i = 0; i < ACTION_MAX; i++)
    PressedKeys[i] = false ;
}

//-----------------------------------------------------------------------------

#ifdef CL
void Clavier::DesinstallePilote()
{
  assert (pilote_installe);
  CL_Keyboard::sig_key_up().disconnect(slot_down);
  CL_Keyboard::sig_key_down().disconnect(slot_up);
  pilote_installe = false;
}

//-----------------------------------------------------------------------------

void Clavier::InstallePilote()
{
  int i ;

  assert (!pilote_installe);
  slot_down = CL_Keyboard::sig_key_down().connect(this, &Clavier::HandleKeyPressed);
  slot_up = CL_Keyboard::sig_key_up().connect(this, &Clavier::HandleKeyReleased);
  pilote_installe = true;

  for (i = 0; i < ACTION_MAX; i++)
    PressedKeys[i] = false ;
}
#endif

//-----------------------------------------------------------------------------

void Clavier::SetKeyAction(int key, Action_t at)
{
  layout[key] = at;
}

//-----------------------------------------------------------------------------

#ifdef CL
void Clavier::HandleKeyEvent(int key, int event_type)
{
  std::map<int, Action_t>::iterator it = layout.find(key);

  if ( it == layout.end() )
    return;

  Action_t action = it->second;

  //We can perform the next actions, only if the player is played localy:
  if(!ActiveTeam().is_local)
    return;

  if (ActiveTeam().GetWeapon().override_keys &&
      ActiveTeam().GetWeapon().IsActive())
    {
      ActiveTeam().AccessWeapon().HandleKeyEvent((int)action, event_type);
      return ;
    }

  if(action <= ACTION_CHANGE_CHARACTER)
    {
      switch (action) {
//         case ACTION_ADD:
// 	  if (lance_grenade.time < 15)
// 	    lance_grenade.time ++;
// 	  break ;
	  
//         case ACTION_SUBSTRACT:
// 	  if (lance_grenade.time > 1)
// 	    lance_grenade.time --;
// 	  break ;
        default:
	  break ;
      }
    }

  ActiveCharacter().HandleKeyEvent((int)action, event_type);
}
#else // CL is defined
void Clavier::HandleKeyEvent( const SDL_Event *event)
{
  std::map<int, Action_t>::iterator it = layout.find(event->key.keysym.sym);

  if ( it == layout.end() )
    return;

  Action_t action = it->second;

  //We can perform the next actions, only if the player is played localy:
  //if(!ActiveTeam().is_local)
  //  return;

  if(action <= ACTION_CHANGE_CHARACTER)
    {
      switch (action) {
//         case ACTION_ADD:
// 	  if (lance_grenade.time < 15)
// 	    lance_grenade.time ++;
// 	  break ;
	  
//         case ACTION_SUBSTRACT:
// 	  if (lance_grenade.time > 1)
// 	    lance_grenade.time --;
// 	  break ;
        default:
	  break ;
      }
    }

   int event_type=0;
   switch( event->type)
     {
      case SDL_KEYDOWN: event_type = KEY_PRESSED;break;
      case SDL_KEYUP: event_type = KEY_RELEASED;break;
     }
  if(event_type==KEY_PRESSED)
    HandleKeyPressed(action);
  if(event_type==KEY_RELEASED)
    HandleKeyReleased(action);

  if (ActiveTeam().GetWeapon().override_keys &&
      ActiveTeam().GetWeapon().IsActive())
    {
      ActiveTeam().AccessWeapon().HandleKeyEvent((int)action, event_type);
      return ;
    }
   
  ActiveCharacter().HandleKeyEvent( action, event_type);
}

#endif // CL not defined
//-----------------------------------------------------------------------------

// Handle a pressed key
#ifdef CL
void Clavier::HandleKeyPressed (const CL_InputEvent &key)
#else
void Clavier::HandleKeyPressed (const Action_t &action)
#endif
{
#ifdef CL
  std::map<int, Action_t>::iterator it = layout.find(key.id);
  
  if ( it == layout.end() )
    return;
  
  Action_t action = it->second;
#endif

#ifdef CL
  if (PressedKeys[action])
    {
      // The key is already pressed... It's a refresh.
      HandleKeyEvent(action, KEY_REFRESH);
      return ;
    }
  else
    PressedKeys[action] = true ;
#endif
  PressedKeys[action] = true ;

  //We can perform the next actions, only if the player is played localy:
  if(!ActiveTeam().is_local)
    return;

  if (game_loop.ReadState() == gamePLAYING &&
      ActiveTeam().GetWeapon().CanChangeWeapon())
    {
      int weapon_sort = -1;

      switch(action) {
        case ACTION_WEAPONS1:
	  weapon_sort = 1;
	  break;

        case ACTION_WEAPONS2:
	  weapon_sort = 2;
	  break;

        case ACTION_WEAPONS3:
	  weapon_sort = 3;
	  break;

        case ACTION_WEAPONS4:
	  weapon_sort = 4;
	  break;

        case ACTION_WEAPONS5:
	  weapon_sort = 5;
	  break;

        case ACTION_WEAPONS6:
	  weapon_sort = 6;
	  break;

        case ACTION_WEAPONS7:
	  weapon_sort = 7;
	  break;

        case ACTION_WEAPONS8:
	  weapon_sort = 8;
	  break;

        case ACTION_CHANGE_CHARACTER:
	  if (game_mode.AllowCharacterSelection())
	    action_handler.NewAction(ActionInt(action,
					ActiveTeam().NextCharacterIndex()));
	  return ;

        default:
	  break ;
      }

      if ( weapon_sort > 0 )
        {
          Weapon_type weapon;
          if (weapons_list.GetWeaponBySort(weapon_sort, weapon))
            action_handler.NewAction(ActionInt(ACTION_CHANGE_WEAPON, weapon));

          return;
        }
    }

#ifdef CL
  // The key pressed was not in the previously managed key...
  // Try to manage it in the KeyEvent handler.

  HandleKeyEvent (key.id, KEY_PRESSED);
#endif
}
//-----------------------------------------------------------------------------

// Handle a released key
#ifdef CL
void Clavier::HandleKeyReleased (const CL_InputEvent &key)
#else
void Clavier::HandleKeyReleased (const Action_t &action)
#endif
{
#ifdef CL
  // Work-around for a bug from lower layers... Perhaps ClanLib.
  // Sometime, a key_release event is sent since the key is still pressed...
  if (CL_Keyboard::get_keycode(key.id))
    return ;

  std::map<int, Action_t>::iterator it = layout.find(key.id);

  if ( it == layout.end() )
    return;

  Action_t action = it->second;
#endif

  PressedKeys[action] = false ;

  // We manage here only actions which are active on KEY_RELEASED event.

  switch(action) {
    case ACTION_QUIT:
      jeu.fin_partie = true;
      return;

    case ACTION_PAUSE:
      jeu.Pause();
      return;

    case ACTION_FULLSCREEN:
#ifdef BUGGY_CODE
      video.SetFullScreen( !video.IsFullScreen() );
#endif
      return;

    case ACTION_TOGGLE_INTERFACE:
      interface.ChangeAffiche (!interface.EstAffiche());
      return;

    case ACTION_CENTER:
      curseur_ver.SuitVerActif();
      camera.ChangeObjSuivi (&ActiveCharacter(), true, true, true);
      return;

    case ACTION_TOGGLE_WEAPONS_MENUS:
      interface.weapons_menu.ChangeAffichage();
      return;

    default:
      break ;
  }

#ifdef CL
  HandleKeyEvent (key.id, KEY_RELEASED);
#endif

#ifdef CL
#ifdef USE_HAND_POSITION_MODIFIER
  bool meta = 
    CL_Keyboard::get_keycode(CL_KEY_LSHIFT)
    || CL_Keyboard::get_keycode(CL_KEY_RSHIFT);

  // Shift: Affiche/cache le niveau d'énergie des vers
  if (meta)
    {
      int frame = ActiveCharacter().image.get_current_frame();
      skin_translate_t &tr = 
	ActiveCharacter().AccessSkin().walking.hand_position[frame];
      if (key.id == CL_KEY_LEFT) { tr.dx--; return; }
      if (key.id == CL_KEY_RIGHT) { tr.dx++; return; }
      if (key.id == CL_KEY_UP) { tr.dy--; return; }
      if (key.id == CL_KEY_DOWN) { tr.dy++; return; }
    }
#endif
#endif
}

//-----------------------------------------------------------------------------

// Refresh keys which are still pressed.
void Clavier::Refresh()
{
#ifdef CL
  if(!ActiveTeam().is_local)
    return;

  if (CL_Keyboard::get_keycode(CL_KEY_UP))
    HandleKeyEvent(CL_KEY_UP, KEY_REFRESH);

  if (CL_Keyboard::get_keycode(CL_KEY_DOWN))
    HandleKeyEvent(CL_KEY_DOWN, KEY_REFRESH);

  if (CL_Keyboard::get_keycode(CL_KEY_LEFT))
    HandleKeyEvent(CL_KEY_LEFT, KEY_REFRESH);

  if (CL_Keyboard::get_keycode(CL_KEY_RIGHT))
    HandleKeyEvent(CL_KEY_RIGHT, KEY_REFRESH);

  if (CL_Keyboard::get_keycode(CL_KEY_SPACE))
    HandleKeyEvent(CL_KEY_SPACE, KEY_REFRESH);

#ifdef USE_HAND_POSITION_MODIFIER
      bool meta = 
	CL_Keyboard::get_keycode(CL_KEY_LSHIFT)
	|| CL_Keyboard::get_keycode(CL_KEY_RSHIFT);
      if (meta) return;
#endif
#endif

  //Treat KEY_REFRESH events:
  for (uint i = 0; i < ACTION_MAX; i++)
  if(PressedKeys[i])
  {
    if (ActiveTeam().GetWeapon().override_keys &&
        ActiveTeam().GetWeapon().IsActive())
    {
      ActiveTeam().AccessWeapon().HandleKeyEvent(i, KEY_REFRESH);
      return ;
    }

    ActiveCharacter().HandleKeyEvent(i,KEY_REFRESH);
  }
}

//-----------------------------------------------------------------------------
                                                                                    
void Clavier::TestCamera()
{
#ifdef CL
  if (CL_Keyboard::get_keycode(CL_KEY_NUMPAD4)) {
    camera.SetXY (-SCROLL_CLAVIER, 0);
    camera.autorecadre = false;
    return;
  }

  if (CL_Keyboard::get_keycode(CL_KEY_NUMPAD6)) {
    camera.SetXY (SCROLL_CLAVIER, 0);
    camera.autorecadre = false;
    return;
  }

  if (CL_Keyboard::get_keycode(CL_KEY_NUMPAD8)) {
    camera.SetXY (0, -SCROLL_CLAVIER);
    camera.autorecadre = false;
    return;
  }

  if (CL_Keyboard::get_keycode(CL_KEY_NUMPAD2)) {
    camera.SetXY (0,SCROLL_CLAVIER);
    camera.autorecadre = false;
    return;
  }
#else

#endif
}
