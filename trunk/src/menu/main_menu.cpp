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
 * Main_Menu du jeu permettant de lancer une partie, modifier les options, d'obtenir
 * des informations, ou encore quitter le jeu.
 *****************************************************************************/

#include "main_menu.h"
#include <string>
#include "../game/config.h"
#include "../game/time.h"
#include "../graphic/effects.h"
#include "../graphic/font.h"
#include "../graphic/fps.h"
#include "../include/app.h"
#include "../include/constant.h"
#include "../sound/jukebox.h"
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../tool/resource_manager.h"
#include <iostream>

#define NETWORK_BUTTON 

#ifndef WIN32
#include <dirent.h>
#endif

// Position du texte de la version
const int VERSION_DY = -40;

const int DEFAULT_SCREEN_HEIGHT = 768 ;

Main_Menu::~Main_Menu(){
  delete version_text;
  delete website_text;
}

Main_Menu::Main_Menu() :
    Menu("main_menu/bg_main", vNo)
{
  int x_button;
  double y_scale;

  normal_font = Font::GetInstance(Font::FONT_NORMAL);
  large_font = Font::GetInstance(Font::FONT_LARGE);

  int button_width = 402;

  y_scale = (double)AppWormux::GetInstance()->video.window.GetHeight() / DEFAULT_SCREEN_HEIGHT ;

  x_button = AppWormux::GetInstance()->video.window.GetWidth()/2 - button_width/2;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);

  s_skin_left = resource_manager.LoadImage(res,"main_menu/skin_1");
  skin_left = new PictureWidget(Rectanglei(0, AppWormux::GetInstance()->video.window.GetHeight() - s_skin_left.GetHeight(), 269, 427));
  skin_left->SetSurface(s_skin_left);

  s_skin_right = resource_manager.LoadImage(res,"main_menu/skin_2");
  skin_right = new PictureWidget(Rectanglei(AppWormux::GetInstance()->video.window.GetWidth()  - s_skin_right.GetWidth(),
                                            AppWormux::GetInstance()->video.window.GetHeight() - s_skin_right.GetHeight(), 380, 329));
  skin_right->SetSurface(s_skin_right);

  s_title = resource_manager.LoadImage(res,"main_menu/title");
  title = new PictureWidget(Rectanglei(AppWormux::GetInstance()->video.window.GetWidth()/2  - s_title.GetWidth()/2 + 10, 0, 648, 168));
  title->SetSurface(s_title);

  int y = 300;
  const int y2 = 580;
#ifdef NETWORK_BUTTON  
  int dy = (y2-y)/4;
#else  
  int dy = (y2-y)/3;
#endif  

  play = new ButtonText( Point2i(x_button, (int)(y * y_scale)),
			res, "main_menu/button",
			_("Play"),
			large_font);
  y += dy;

#ifdef NETWORK_BUTTON  
  network = new ButtonText( Point2i(x_button, (int)(y * y_scale)),
			   res, "main_menu/button",
			   _("Network Game"),
			   large_font );
  y += dy;
#else
  network = NULL;
#endif
  
  options = new ButtonText( Point2i(x_button, (int)(y * y_scale) ),
			   res, "main_menu/button",
			   _("Options"),
			   large_font);
  y += dy;

  infos =  new ButtonText( Point2i(x_button, (int)(y * y_scale) ),
			  res, "main_menu/button",
			  _("Credits"),
			  large_font);
  y += dy;

  quit =  new ButtonText( Point2i(x_button,(int)(y * y_scale) ),
			 res, "main_menu/button",
			 _("Quit"),
			 large_font);

  widgets.AddWidget(skin_right);
  widgets.AddWidget(skin_left);
  widgets.AddWidget(play);
#ifdef NETWORK_BUTTON 
  widgets.AddWidget(network);
#endif
  widgets.AddWidget(options);
  widgets.AddWidget(infos);
  widgets.AddWidget(quit);
  widgets.AddWidget(title);

  resource_manager.UnLoadXMLProfile( res);

  std::string s("Version "+Constants::VERSION);
  version_text = new Text(s, green_color, normal_font, false);

  std::string s2(Constants::WEB_SITE);
  website_text = new Text(s2, green_color, normal_font, false);
}

void Main_Menu::button_clic()
{
  jukebox.Play("share", "menu/clic");
}

void Main_Menu::OnClic(const Point2i &mousePosition, int button)
{
  Widget* b = widgets.Clic(mousePosition,button);
  if(b == play)
  {
    choice = menuPLAY;
    close_menu = true;
    button_clic();
  }
#ifdef NETWORK_BUTTON  
  else if(b == network)
  {
    choice = menuNETWORK;
    close_menu = true;
    button_clic();
  }
#endif  
  else if(b == options)
  {
    choice = menuOPTIONS;
    close_menu = true;
    button_clic();
  }
  else if(b == infos)
  {
    choice = menuCREDITS;
    close_menu = true;
    button_clic();
  }
  else if(b == quit)
  {
    choice = menuQUIT;
    close_menu = true;
    button_clic();
  }
}

menu_item Main_Menu::Run ()
{
  choice = menuNULL;

  Menu::Run();

  assert( choice != menuNULL );
  return choice;
}

void Main_Menu::key_ok()
{
  choice = menuPLAY;
  close_menu = true;
}

void Main_Menu::key_cancel()
{
  choice = menuQUIT;
  close_menu = true;
}
