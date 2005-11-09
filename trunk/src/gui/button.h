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

#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#ifdef CL
#include <ClanLib/display.h>
#else
#endif
//-----------------------------------------------------------------------------

class Sprite;
struct Profile;

class Button
{
protected:
  uint m_x, m_width, m_y, m_height;
#ifdef CL
  CL_Sprite image;
#else
  Sprite *image;
#endif
   
public:
  Button();
  Button (uint x, uint y, uint w, uint h);
  virtual ~Button();
  void SetPos(uint x, uint y);
  void SetSize (uint larg, uint haut);
#ifdef CL
  void SetImage (const std::string& resource_id, CL_ResourceManager* manager);
#else
  void SetImage (const Profile *res_profile, const std::string& resource_id);
#endif 
  bool Test (uint souris_x, uint souris_y);
  virtual void Draw (uint souris_x, uint souris_y);
  uint GetX() const;
  uint GetY() const;
  uint GetWidth() const;
  uint GetHeight() const;

protected:
  void DrawImage (uint souris_x, uint souris_y);
};

//-----------------------------------------------------------------------------
#endif
