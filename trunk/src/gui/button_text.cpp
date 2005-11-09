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
 * Button with text.
 *****************************************************************************/

#include "button_text.h"
//-----------------------------------------------------------------------------

ButtonText::ButtonText() : Button() 
{
#ifdef CL
  m_police = NULL;
#else
  font = NULL;
#endif
}

//-----------------------------------------------------------------------------

ButtonText::ButtonText (uint x, uint y, uint w, uint h, const std::string &text)
  : Button(x, y, w, h)
{ 
  m_text = text;
}


//-----------------------------------------------------------------------------

#ifdef CL
void ButtonText::SetFont (Police *police)
{ 
  m_police = police;
}
#else
void ButtonText::SetFont (Font *font)
{ 
  this->font = font;
}
#endif

//-----------------------------------------------------------------------------

void ButtonText::SetText(const std::string &text) 
{ 
  m_text = text; 
}

//-----------------------------------------------------------------------------

void ButtonText::Draw (uint souris_x, uint souris_y)
{
  DrawImage (souris_x, souris_y);
#ifdef CL
  assert (m_police != NULL);
#else
  assert (font != NULL);
#endif
  const int x = GetX()+GetWidth()/2;
  const int y = GetY()+GetHeight()/2;
#ifdef CL
  m_police -> WriteCenter (x, y, m_text);
#else
  font->WriteCenter (x, y, m_text, white_color);
#endif
}

//-----------------------------------------------------------------------------

std::string ButtonText::GetText() const 
{ 
  return m_text;
}

//-----------------------------------------------------------------------------
