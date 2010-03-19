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

#ifndef LABEL_H
#define LABEL_H

#include "include/base.h"
#include "gui/widget.h"
#include "graphic/text.h"
#include <string>

class Label : public Widget
{
  /* If you need this, implement it (correctly)*/
  Label(const Label&);
  Label operator=(const Label&);
  /*********************************************/
 protected:
  Text * textEngine;

 private:
  bool center;

  Font::font_style_t DetectFontStyle(const std::string & fontStyle);

 public:
  Label(const std::string & label,
        uint max_width,
        Font::font_size_t font_size = Font::FONT_SMALL,
        Font::font_style_t font_style = Font::FONT_BOLD,
        const Color & color = white_color,
        bool center = false,
        bool shadowed = true,
        const Color & shadowColor = black_color);
  Label(const Point2i & size);
  Label(Profile * profile,
        const xmlNode * pictureNode);
  Label(void) {}
  virtual ~Label();

  virtual bool LoadXMLConfiguration(void);
  virtual void Draw(const Point2i & mousePosition) const;
  void DrawCursor(const Point2i & text_pos,
                  std::string::size_type cursor_pos) const;

  void SetText(const std::string & new_txt);
  void SetFont(const Color & fontColor,
               const Font::font_size_t fontSize,
               const Font::font_style_t fontStyle,
               bool fontShadowed,
               const Color & shadowColor = black_color);

  const std::string & GetText() { return textEngine->GetText(); };
  Text * GetTextEngine() const { return textEngine; };
  Font::font_size_t GetFontSize() const { return textEngine->GetFontSize(); };
  Font::font_style_t GetFontStyle() const { return textEngine->GetFontStyle(); };

  virtual void OnFontChange();
  virtual void Pack();
};

#endif
