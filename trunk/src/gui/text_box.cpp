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
 * Text box widget
 *****************************************************************************/

#include <SDL_keyboard.h>
#include "gui/text_box.h"
#include "graphic/text.h"
#include "tool/text_handling.h"
#include "tool/copynpaste.h"

TextBox::TextBox(const std::string &label, uint width,
                 Font::font_size_t fsize, Font::font_style_t fstyle) :
  Label(label, 0, fsize, fstyle,
        white_color, Text::ALIGN_LEFT_TOP, true),
  max_nb_chars(0),
  cursor_pos(label.size())
{
  size.x = width;
  Widget::SetBorder(defaultOptionColorRect, 1);
  Widget::SetHighlightBgColor(highlightOptionColorBox);
}

TextBox::TextBox(Profile * profile,
                 const xmlNode * textBoxNode) :
  Label(profile, textBoxNode),
  max_nb_chars(0),
  cursor_pos(0)
{
}

bool TextBox::LoadXMLConfiguration()
{
  if (NULL == profile || NULL == widgetNode) {
    return false;
  }

  XmlReader * xmlFile = profile->GetXMLDocument();

  Label::LoadXMLConfiguration();

  Color hlBgColor = highlightOptionColorBox;
  xmlFile->ReadHexColorAttr(widgetNode, "hlBgColor", hlBgColor);
  Widget::SetHighlightBgColor(hlBgColor);

  SetText(Text::GetText());

  return true;
}

void TextBox::BasicSetText(std::string const & new_txt)
{
  std::string _new_txt = new_txt;

  if (max_nb_chars != 0 && _new_txt.size() > max_nb_chars) {
    _new_txt.resize(max_nb_chars);
  }

  //const Font * font = Font::GetInstance(GetFontSize(), GetFontStyle());

  //if (font->GetWidth(_new_txt) < GetSizeX() - 5) {
    Label::SetText(_new_txt);
  //} else {
  //  cursor_pos = GetText().size();
  //}
}

void TextBox::SetText(std::string const & new_txt)
{
  BasicSetText(new_txt);
  cursor_pos = GetText().size();
}

void TextBox::SetMaxNbChars(unsigned int nb_chars)
{
  max_nb_chars = nb_chars;
}

bool TextBox::SendKey(const SDL_keysym & key)
{
  bool used = true;

  NeedRedrawing();

  std::string new_txt = GetText();

  used = TextHandle(new_txt, cursor_pos, key);

  if (new_txt != GetText()) {
    BasicSetText(new_txt);
  }

  return used;
}

void TextBox::Draw(const Point2i & mousePosition) const
{
  Rectanglei clip;
  Rectanglei wlr = GetClip(clip);
  if (!wlr.GetSizeX() || !wlr.GetSizeY())
      return;

  Label::Draw(mousePosition);
  DrawCursor(position, cursor_pos);

  // Restore initial clip rectangle
  UnsetClip(clip);
}

Widget * TextBox::ClickUp(const Point2i & mousePosition,
                          uint button)
{
  NeedRedrawing();

  if (button == SDL_BUTTON_MIDDLE) {
    std::string new_txt = GetText();
    bool        used    = RetrieveBuffer(new_txt, cursor_pos);

    if (new_txt != GetText())
      BasicSetText(new_txt);
    return used ? this : NULL;
  } else if (button == SDL_BUTTON_LEFT) {
    const std::string      cur_txt = GetText();
    const Font*            font    = Font::GetInstance(GetFontSize(), GetFontStyle());
    std::string            txt     = "";
    std::string::size_type pos     = 0;

    cursor_pos = 0;
    while (pos <= cur_txt.size() &&
           this->position.x + font->GetWidth(txt) < mousePosition.x+2) {
      cursor_pos = pos;
      while ((cur_txt[pos++] & 0xc0) == 0x80
             && pos < cur_txt.size()) { } // eat all UTF-8 characters
      txt = cur_txt.substr(0, pos);
    }

    Label::Draw(mousePosition);
    DrawCursor(position, cursor_pos);

    return this;
  }

  // Om nom nom
  return this;
}


PasswordBox::PasswordBox(const std::string & label,
                         uint max_width,
                         Font::font_size_t fsize,
                         Font::font_style_t fstyle) :
  TextBox(label, max_width, fsize, fstyle)
{
}

PasswordBox::PasswordBox(Profile * profile,
                         const xmlNode * passwordBoxNode) :
  TextBox(profile, passwordBoxNode)
{
}

void PasswordBox::BasicSetText(std::string const & new_txt)
{
  std::string _new_txt = new_txt;

  if (max_nb_chars != 0 && _new_txt.size() > max_nb_chars) {
    _new_txt.resize(max_nb_chars);
  }
  clear_text = _new_txt;

  //printf("Real text: %s\n", clear_text.c_str());

  Font * font = Font::GetInstance(GetFontSize(), GetFontStyle());

  if (font->GetWidth(_new_txt) < GetSizeX() - 5) {
    Label::SetText(std::string(clear_text.size(), '*'));
  } else {
    cursor_pos = GetText().size();
  }
}

bool PasswordBox::SendKey(const SDL_keysym & key)
{
  bool used = true;

  NeedRedrawing();

  std::string new_txt = clear_text;

  used = TextHandle(new_txt, cursor_pos, key);

  if (new_txt != GetText()) {
    BasicSetText(new_txt);
  }

  return used;
}
