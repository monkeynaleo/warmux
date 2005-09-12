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
//-----------------------------------------------------------------------------
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <string>
#include "../include/app.h"
#include "../include/constant.h" // VERSION
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../sound/jukebox.h"
#include "../graphic/font.h"
#include "infos_menu.h"

#ifndef WIN32
#include <dirent.h>
#endif

using namespace Wormux;
using namespace std;
//-----------------------------------------------------------------------------
Main_Menu main_menu;
//-----------------------------------------------------------------------------

// Position du texte de la version
const int VERSION_DY = -40;

const int DEFAULT_SCREEN_WIDTH = 800 ;
const int DEFAULT_SCREEN_HEIGHT = 600 ;
const int BUTTON_WIDTH = 282 ;  // Button width at default screen resolution
const int BUTTON_HEIGHT = 57 ;  // Button height at default screen resolution

//-----------------------------------------------------------------------------

Main_Menu::Main_Menu()
{
}

//-----------------------------------------------------------------------------
void Main_Menu::FreeMem()
{
  app.DeleteBackground();
  SDL_FreeSurface(background);
  delete play;
  delete network;
  delete options;
  delete infos;
  delete quit;
}

//-----------------------------------------------------------------------------
void button_click()
{ jukebox.Play("share", "menu/clic"); }

bool sig_play()
{ button_click(); main_menu.choice=menuPLAY;return true; }
bool sig_network()
{ button_click(); main_menu.choice=menuNETWORK;return true; }
bool sig_options()
{ button_click(); main_menu.choice=menuOPTIONS;return true; }
bool sig_infos()
{ button_click(); menu_infos.Run();return true; }
bool sig_quit()
{ /*button_click()*/; main_menu.choice=menuQUIT;return true; }
//-----------------------------------------------------------------------------

void Main_Menu::Init()
{
  int x_button, button_width, button_height ;
  double y_scale ;

#ifdef CL
  background = CL_Surface("intro/fond", graphisme.LitRes());
#else
//  app.SetBackground("../data/menu/img/background.png",BKMODE_STRETCH); -->doesn't work with relative path
 background=IMG_Load("../data/menu/img/background.png");
#endif
  app.SetBackground(background,BKMODE_STRETCH);
  app.EnableBackground(true);

  x_button = (int)((double)474 / DEFAULT_SCREEN_WIDTH * app.sdlwindow->w) ;
  y_scale = (double)1 / DEFAULT_SCREEN_HEIGHT * app.sdlwindow->h ;

  button_width = (int)((double)BUTTON_WIDTH / DEFAULT_SCREEN_WIDTH * app.sdlwindow->w) ;
  button_height = (int)((double)BUTTON_HEIGHT / DEFAULT_SCREEN_HEIGHT * app.sdlwindow->h) ;

  play = new PG_Button(NULL,
                        PG_Rect(x_button,(int)(192 * y_scale), //Position
                                button_width,button_height), //Size
                        _("Play"));
  network = new PG_Button(NULL,
                        PG_Rect(x_button,(int)(261 * y_scale), //Position
                                button_width,button_height), //Size
                        _("Network Game"));
  options = new PG_Button(NULL,
                        PG_Rect(x_button,(int)(329 * y_scale), //Position
                                button_width,button_height), //Size
                        _("Options"));
  infos =  new PG_Button(NULL,
                        PG_Rect(x_button,(int)(397 * y_scale), //Position
                                button_width,button_height), //Size
                        _("Info"));
  quit =  new PG_Button(NULL,
                        PG_Rect(x_button,(int)(465 * y_scale), //Position
                                button_width,button_height), //Size
                        _("Quit"));
  play->sigClick.connect(slot(sig_play));
  network->sigClick.connect(slot(sig_network));
  options->sigClick.connect(slot(sig_options));
  infos->sigClick.connect(slot(sig_infos));
  quit->sigClick.connect(slot(sig_quit));
}

//-----------------------------------------------------------------------------

menu_item Main_Menu::Run ()
{
  string txt_version;
  txt_version = string("Version ") + string(VERSION);
  SDL_Event event;

  app.FlipPage();

  play->Show();
  network->Show();
  options->Show();
  infos->Show();
  quit->Show();
 
  choice = menuNULL;
  while (choice == menuNULL)
  {
    while ( SDL_PollEvent(&event) ) {
      app.PumpIntoEventQueue(&event);
    }

    //TODO:Use videomode
    app.RedrawBackground(PG_Rect(0,0,app.sdlwindow->w,app.sdlwindow->h));
    
    big_font.WriteCenter( app.sdlwindow->w/2,
 			  app.sdlwindow->h+VERSION_DY,
 			  txt_version, white_color);
    
    PG_Widget::BulkBlit();

    app.FlipPage();
  }
  
  return choice;
}

//-----------------------------------------------------------------------------
