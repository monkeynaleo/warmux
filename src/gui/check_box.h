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
#include <string>
#include <ClanLib/display.h>
//-----------------------------------------------------------------------------

class CheckBox
{
 private:
  std::string m_label;
  uint m_x, m_y;
  uint m_height, m_width;
  uint m_img_x, m_img_y;
  uint m_label_x, m_label_y;
  bool m_value;
  CL_Sprite m_image;
  
 public:
  CheckBox();
  void Init (const std::string &label, uint x, uint y, bool value=true) ;  
  void Display (uint mouse_x, uint mouse_y) ;
  bool Clic (uint mouse_x, uint mouse_y) ;
  bool GetValue() const;
  void SetValue(bool value);
  void SetXY (uint x, uint y);
};

//-----------------------------------------------------------------------------
#endif

