/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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

#include "gui/check_box.h"
#include "graphic/text.h"
#include "graphic/font.h"
#include "graphic/sprite.h"
#include "graphic/video.h"
#include "include/app.h"
#include "tool/resource_manager.h"

CheckBox::CheckBox(const std::string & label, 
                   uint width, 
                   bool value):
  Text(label, 
       white_color, 
       Font::FONT_SMALL, 
       Font::FONT_BOLD),
  m_value(value),
  m_checked_image(NULL)
{
  Init(width);
}

CheckBox::CheckBox(Profile * profile,
                   const xmlNode * pictureNode) :
  Widget(profile, pictureNode),
  m_value(false),
  m_checked_image(NULL)
{
}

void CheckBox::Init(uint width)
{
  Profile * res = GetResourceManager().LoadXMLProfile( "graphism.xml", false);
  m_checked_image = GetResourceManager().LoadSprite(res, "menu/check");
  GetResourceManager().UnLoadXMLProfile(res);

  m_checked_image->cache.EnableLastFrameCache();

  position = Point2i(W_UNDEF, W_UNDEF);
  size.x = width;
  size.y = Text::GetHeight();
}

CheckBox::~CheckBox()
{
  if (NULL != m_checked_image) {
    delete m_checked_image;
  }
}

void CheckBox::Pack()
{
  Text::SetMaxWidth(size.x - m_checked_image->GetWidth() -2);
  size.y = std::max(uint(Text::GetHeight()),
		    m_checked_image->GetHeight());
}

bool CheckBox::LoadXMLConfiguration()
{
  if (NULL == profile || NULL == widgetNode) {
    return false;
  }

  XmlReader * xmlFile = profile->GetXMLDocument();

  ParseXMLPosition();
  ParseXMLSize();
  ParseXMLBorder();

  std::string xmlText("Text not found");
  xmlFile->ReadStringAttr(widgetNode, "text", xmlText);

  std::string file("menu/pic_not_found.png");
  xmlFile->ReadStringAttr(widgetNode, "pictureChecked", file);
  file = profile->relative_path + file;
  Surface picChecked(file.c_str());

  file = "menu/pic_not_found.png";
  xmlFile->ReadStringAttr(widgetNode, "pictureUnchecked", file);
  file = profile->relative_path + file;
  Surface picUnchecked(file.c_str());

  m_checked_image = new Sprite();
  m_checked_image->AddFrame(picChecked);
  m_checked_image->AddFrame(picUnchecked);

  //m_checked_image->cache.EnableLastFrameCache();

  return true;
}


void CheckBox::Draw(const Point2i &/*mousePosition*/) const
{
  Surface& surf = GetMainWindow();

  Text::DrawTopLeft(GetPosition());

  if (m_value) {
    m_checked_image->SetCurrentFrame(0);
  } else {
    m_checked_image->SetCurrentFrame(1);
  }

  m_checked_image->Blit(surf, GetPositionX() + GetSizeX() - 16, GetPositionY());
}

Widget * CheckBox::ClickUp(const Point2i &/*mousePosition*/, 
                           uint /*button*/)
{
  NeedRedrawing();
  m_value = !m_value;
  return this;
}
