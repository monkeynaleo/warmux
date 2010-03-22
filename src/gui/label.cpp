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
 * Label in GUI.
 *****************************************************************************/

#include "gui/label.h"

Label::Label(const std::string & text,
             uint maxWidth,
             Font::font_size_t fontSize,
             Font::font_style_t fontStyle,
             const Color & fontColor,
             bool centered,
             bool shadowed,
             const Color & shadowColor) :
  Text(text, fontColor, fontSize,
       fontStyle, shadowed, shadowColor),
  center(centered)
{
  size.x = maxWidth;
  SetMaxWidth(size.x);
  size.y = GetHeight();
}

Label::Label(const Point2i & size) :
  Widget(size),
  center(false)
{
}

Label::Label(Profile * profile,
             const xmlNode * pictureNode) :
  Widget(profile, pictureNode),
  center(false)
{
}

Label::~Label()
{
}

bool Label::LoadXMLConfiguration()
{
  if (NULL == profile || NULL == widgetNode) {
    //TODO error ... xml attributs not initialized !
    return false;
  }

  XmlReader * xmlFile = profile->GetXMLDocument();

  ParseXMLPosition();
  ParseXMLSize();
  ParseXMLBorder();
  ParseXMLBackground();

  std::string xmlText("Text not found");
  xmlFile->ReadStringAttr(widgetNode, "text", xmlText);

  Color textColor(0, 0, 0, 255);
  xmlFile->ReadHexColorAttr(widgetNode, "textColor", textColor);

  // Load the font size ... based on 72 DPI 
  int fontSize = ParseVerticalTypeAttribut("fontSize", 12);

  std::string fontStyle;
  xmlFile->ReadStringAttr(widgetNode, "fontStyle", fontStyle);

  bool activeShadow = false;
  xmlFile->ReadBoolAttr(widgetNode, "shadow", activeShadow);
  Color shadowColor(255, 255, 255, 255);
  xmlFile->ReadHexColorAttr(widgetNode, "shadowColor", shadowColor);

  Text::SetText(xmlText);
  Text::SetFont(textColor, 
                (Font::font_size_t)fontSize, 
                 DetectFontStyle(fontStyle),
                 activeShadow,
                 shadowColor);
  Text::Init();
  return true;
}

Font::font_style_t Label::DetectFontStyle(const std::string & fontStyle)
{
  if ("bold" == fontStyle) {
    return Font::FONT_BOLD;
  } else if ("italic" == fontStyle) {
    return Font::FONT_ITALIC;
  }
  return Font::FONT_NORMAL;
}

void Label::Draw(const Point2i & mousePosition) const
{
  (void)mousePosition;

  if (!center) {
    DrawTopLeft(position);
  } else {
    DrawCenterTop(Point2i(position.x + size.x/2, position.y));
  }
}

void Label::Pack()
{
  SetMaxWidth(size.x);
  size.y = GetHeight();
}

void Label::SetText(const std::string & new_txt)
{
  NeedRedrawing();

  Text::SetText(new_txt);

  SetMaxWidth(size.x);
  size.y = GetHeight();
}

