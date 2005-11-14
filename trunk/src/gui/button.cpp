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
 * Button for graphic interface. It could be in two states : idle or 
 * "mouseover".
 *****************************************************************************/

#include "button.h"
#include "../tool/resource_manager.h"
#include "../tool/sprite.h"

//-----------------------------------------------------------------------------

Button::Button ()
{ 
   SetPos(0,0); 
   SetSize(0,0);
}

//-----------------------------------------------------------------------------

Button::Button (uint x, uint y, uint w, uint h)
  : m_x(x), m_width(w), m_y(y), m_height(h)
{}

//-----------------------------------------------------------------------------

Button::~Button() 
{
} 

//-----------------------------------------------------------------------------

void Button::SetPos (uint x, uint y)
{
  m_x  = x;
  m_y  = y;
}

//-----------------------------------------------------------------------------

void Button::SetSize (uint w, uint h)
{
  m_width = w;
  m_height = h;
}

//-----------------------------------------------------------------------------

bool Button::Test (uint souris_x, uint souris_y)
{
  return ((m_x <= souris_x) && (souris_x <= m_x+m_width)
	  && (m_y <= souris_y) && (souris_y <= m_y+m_height));
}

//-----------------------------------------------------------------------------

void Button::Draw (uint souris_x, uint souris_y)
{ 
   DrawImage (souris_x, souris_y);
}

//-----------------------------------------------------------------------------

void Button::DrawImage (uint souris_x, uint souris_y)
{
  uint frame = Test(souris_x,souris_y)?1:0;
#ifdef CL
  image.set_frame (frame);
  image.draw(CL_Rect(m_x, m_y, m_x+m_width, m_y+m_height)); 
#else
  image->SetCurrentFrame (frame);
  image->Draw(m_x, m_y);
#endif
}

//-----------------------------------------------------------------------------

#ifdef CL
void Button::SetImage (const std::string& resource_id, CL_ResourceManager* manager)
{ 
  image = CL_Sprite(resource_id, manager);
  SetSize (image.get_width(), image.get_height());
}
#else
void Button::SetImage (const Profile *res_profile, const std::string& resource_id)
{
  image = resource_manager.LoadSprite(res_profile,resource_id);
  SetSize (image->GetWidth(), image->GetHeight());   
}
#endif

//-----------------------------------------------------------------------------

uint Button::GetX() const { return m_x; }
uint Button::GetY() const { return m_y; }
uint Button::GetWidth() const { return m_width; }
uint Button::GetHeight() const { return m_height; }

//-----------------------------------------------------------------------------
