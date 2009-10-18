/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * Keyboard management
 *****************************************************************************/

#ifndef KEYBOARD_H
#define KEYBOARD_H
//-----------------------------------------------------------------------------
#include "interface/man_machine_interface.h"
#include <WORMUX_singleton.h>
#include <set>
#include "SDL_keyboard.h"
#include "tool/xml_document.h"
//-----------------------------------------------------------------------------

class Keyboard : public ManMachineInterface, public Singleton<Keyboard>
{
private:
  int modifier_bits;
  std::set<SDLKey> pressed_keys;
  void HandleKeyComboEvent(int key_code, Key_Event_t event_type);
protected:
  friend class Singleton<Keyboard>;
  Keyboard();
  void SetDefaultConfig();

public:
  void HandleKeyEvent(const SDL_Event& event);
  void SetConfig(const xmlNode *node);
};

//-----------------------------------------------------------------------------
#endif /* KEYBOARD_H */
