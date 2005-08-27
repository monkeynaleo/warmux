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
 * Police de caractère.
 *****************************************************************************/

#include "font.h"
//-----------------------------------------------------------------------------
#include "../tool/string_tools.h"
//-----------------------------------------------------------------------------

Police::Police()
{ m_police = NULL; }

//-----------------------------------------------------------------------------

void Police::Load (const std::string& resource_id, 
		     CL_ResourceManager* manager)
{
  assert (m_police == NULL);
  m_police = new CL_Font(resource_id, manager);
}
//-----------------------------------------------------------------------------

void Police::WriteLeft (int x, int y, const std::string &txt)
{ 
  Acces().set_alignment (origin_top_left);
  Acces().draw(x,y,txt); 
}

//-----------------------------------------------------------------------------

void Police::WriteLeftBottom (int x, int y, const std::string &txt)
{ 
  Acces().set_alignment (origin_bottom_left);
  Acces().draw(x,y,txt); 
}

//-----------------------------------------------------------------------------

void Police::WriteRight (int x, int y, const std::string &txt)
{ 
  Acces().set_alignment (origin_top_right);
  Acces().draw(x,y,txt); 
}

//-----------------------------------------------------------------------------

void Police::WriteCenter (int txt_x, int txt_y, const std::string &txt)
{ 
  Acces().set_alignment (origin_center);
  Acces().draw(txt_x,txt_y,txt); 
}

//-----------------------------------------------------------------------------

void Police::WriteCenterTop (int txt_x, int txt_y, const std::string &txt)
{ 
  Acces().set_alignment (origin_top_center);
  Acces().draw(txt_x,txt_y,txt); 
}

//-----------------------------------------------------------------------------

CL_Font &Police::Acces() 
{ assert (m_police != NULL); return *m_police; }

//-----------------------------------------------------------------------------

const CL_Font &Police::Read() const 
{ assert (m_police != NULL); return *m_police; }

//-----------------------------------------------------------------------------


uint Police::GetWidth (const std::string &txt)
{ 
  return Acces().get_width(txt);
}

//-----------------------------------------------------------------------------

uint Police::GetHeight (const std::string &txt)
{ 
  return Acces().get_height(txt);
}

//-----------------------------------------------------------------------------
