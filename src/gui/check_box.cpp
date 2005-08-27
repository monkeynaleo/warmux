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

#include "check_box.h"
//-----------------------------------------------------------------------------
#include <algorithm>
#include "../graphic/graphism.h"
using namespace Wormux;
//-----------------------------------------------------------------------------

CheckBox::CheckBox()
{}

//-----------------------------------------------------------------------------

void CheckBox::Init (const std::string &label, uint x, uint y, bool value) 
{
  m_image = CL_Sprite("menu/check", graphisme.LitRes());

  // Copy arguments
  m_label = label;
  m_x = x;
  m_y = y;
  m_value = value;

  // Calculate coordonnates
  uint label_width = police_mix.GetWidth(m_label);
  uint label_height = police_mix.GetHeight(m_label);

  m_width = label_width + m_image.get_width();
#ifndef WIN32
  m_height = std::max(label_height, uint(m_image.get_height()));
#else
  m_height = _cpp_max(label_height, uint(m_image.get_height()));
#endif
  m_img_x = m_x + label_width;
  m_img_y = m_y + (m_height - m_image.get_height())/2;
  m_label_x = m_x;
  m_label_y = m_y + (m_height - label_height)/2;
}

//-----------------------------------------------------------------------------

void CheckBox::SetXY (uint x, uint y)
{
  m_x = x; m_y = y;

  uint label_width = police_mix.GetWidth(m_label);
  uint label_height = police_mix.GetHeight(m_label);

  m_img_x = m_x + label_width;
  m_img_y = m_y + (m_height - m_image.get_height())/2;
  m_label_x = m_x;
  m_label_y = m_y + (m_height - label_height)/2;
}

//-----------------------------------------------------------------------------

void CheckBox::Display (uint mouse_x, uint mouse_y)
{
  police_mix.WriteLeft(m_label_x, m_label_y, m_label);
  if (m_value)
    m_image.set_frame(0);
  else 
    m_image.set_frame(1);

  m_image.draw(m_img_x, m_img_y);
}

//-----------------------------------------------------------------------------

bool CheckBox::Clic (uint mouse_x, uint mouse_y)
{
  if ((mouse_x < m_x)
      || (m_x+m_width <= mouse_x)
      || (mouse_y < m_y)
      || (m_y+m_height <= mouse_y)) return false;

  m_value = !m_value ;
  return true ;
}

//-----------------------------------------------------------------------------

bool CheckBox::GetValue()  const { return m_value; }
void CheckBox::SetValue(bool value)  { m_value = value; }

//-----------------------------------------------------------------------------
