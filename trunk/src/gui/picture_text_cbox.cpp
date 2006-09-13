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
 * A beautiful checkbox with picture and text
 *****************************************************************************/

#include "picture_text_cbox.h"
#include "../include/app.h"
#include "../graphic/font.h"
#include "../graphic/sprite.h"
#include "../tool/resource_manager.h"

PictureTextCBox::PictureTextCBox(const std::string &label, const std::string &resource_id, const Rectanglei &rect, bool value):
  CheckBox(label, rect, value)
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);   
  m_image = resource_manager.LoadImage(res, resource_id);
 
  SetPosition( rect.GetPosition() );
  SetSize( rect.GetSize() );

  SetSizeY( (*Font::GetInstance(Font::FONT_SMALL)).GetHeight() );
  m_value = value;

  txt_label = new Text(label, white_color, Font::GetInstance(Font::FONT_SMALL));
}

void PictureTextCBox::Draw(const Point2i &mousePosition, Surface& surf)
{
  AppWormux::GetInstance()->video.window.Blit(m_image, GetPosition());

  txt_label->DrawTopLeft( GetPositionX(), GetPositionY()+m_image.GetHeight() );
  
  if (m_value)
    m_checked_image->SetCurrentFrame(0);
  else 
    m_checked_image->SetCurrentFrame(1);

  m_checked_image->Blit(surf, 
			GetPositionX() + GetSizeX() - 16, 
			GetPositionY() + m_image.GetHeight()/2);
}

