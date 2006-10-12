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
 * Menu du jeu permettant de lancer une partie, modifier les options, d'obtenir
 * des informations, ou encore quitter le jeu.
 *****************************************************************************/

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "../include/base.h"
#include "../graphic/fps.h"
#include "../graphic/surface.h"
#include "../gui/widget_list.h"
#include "../gui/button_text.h"
#include "../gui/picture_widget.h"
#include "../menu/menu.h"
#include <SDL.h>
#include <vector>

typedef enum
{
  menuNULL=0,
  menuPLAY,
  menuNETWORK,
  menuOPTIONS,
  menuCREDITS,
  menuQUIT
} menu_item;

class Main_Menu : public Menu
{
  PictureWidget *title;
  Surface s_title;

  Sprite *skin_left, *skin_right;

  ButtonText *play, *network, *options, *infos, *quit;

  Text *version_text, *website_text;
 

public:
  menu_item choice;

  Main_Menu();
  ~Main_Menu();
  menu_item Run ();

  void Redraw(const Rectanglei& rect, Surface& surf);

protected:
   void __sig_ok();
   void __sig_cancel();
   void key_ok();
   void key_cancel();

private:
   virtual void DrawBackground(const Point2i &mousePosition);
   void OnClic(const Point2i &mousePosition, int button);

  // Main drawing function: refresh parts of screen 
  void Draw(const Point2i &mousePosition) {};
  void button_clic();
};

#endif
