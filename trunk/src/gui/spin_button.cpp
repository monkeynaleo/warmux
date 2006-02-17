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
 *****************************************************************************/

#include "spin_button.h"
#include <sstream>
#include <iostream>
#include "../include/app.h"
#include "../tool/math_tools.h"
#include "../tool/resource_manager.h"
#include "../include/global.h"

SpinButton::SpinButton (const std::string &label, const Rectanglei &rect,
			     int value, int step, int min_value, int max_value){
  position =  rect.GetPosition();
  size = rect.GetSize();
  SetSizeY( global().small_font().GetHeight() );
	  
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false); 

  txt_label = new Text(label, white_color, &global().small_font());

  if ( min_value != -1 && min_value <= value)
    m_min_value = min_value;
  else m_min_value = value/2;

  if ( max_value != -1 && max_value >= value)
    m_max_value = max_value;
  else m_max_value = value*2;

  txt_value = new Text("", white_color, &global().small_font());
  SetValue(value);


  std::ostringstream max_value_s;
  max_value_s << m_max_value ;
  uint max_value_w = global().small_font().GetWidth(max_value_s.str());
  
  uint margin = 5;

  m_plus = new Button( Rectanglei(GetPositionX() + GetSizeX() - 5, GetPositionY(), 5, 10), res, "menu/plus");
  m_minus = new Button( Rectanglei(GetPositionX() + GetSizeX() - max_value_w - 5 - 2 * margin, GetPositionY(), 5, 10), res, "menu/minus");   

  m_step = step;
}

SpinButton::~SpinButton (){
  delete txt_label;
  delete txt_value;
}

void SpinButton::SetSizePosition(const Rectanglei &rect){
  StdSetSizePosition(rect);

  std::ostringstream max_value_s;
  max_value_s << m_max_value ;
  uint max_value_w = global().small_font().GetWidth(max_value_s.str());
  
  uint margin = 5;
  
  m_plus->SetSizePosition( Rectanglei(GetPositionX() + GetSizeX() - 5, GetPositionY(), 5, 10) );
  m_minus->SetSizePosition( Rectanglei(GetPositionX() + GetSizeX() - max_value_w - 5 - 2 * margin, GetPositionY(), 5, 10) );
}

void SpinButton::Draw (uint mouse_x, uint mouse_y)
{
  txt_label->DrawTopLeft( GetPosition() );
   
  m_minus->Draw (mouse_x, mouse_y);
  m_plus->Draw (mouse_x, mouse_y);

  uint center = (m_plus->GetPositionX() + 5 + m_minus->GetPositionX() )/2;
  txt_value->DrawCenterTop(center, position.y);
}

bool SpinButton::Clic (uint mouse_x, uint mouse_y, uint button) 
{
  if ((button == SDL_BUTTON_WHEELDOWN && MouseIsOver(mouse_x, mouse_y)) ||
      (button == SDL_BUTTON_LEFT && m_minus->MouseIsOver(mouse_x, mouse_y))) {
    SetValue(m_value - m_step);
    return true;
  } else if ((button == SDL_BUTTON_WHEELUP && MouseIsOver(mouse_x, mouse_y)) ||
              (button == SDL_BUTTON_LEFT && m_plus->MouseIsOver(mouse_x, mouse_y))) {
    SetValue(m_value + m_step);
    return true;
  }
  return false;
}

int SpinButton::GetValue()  const { return m_value; }

void SpinButton::SetValue(int value)  
{
  m_value = BorneLong(value, m_min_value, m_max_value);  

  std::ostringstream value_s;
  value_s << m_value ;

  std::string s(value_s.str());
  txt_value->Set(s);
}
