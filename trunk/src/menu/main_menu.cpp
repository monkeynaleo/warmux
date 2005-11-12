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
#include "../game/config.h"
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../tool/resource_manager.h"
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

const std::string VERSION("0.7beta");
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
  delete background;
  delete play;
  delete network;
  delete options;
  delete infos;
  delete quit;
}

//-----------------------------------------------------------------------------

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
 background=new Sprite(IMG_Load((config.data_dir+"menu/img/background.png").c_str()));
 background->Blit( app.sdlwindow, 0, 0);
#endif

  x_button = (int)((double)474 / DEFAULT_SCREEN_WIDTH * app.sdlwindow->w) ;
  y_scale = (double)1 / DEFAULT_SCREEN_HEIGHT * app.sdlwindow->h ;

  button_width = (int)((double)BUTTON_WIDTH / DEFAULT_SCREEN_WIDTH * app.sdlwindow->w) ;
  button_height = (int)((double)BUTTON_HEIGHT / DEFAULT_SCREEN_HEIGHT * app.sdlwindow->h) ;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");

  play = new ButtonText(x_button,(uint)(192 * y_scale),//Position
		    button_width,button_height, //Size
		    _("Play"));
  play->SetFont(&large_font);
  play->SetImage (res, "intro/jouer");//->SetImage (res, "intro/play");

  network = new ButtonText(x_button,(int)(261 * y_scale), //Position
		       button_width,button_height, //Size
		       _("Network Game"));
  network->SetFont(&large_font);
  network->SetImage (res,"intro/jouer");

  options = new ButtonText(x_button,(int)(329 * y_scale), //Position
		       button_width,button_height, //Size
		       _("Options"));
  options->SetFont(&large_font);
  options->SetImage (res,"intro/options");

  infos =  new ButtonText(x_button,(int)(397 * y_scale), //Position
		      button_width,button_height, //Size
		      _("Info"));
  infos->SetFont(&large_font);
  infos->SetImage (res,"intro/infos");

  quit =  new ButtonText(x_button,(int)(465 * y_scale), //Position
		     button_width,button_height, //Size
		     _("Quit"));
  quit->SetFont(&large_font);
  quit->SetImage (res,"intro/quitter");

  delete res;
}

//-----------------------------------------------------------------------------
void Main_Menu::onClick ( int x, int y)
{     
  if (play->Test (x, y)) sig_play();
  else if (network->Test (x, y)) sig_network();
  else if (options->Test (x, y)) sig_options();
  //else if (infos->Test (x, y)) sig_infos();
  else if (quit->Test (x, y)) sig_quit();
}

//-----------------------------------------------------------------------------
menu_item Main_Menu::Run ()
{
  string txt_version;
  txt_version = string("Version ") + string(VERSION);
  SDL_Event event;
  int x=0, y=0;

  SDL_Flip( app.sdlwindow);
 
  choice = menuNULL;
  while (choice == menuNULL)
  {
    
    // Poll and treat events
   SDL_Event event;
     
   while( SDL_PollEvent( &event) ) {      
     if ( event.type == SDL_MOUSEBUTTONDOWN )
       {
	 onClick( event.button.x, event.button.y);
       }
   }

   SDL_GetMouseState( &x, &y);
   
   background->ScaleSize(app.sdlwindow->w, app.sdlwindow->h);
   background->Blit( app.sdlwindow, 0, 0);

   play->Draw(x,y);
   network->Draw(x,y);
   options->Draw(x,y);
   infos->Draw(x,y);
   quit->Draw(x,y);

   
   big_font.WriteCenter( app.sdlwindow->w/2,
			 app.sdlwindow->h+VERSION_DY,
			 txt_version, white_color);
   
   SDL_Flip(app.sdlwindow);
  }
  
  return choice;
}

//-----------------------------------------------------------------------------
