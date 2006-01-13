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

// Position du texte de la version
const int VERSION_DY = -40;

const int DEFAULT_SCREEN_WIDTH = 800 ;
const int DEFAULT_SCREEN_HEIGHT = 600 ;
const int BUTTON_WIDTH = 282 ;  // Button width at default screen resolution
const int BUTTON_HEIGHT = 57 ;  // Button height at default screen resolution

//-----------------------------------------------------------------------------

Main_Menu::~Main_Menu()
{
  delete background;
  delete play;
  delete network;
  delete options;
  delete infos;
  delete quit;

  delete text;
}

//-----------------------------------------------------------------------------
void Main_Menu::button_click()
{ jukebox.Play("share", "menu/clic"); }

bool Main_Menu::sig_play()
{ button_click(); choice=menuPLAY;return true; }
bool Main_Menu::sig_network()
{ button_click(); choice=menuNETWORK;return true; }
bool Main_Menu::sig_options()
{ button_click(); choice=menuOPTIONS;return true; }
bool Main_Menu::sig_infos()
{ button_click(); menu_infos.Run();return true; }
bool Main_Menu::sig_quit()
{ choice=menuQUIT;return true; }
//-----------------------------------------------------------------------------

Main_Menu::Main_Menu()
{
  int x_button, button_width, button_height ;
  double x_scale, y_scale ;

//  app.SetBackground("../data/menu/img/background.png",BKMODE_STRETCH); -->doesn't work with relative path
 background=new Sprite(IMG_Load((config.data_dir+"menu/img/background.png").c_str()));
 background->Blit( app.sdlwindow, 0, 0);

  x_scale = (double)app.sdlwindow->w / DEFAULT_SCREEN_WIDTH ;
  y_scale = (double)app.sdlwindow->h / DEFAULT_SCREEN_HEIGHT ;

  x_button = (int)(474 * x_scale);
  button_width = (int)(BUTTON_WIDTH * x_scale); 
  button_height = (int)(BUTTON_HEIGHT * y_scale);

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);

  play = new ButtonText(x_button,(uint)(192 * y_scale),//Position
			button_width,button_height, //Size
			res, "intro/txt_button",
			_("Play"),
			&large_font);

  network = new ButtonText(x_button,(int)(261 * y_scale), //Position
			   button_width,button_height, //Size
			   res, "intro/txt_button",
			   _("Network Game"),
			   &large_font );

  options = new ButtonText(x_button,(int)(329 * y_scale), //Position
			   button_width,button_height, //Size
			   res, "intro/txt_button",
			   _("Options"),
			   &large_font);

  infos =  new ButtonText(x_button,(int)(397 * y_scale), //Position
			  button_width,button_height, //Size
			  res, "intro/txt_button",
			  _("Info"),
			  &large_font);

  quit =  new ButtonText(x_button,(int)(465 * y_scale), //Position
			 button_width,button_height, //Size
			 res, "intro/txt_button",
			 _("Quit"),
			 &large_font);

  resource_manager.UnLoadXMLProfile( res);

  std::string s("Version "+VERSION);
  text = new Text(s, white_color, &big_font);
}

//-----------------------------------------------------------------------------
void Main_Menu::onClick ( int x, int y, int button)
{       
  if (play->MouseIsOver (x, y)) sig_play();
  else if (network->MouseIsOver (x, y)) sig_network();
  else if (options->MouseIsOver (x, y)) sig_options();
  //else if (infos->MouseIsOver (x, y)) sig_infos();
  else if (quit->MouseIsOver (x, y)) sig_quit();
}

//-----------------------------------------------------------------------------
menu_item Main_Menu::Run ()
{
  int x=0, y=0;

  SDL_Flip( app.sdlwindow);
 
  choice = menuNULL;
  while (choice == menuNULL)
  {
    
    // Poll and treat events
   SDL_Event event;
     
   while( SDL_PollEvent( &event) ) {      
       if ( event.type == SDL_MOUSEBUTTONDOWN ) {
           onClick( event.button.x, event.button.y, event.button.button);
       } else if ( event.type == SDL_KEYDOWN )  {
	     if ( event.key.keysym.sym == SDLK_ESCAPE)
         {
             choice = menuQUIT;
             break;
         }
         
       } else if ( event.type == SDL_QUIT)  {
           choice = menuQUIT;
           break;
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

   text->DrawCenter( app.sdlwindow->w/2,
		     app.sdlwindow->h+VERSION_DY);
   
   SDL_Flip(app.sdlwindow);
  }
  
  return choice;
}

//-----------------------------------------------------------------------------
