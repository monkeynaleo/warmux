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
 * Game menu from which one may start a new game, modify options, obtain some
 * infomations or leave the game.
 *****************************************************************************/

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "menu.h"

// Forward declarations
class Text;
class ButtonPic;

class MainMenu : public Menu
{
  /* If you need this, implement it (correctly) */
  MainMenu(const MainMenu&);
  MainMenu operator=(const MainMenu&);
  /**********************************************/

  ButtonPic *play, *options, *help, *credits, *quit;
#ifndef ANDROID
  ButtonPic *network;
#endif
  Text * version_text, *website_text;

public:
  typedef enum
  {
    NONE = 0,
    PLAY,
    NETWORK,
    OPTIONS,
    HELP,
    CREDITS,
    QUIT
  } menu_item;

  menu_item choice;

  MainMenu();
  ~MainMenu();

  void Init(void);
  menu_item Run();

protected:
   bool signal_ok();
   bool signal_cancel();
   void SelectAction(const Widget * widget);

private:
   virtual void DrawBackground();
   void OnClick(const Point2i &mousePosition, int button);
   void OnClickUp(const Point2i &mousePosition, int button);

  // Main drawing function: refresh parts of screen
   void Draw(const Point2i &/*mousePosition*/) {};
   void button_click() const;
};

#endif
