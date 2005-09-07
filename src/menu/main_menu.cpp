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

#include "main_menu.h"
//-----------------------------------------------------------------------------
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <string.h>
#include "../include/app.h"
#include "../include/constant.h" // VERSION
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../sound/jukebox.h"
#include "../graphic/font.h"

#ifdef CL
#include "../graphic/graphism.h"
#endif

#ifndef WIN32
#include <dirent.h>
#endif

using namespace Wormux;
using namespace std;
//-----------------------------------------------------------------------------

// Position du texte de la version
const int VERSION_DY = -40;

const int DEFAULT_SCREEN_WIDTH = 800 ;
const int DEFAULT_SCREEN_HEIGHT = 600 ;
const int BUTTON_WIDTH = 282 ;  // Button width at default screen resolution
const int BUTTON_HEIGHT = 57 ;  // Button height at default screen resolution

//-----------------------------------------------------------------------------
Menu menu;
//-----------------------------------------------------------------------------

Menu::Menu()
{
  background=NULL;
}

//-----------------------------------------------------------------------------
Menu::~Menu()
{
  SDL_FreeSurface(background);
}

//-----------------------------------------------------------------------------
bool sig_play()
{ menu.choice=menuPLAY;printf("\nsig %i",menu.choice);return true; }
bool sig_network()
{ menu.choice=menuNETWORK;printf("\nsig %i",menu.choice);return true; }
bool sig_options()
{ menu.choice=menuOPTIONS;printf("\nsig %i",menu.choice);return true; }
bool sig_infos()
{ menu.choice=menuINFOS;printf("\nsig %i",menu.choice);return true; }
bool sig_quit()
{ menu.choice=menuQUIT;printf("\nsig %i",menu.choice);return true; }
//-----------------------------------------------------------------------------

void Menu::Init()
{
  int x_button, button_width, button_height ;
  double y_scale ;

#ifdef CL
  background = CL_Surface("intro/fond", graphisme.LitRes());
#else
  background = IMG_Load("../data/menu/img/background.png");
#endif

#ifdef CL
  x_button = (int)((double)474 / DEFAULT_SCREEN_WIDTH * video.GetWidth()) ;
  y_scale = (double)1 / DEFAULT_SCREEN_HEIGHT * video.GetHeight() ;

  button_width = (int)((double)BUTTON_WIDTH / DEFAULT_SCREEN_WIDTH * video.GetWidth()) ;
  button_height = (int)((double)BUTTON_HEIGHT / DEFAULT_SCREEN_HEIGHT * video.GetHeight()) ;
#else
  x_button = (int)((double)474 / DEFAULT_SCREEN_WIDTH * 640) ;
  y_scale = (double)1 / DEFAULT_SCREEN_HEIGHT * 480 ;

  button_width = (int)((double)BUTTON_WIDTH / DEFAULT_SCREEN_WIDTH * 640) ;
  button_height = (int)((double)BUTTON_HEIGHT / DEFAULT_SCREEN_HEIGHT * 480) ;
#endif

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

menu_item Menu::Run ()
{
  string txt_version;
  int x,y;
  txt_version = string("Version ") + string(VERSION);
  SDL_Event event;
  
  play->Show();
  network->Show();
  options->Show();
  infos->Show();
  quit->Show();
  
  choice = menuNULL;
  while (choice == menuNULL)
  {
    SDL_BlitSurface(background,NULL,app.sdlwindow,NULL);

#ifdef CL
    big_font.WriteCenter (video.GetWidth()/2,
                           video.GetHeight()+VERSION_DY,
                           txt_version);
#endif

    while ( SDL_PollEvent(&event) ) {
      app.PumpIntoEventQueue(&event);
    }
    printf("\n%i",choice);
    PG_Widget::BulkBlit();
    SDL_Flip(app.sdlwindow);
  }
#ifdef CL
  if (choice != menuQUIT) jukebox.Play("menu/ok");
#endif
printf("\nsortie....");fflush(stdout);
  return choice;
}

//-----------------------------------------------------------------------------
