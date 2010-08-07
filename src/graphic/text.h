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
 ******************************************************************************/

#ifndef TEXT_H
#define TEXT_H

#include <string>
#include "colors.h"
#include "font.h"
#include "surface.h"
#include "tool/xml_document.h"

class Text
{
public:
  typedef enum
  {
    ALIGN_CENTER,
    ALIGN_TOP_LEFT,
    ALIGN_TOP_RIGHT,
    ALIGN_CENTER_TOP,
    ALIGN_CENTER_LEFT,
    ALIGN_CENTER_RIGHT
  } Alignment;

protected:
  Surface surf;
  Surface background; //shadow or outline or nothing;
  std::string txt;
  Color color;
  bool shadowed;
  bool dummy;
  uint bg_offset;
  uint max_width;
  Color shadowColor;
  Font::font_size_t font_size;
  Font::font_style_t font_style;

  virtual void Render();
  int GetLineHeight(const Font *f) const { return (7*f->GetHeight())>>3; }
  void RenderMultiLines();

public:
  Text(const std::string & text,
       const Color & fontColor = white_color,
       uint fontSize = 12,
       Font::font_style_t fontStyle = Font::FONT_BOLD,
       bool shadowed = true,
       const Color & shadowColor = black_color,
       bool dummy = false);
  Text(void);

  virtual ~Text();

  void Init();
  void LoadXMLConfiguration(XmlReader * xmlFile,
                            const xmlNode * textNode);
  Font::font_style_t DetectFontStyle(const std::string & fontStyle);

  //Draw method using windows coordinates
  void DrawCenter(const Point2i & position) const;
  void DrawTopLeft(const Point2i & position) const;
  void DrawTopRight(const Point2i & position) const;
  void DrawCenterTop(const Point2i & position) const;
  void DrawCenterLeft(const Point2i & position) const;
  void DrawCenterRight(const Point2i & position) const;

  //Draw text cursor only (text_pos = position for DrawTopLeft)
  void DrawCursor(const Point2i & text_pos,
                  std::string::size_type cursor_pos) const;

  //Draw method using map coordinates
  void DrawCenterTopOnMap(const Point2i & position) const;

  void SetText(const std::string & new_txt);
  const std::string & GetText() const;
  void SetColor(const Color & new_color);
  void SetMaxWidth(uint max_w);
  int GetWidth() const;
  int GetHeight() const;

  const Color & GetFontColor() const { return color; };
  Font::font_size_t GetFontSize() const { return font_size; };
  Font::font_style_t GetFontStyle() const { return font_style; };
  bool IsFontShadowed() const { return shadowed; };
  const Color & GetShadowColor(void) const { return this->shadowColor; };

  void SetFont(const Color & font_color,
               const Font::font_size_t font_size,
               const Font::font_style_t font_style,
               bool font_shadowed,
               const Color & shadowColor = black_color);
};

void DrawTmpBoxText(Font& font,
                    Point2i pos,
                    const std::string& txt, uint space=3,
                    const Color& boxColor = defaultColorBox,
                    const Color& rectColor = defaultColorRect);

#endif
