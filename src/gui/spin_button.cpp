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

#include "spin_button.h"
//-----------------------------------------------------------------------------
#include "../graphic/graphism.h"
#include "../tool/math_tools.h"
#ifndef CL
#include "../tool/resource_manager.h"
#endif
# include <sstream>
using namespace Wormux;
//-----------------------------------------------------------------------------

SpinButton::SpinButton()
{}

//-----------------------------------------------------------------------------

void SpinButton::Init (const std::string &label, uint x, uint y,
		       int value, int step, int min_value, int max_value)
{
  // Load images
#ifdef CL
  m_plus.SetImage ("menu/plus",graphisme.LitRes());
  m_minus.SetImage ("menu/minus", graphisme.LitRes());
#else
  // UGLY -> TODO find a place to store the graphism.xml (in app ?) profile 
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml"); 
  m_plus.SetImage (res, "menu/plus");
  m_minus.SetImage (res, "menu/minus");   
#endif
   
  // Copy arguments
  m_label = label;
  m_x = x;
  m_y = y;

  if ( min_value != -1 && min_value <= value)
    m_min_value = min_value;
  else m_min_value = value/2;

  if ( max_value != -1 && max_value >= value)
    m_max_value = max_value;
  else m_max_value = value*2;

  m_value = value;
  m_step = step;

  //std::ostringstream max_value_s;
  max_value_s << max_value ;

  // Calculate coordonnates
#ifdef CL
  /*uint*/ label_width = police_mix.GetWidth(m_label);
  /*uint*/ label_height = police_mix.GetHeight(m_label);

  // for having margins
  /*uint */value_width = police_mix.GetHeight( max_value_s.str() ) * 2 ;
#else
  /*uint*/ label_width = small_font.GetWidth(m_label);
  /*uint*/ label_height = small_font.GetHeight();

  // for having margins
  /*uint */value_width = small_font.GetHeight() * 2 ;
   
#endif
   
  m_width  = label_width + m_minus.GetWidth();
  m_width += value_width + m_plus.GetWidth();
#if !defined WIN32 || defined __MINGW32__
  m_height = std::max(label_height, m_plus.GetHeight());
#else
  m_height = _cpp_max(label_height, m_plus.GetHeight());
#endif
  m_img_minus_x = m_x + label_width;
  m_img_minus_y = m_y + (m_height - m_minus.GetWidth())/2;

  m_value_x = m_x + label_width + m_minus.GetWidth() + (value_width/2);
  m_value_y = m_y + (m_height - label_height)/2;

  m_img_plus_x = m_x + label_width + m_minus.GetWidth() + value_width ;
  m_img_plus_y = m_y + (m_height - m_plus.GetWidth())/2;

  m_plus.SetPos (m_img_plus_x, m_img_plus_y);
  m_minus.SetPos (m_img_minus_x, m_img_minus_y);

  m_label_x = m_x;
  m_label_y = m_y + (m_height - label_height)/2;
}

//-----------------------------------------------------------------------------

void SpinButton::SetXY (uint x, uint y)
{
  m_x = x;
  m_y = y;

  m_label_x = m_x;
  m_label_y = m_y + (m_height - label_height)/2;

  label_width += 10;//add width (10pixels)

  m_img_minus_x = m_x + label_width;
  m_img_minus_y = m_y + (m_height - m_minus.GetWidth())/2;

  m_value_x = m_x + label_width + m_minus.GetWidth() + (value_width/2);
  m_value_y = m_y + (m_height - label_height)/2;

  m_img_plus_x = m_x + label_width + m_minus.GetWidth() + value_width ;
  m_img_plus_y = m_y + (m_height - m_plus.GetWidth())/2;

  m_plus.SetPos (m_img_plus_x, m_img_plus_y);
  m_minus.SetPos (m_img_minus_x, m_img_minus_y);
}
//-----------------------------------------------------------------------------

void SpinButton::Display (uint mouse_x, uint mouse_y)
{
#ifdef CL
  police_mix.WriteLeft(m_label_x, m_label_y, m_label);
#else
  small_font.WriteLeft(m_label_x, m_label_y, m_label, white_color); 
#endif
   
  // Display text
  std::ostringstream value_s;
  value_s << m_value ;
#ifdef CL
  police_mix.WriteCenterTop(m_value_x, m_value_y, value_s.str());
#else
  small_font.WriteCenter(m_value_x, m_value_y, value_s.str(), white_color);
#endif
   
  // Display minus image
  m_minus.Draw (mouse_x, mouse_y);
  m_plus.Draw (mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------

bool SpinButton::Clic (uint mouse_x, uint mouse_y) 
{
  if (m_minus.Test(mouse_x, mouse_y)) {
    SetValue(m_value - m_step);
    return true;
  } else if (m_plus.Test(mouse_x, mouse_y)) {
    SetValue(m_value + m_step);
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

int SpinButton::GetValue()  const { return m_value; }

//-----------------------------------------------------------------------------

void SpinButton::SetValue(int value)  
{
  m_value = BorneLong(value, m_min_value, m_max_value);
}

//-----------------------------------------------------------------------------
