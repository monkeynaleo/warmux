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
#include "../graphic/sprite.h"
#include "../tool/resource_manager.h"

using namespace Wormux;
//-----------------------------------------------------------------------------


CheckBox::CheckBox (const std::string &label, uint x, uint y, uint w, bool value) :
  Widget(x, y, w, small_font.GetHeight())
{
  // UGLY -> TODO find a place to store the graphism.xml (in app ?) profile 
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");   
  m_image = resource_manager.LoadSprite( res, "menu/check");

  // Copy arguments
  m_value = value;

  txt_label = new Text(label, white_color, &small_font);
}

//-----------------------------------------------------------------------------

CheckBox::~CheckBox()
{
  delete txt_label;
}

//-----------------------------------------------------------------------------

void CheckBox::Draw (uint mouse_x, uint mouse_y)
{
  txt_label->DrawTopLeft(x, y);
 
  if (m_value)
    m_image->SetCurrentFrame(0);
  else 
    m_image->SetCurrentFrame(1);

  m_image->Draw(x+w-16, y);
}

//-----------------------------------------------------------------------------

bool CheckBox::Clic (uint mouse_x, uint mouse_y)
{
  if (!MouseIsOver(mouse_x, mouse_y)) return false;

  m_value = !m_value ;
  return true ;
}

//-----------------------------------------------------------------------------

bool CheckBox::GetValue()  const { return m_value; }
void CheckBox::SetValue(bool value)  { m_value = value; }

//-----------------------------------------------------------------------------
