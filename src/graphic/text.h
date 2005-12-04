#ifndef TEXT_H
#define TEXT_H

#include <string>
#include "font.h"
#include "colors.h"

class Text
{
  SDL_Surface* surf;
  std::string txt;
  Font* font;
  SDL_Color color;

public:
  Text(std::string new_txt, SDL_Color new_color = white_color, Font* new_font = &small_font);
  ~Text();

  //Draw method using windows coordinates
  void DrawCenter(int x, int y);
  void DrawTopLeft(int x, int y);
  void DrawCenterTop(int x, int y);

  //Draw method using map coordinates
  void DrawCenterOnMap(int x, int y);
  void DrawTopLeftOnMap(int x, int y);
  void DrawCenterTopOnMap(int x, int y);

  void Set(std::string &new_txt);
  int GetWidth() const {return surf->w;};
  int GetHeight() const {return surf->h;}
};

#endif
