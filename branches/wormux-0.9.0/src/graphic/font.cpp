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

#include <iostream>

#include "include/app.h"
#include "game/config.h"
#include "graphic/font.h"
#include "graphic/video.h"
#include "map/map.h"
#include <WORMUX_file_tools.h>

Font* Font::FONT_ARRAY[] = {NULL, NULL, NULL, NULL, NULL, NULL};
Font* Font::FONT_ARRAY_BOLD[] = {NULL, NULL, NULL, NULL, NULL, NULL};
Font* Font::FONT_ARRAY_ITALIC[] = {NULL, NULL, NULL, NULL, NULL, NULL};
bool  Font::LIB_INIT = false;

/*
 * Constants
 */
// Size
const int Font::FONT_SIZE[] = {40, 32, 24, 16, 12, 10};

Font* Font::GetInstance(font_size_t ftype, font_style_t fstyle) {
  Font * font = NULL;
  int type = (int)ftype;

  if (FONT_ARRAY[ftype] == NULL) {
    try {
      if (!LIB_INIT && TTF_Init() == -1) {
        Error(Format("Initialisation of TTF library failed: %s", TTF_GetError()));
        exit(1);
      }
      LIB_INIT = true;

      // Load the font in the different styles
      FONT_ARRAY_BOLD[type] = new Font(FONT_SIZE[type]);
      FONT_ARRAY_BOLD[type]->SetBold();

      FONT_ARRAY_ITALIC[type] = new Font(FONT_SIZE[type]);
      FONT_ARRAY_ITALIC[type]->SetItalic();

      FONT_ARRAY[type] = new Font(FONT_SIZE[type]);
    }

    catch (const std::string e)
    {
      std::cerr << e << std::endl;
      exit(-1);
    }
  }

  switch(fstyle) {
  case FONT_BOLD:
    font = FONT_ARRAY_BOLD[type];
    break;
  case FONT_ITALIC:
    font = FONT_ARRAY_ITALIC[type];
    break;
  case FONT_NORMAL:
    font = FONT_ARRAY[type];
    break;
  }
  return font;
}

Font::Font(int size):
  surface_text_table(),
  m_font(NULL)
{
  const std::string filename = Config::GetInstance()->GetTtfFilename();

  if (DoesFileExist(filename))
  {
    m_font = TTF_OpenFont(filename.c_str(), size);
    if (!m_font)
      Error(Format("Error in font file %s (size:%d): %s", filename.c_str(), size, TTF_GetError()));
  }
  else
    Error("Can't find font file");

  TTF_SetFontStyle(m_font, TTF_STYLE_NORMAL);
}

Font::~Font(){
  if( m_font != NULL ){
    TTF_CloseFont(m_font);
    m_font = NULL;
  }

  txt_iterator it;

  // Fix bug #10866 and also fix memory leak.
  surface_text_table.clear();
}

void Font::ReleaseInstances(void)
{
  uint i;

  for (i=0; i<sizeof(FONT_ARRAY)/sizeof(Font*); i++)
  {
    if (FONT_ARRAY[i])
    {
      delete FONT_ARRAY[i];
      FONT_ARRAY[i] = NULL;
    }
  }

  for (i=0; i<sizeof(FONT_ARRAY_BOLD)/sizeof(Font*); i++)
  {
    if (FONT_ARRAY_BOLD[i])
    {
      delete FONT_ARRAY_BOLD[i];
      FONT_ARRAY_BOLD[i] = NULL;
    }
  }

  for (i=0; i<sizeof(FONT_ARRAY_ITALIC)/sizeof(Font*); i++)
  {
    if (FONT_ARRAY_ITALIC[i])
    {
      delete FONT_ARRAY_ITALIC[i];
      FONT_ARRAY_ITALIC[i] = NULL;
    }
  }

  TTF_Quit();
  LIB_INIT = false;
}

void Font::SetBold()
{
  TTF_SetFontStyle(m_font, TTF_STYLE_BOLD);
}

void Font::SetItalic()
{
  TTF_SetFontStyle(m_font, TTF_STYLE_ITALIC);
}

void Font::Write(const Point2i& pos, const Surface &surface) const {
  GetMainWindow().Blit(surface, pos);

  // TODO: Remove this line! (and use GameFont instead of Font)
  GetWorld().ToRedrawOnScreen( Rectanglei(pos, surface.GetSize()) );
}

void Font::WriteLeft(const Point2i &pos, const std::string &txt,
                     const Color &color){
  Surface surface(Render(txt, color, true));
  Write(pos, surface);
}

void Font::WriteLeftBottom(const Point2i &pos, const std::string &txt,
                           const Color &color){
  Surface surface(Render(txt, color, true));
  Write(pos - Point2i(0, surface.GetHeight()), surface);
}

void Font::WriteRight(const Point2i &pos, const std::string &txt,
                      const Color &color){
  Surface surface(Render(txt, color, true));
  Write(pos - Point2i(surface.GetWidth(), 0), surface);
}

void Font::WriteCenter (const Point2i &pos, const std::string &txt,
                        const Color &color){
  Surface surface(Render(txt, color, true));
  Write(pos - Point2i(surface.GetWidth()/2, surface.GetHeight()), surface);
}

void Font::WriteCenterTop(const Point2i &pos, const std::string &txt,
                          const Color &color){
  Surface surface(Render(txt, color, true));
  Write(pos - Point2i(surface.GetWidth()/2, 0), surface);
}

Surface Font::CreateSurface(const std::string &txt, const Color &color){
  return Surface( TTF_RenderUTF8_Blended(m_font, txt.c_str(), color.GetSDLColor()) );
}

Surface Font::Render(const std::string &txt, const Color &color, bool cache){
  Surface surface;

  if( cache ){
    txt_iterator p = surface_text_table.find(txt);
    if( p == surface_text_table.end() ){
      if( surface_text_table.size() > 5 ){
        //SDL_FreeSurface( surface_text_table.begin()->second );
        surface_text_table.erase( surface_text_table.begin() );
      }
      surface = CreateSurface(txt, color);
      surface_text_table.insert( txt_sample(txt, surface) );
    } else {
      txt_iterator p2 = surface_text_table.find( txt );
      surface = p2->second;
    }
  } else
    surface = CreateSurface(txt, color);

  ASSERT( !surface.IsNull() );
  return surface;
}

int Font::GetWidth (const std::string &txt) const {
  int width=-1;

  TTF_SizeUTF8(m_font, txt.c_str(), &width, NULL);

  return width;
}

int Font::GetHeight () const {
  return TTF_FontHeight(m_font);
}

int Font::GetHeight (const std::string &str) const {
  int height=-1;

  TTF_SizeUTF8(m_font, str.c_str(), NULL, &height);

  return height;
}

Point2i Font::GetSize(const std::string &txt) const {
  return Point2i(GetWidth(txt), GetHeight(txt));
}

Surface Font::GenerateSurface(const std::string &txt, const Color &color,
                              font_size_t font_size, font_style_t font_style)
{
  return Surface(Font::GetInstance(font_size, font_style)->CreateSurface(txt, color));
}
