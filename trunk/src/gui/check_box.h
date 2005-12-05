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
 * Checkbox in GUI.
 *****************************************************************************/

#ifndef CHECK_BOX_H
#define CHECK_BOX_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../graphic/text.h"
#include "../gui/widget.h"
#include <string>
//-----------------------------------------------------------------------------

class Sprite;

class CheckBox : public Widget
{
 private:
  Text * txt_label;
  bool m_value;
  Sprite *m_image;
   
 public:
  CheckBox(const std::string &label, uint x, uint y, uint w, bool value=true) ;  
  ~CheckBox();
  void Draw (uint mouse_x, uint mouse_y) ;
  bool Clic (uint mouse_x, uint mouse_y) ;
  bool GetValue() const;
  void SetValue(bool value);
};

//-----------------------------------------------------------------------------
#endif

