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
 * Spinbutton in GUI.
 *****************************************************************************/

#ifndef SPIN_BUTTON_H
#define SPIN_BUTTON_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "button.h"
#include <string>
#include <sstream>
//-----------------------------------------------------------------------------

class SpinButton
{
 private:
  std::string m_label;
  uint m_x, m_y;
  uint m_height, m_width;
  uint m_label_x, m_label_y;
  uint m_value_x, m_value_y;

  uint value_width;
  uint label_width, label_height;
  uint m_img_plus_x, m_img_plus_y;
  uint m_img_minus_x, m_img_minus_y;
  std::ostringstream max_value_s;

  int m_value;
  int m_min_value, m_max_value, m_step;
  Button m_plus, m_minus;
  
 public:
  SpinButton();
  void Init (const std::string &label, uint x, uint y, int value=0, 
	     int step=1, int min_value=-1, int max_value=-1) ;  
  void Display (uint mouse_x, uint mouse_y) ;
  bool Clic (uint mouse_x, uint mouse_y) ;
  int GetValue() const;
  void SetValue(int value);
  void SetXY (uint x, uint y);
};

//-----------------------------------------------------------------------------
#endif

