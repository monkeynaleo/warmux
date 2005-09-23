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

#ifndef KEYBOARD_H
#define KEYBOARD_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#ifdef CL
# include <ClanLib/display.h>  // Pour CL_Slot
#else
#include <SDL.h>
#endif
# include <map>
#include "../include/action.h"
//-----------------------------------------------------------------------------

class Clavier
{
private:
#ifdef CL
  CL_Slot slot_up, slot_down;
  bool pilote_installe;
#endif
  std::map<int, Action_t> layout;
  bool PressedKeys[ACTION_MAX];

private:
  // Traite une touche relachée
#ifdef CL
  void HandleKeyPressed (const CL_InputEvent &event);
  void HandleKeyReleased (const CL_InputEvent &event);
  void HandleKeyEvent(int key, int event_type) ;
#else
  void HandleKeyPressed (const SDL_keysym *key);
  void HandleKeyReleased (const SDL_keysym *key);
 public:
   void HandleKeyEvent( const SDL_Event *event) ;
 private:
#endif
   
public:
  Clavier();

  // On veut bouger la caméra au clavier ?
  void TestCamera();

  // Refresh des touches du clavier
  void Refresh();
  
  // Associe une touche à une action.
  void SetKeyAction(int key, Action_t at);

#ifdef CL
  // Installe/désinstalle le pilote
  void DesinstallePilote();
  void InstallePilote();
#endif
};

extern Clavier clavier;

//-----------------------------------------------------------------------------
#endif
