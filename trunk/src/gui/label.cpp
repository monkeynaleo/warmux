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

Label::Label(const std::string & _text,
             uint max_width,
             Font::font_size_t fsize,
             Font::font_style_t fstyle,
             const Color & fontColor,
             bool _center,
             bool shadowed,
             const Color & shadowColor) :
  textEngine(NULL),
  center(_center)
{
  textEngine = new Text(_text, fontColor, fsize,
                        fstyle, shadowed, shadowColor);
  size.x = max_width;
  textEngine->SetMaxWidth(size.x);
  size.y = textEngine->GetHeight();
}

Label::Label(const Point2i & size) :
  Widget(size),
  textEngine(NULL),
  center(false)
{
}

Label::Label(Profile * profile,
             const xmlNode * pictureNode) :
  Widget(profile, pictureNode),
  textEngine(NULL),
  center(false)
{
}

Label::~Label()
{
  if (NULL != textEngine) {
    delete textEngine;
  }
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

  // Load the font size ... 
  int fontSize = ParseVerticalTypeAttribut("fontSize", 12);

  std::string fontStyle;
  xmlFile->ReadStringAttr(widgetNode, "fontStyle", fontStyle);

  bool activeShadow = false;
  xmlFile->ReadBoolAttr(widgetNode, "shadow", activeShadow);
  Color shadowColor(255, 255, 255, 255);
  xmlFile->ReadHexColorAttr(widgetNode, "shadowColor", shadowColor);

  textEngine->SetText(xmlText);
  textEngine->SetFont(textColor, 
                      (Font::font_size_t)fontSize, 
                      DetectFontStyle(fontStyle),
                      activeShadow,
                      shadowColor);

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
    textEngine->DrawTopLeft(position);
  } else {
    textEngine->DrawCenterTop(Point2i(position.x + size.x/2, position.y));
  }
}

void Label::DrawCursor(const Point2i & textPos, 
                      std::string::size_type cursorPos) const
{
  textEngine->DrawCursor(textPos, cursorPos);
}

void Label::Pack()
{
  textEngine->SetMaxWidth(size.x);
  size.y = textEngine->GetHeight();
}

void Label::SetText(const std::string & new_txt)
{
  NeedRedrawing();

  textEngine->SetText(new_txt);

  textEngine->SetMaxWidth(size.x);
  size.y = textEngine->GetHeight();
}

void Label::SetFont(const Color & fontColor,
                    const Font::font_size_t fontSize,
                    const Font::font_style_t fontStyle,
                    bool fontShadowed,
                    const Color & shadowColor)
{
  textEngine->SetFont(fontColor, fontSize, fontStyle,
                      fontShadowed, shadowColor);
}

void Label::OnFontChange()
{
  SetText(textEngine->GetText());
}
