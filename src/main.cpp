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
 * Application Wormux lancant le programme (fonction 'main').
 *****************************************************************************/

#include "include/app.h"
//-----------------------------------------------------------------------------
#define EMAIL "wormux-dev@gna.org"
//-----------------------------------------------------------------------------
#ifdef CL
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include "graphic/graphism.h"
#include "network/network.h"
#include "graphic/video.h"
#include "map/wind.h"
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <algorithm>
#include <exception>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include "include/action_handler.h"
#include "game/game.h"
#include "game/config.h"
#include "menu/options_menu.h"
#include "graphic/font.h"
#include "include/constant.h"
#include "sound/jukebox.h"
#include "tool/i18n.h"
#include "tool/random.h"
#include "game/config.h"
#include "menu/main_menu.h"
#include "menu/infos_menu.h"
#include "sound/jukebox.h"


using namespace Wormux;
//-----------------------------------------------------------------------------
#define MSG_CHARGEMENT_X (config.ecran.larg/2)
#define MSG_CHARGEMENT_Y (config.ecran.haut/2)
//-----------------------------------------------------------------------------
AppWormux app;
//-----------------------------------------------------------------------------

AppWormux::AppWormux()
{
}

//-----------------------------------------------------------------------------

void AppWormux::WelcomeMessage()
{
  std::cout << "=== " << _("Wormux version ") << VERSION << std::endl;
  std::cout << "=== " << _("Authors:") << ' ';
  for (std::vector<std::string>::iterator it=AUTHORS.begin(),
	 fin=AUTHORS.end();
       it != fin;
       ++it)
  {
    if (it != AUTHORS.begin()) std::cout << ", ";
    std::cout << *it;
  }
  std::cout << std::endl
	    << "=== " << _("Website: ") << WEB_SITE << std::endl
	    << std::endl;

  // Affiche l'absence de garantie sur le jeu
  std::cout << "Wormux version " << VERSION
	    << ", Copyright (C) 2001-2004 Lawrence Azzoug"
	    << std::endl
	    << "Wormux comes with ABSOLUTELY NO WARRANTY. This is free "
	    << "software, and you are welcome to redistribute it under "
	    << "certain conditions." << std::endl
	    << std::endl
	    << "Read COPYING.txt for details." << std::endl
	    << std::endl;

#ifdef DEBUG
  std::cout << "!!! This program was compiled in DEBUG mode (development"
	    << std::endl
	    << "!!! version)." << std::endl
	    << std::endl;
#endif

  std::cout << "[ " << _("Run game") << " ]" << std::endl;
}

//-----------------------------------------------------------------------------

void AppWormux::Prepare()
{
  InitCst();
  InitI18N();
  WelcomeMessage();
  InitRandom();
  action_handler.Init();
  config.Charge();
}

//-----------------------------------------------------------------------------

bool AppWormux::Init(int argc, char **argv)
{
  // Network
#ifdef CL
  if ((argc == 3) && (strcmp(argv[1],"server")==0)) {
	// wormux server <port>
	network.server_start (argv[2]);
  } else if (argc == 3) {
	// wormux <server_ip> <server_port>
	network.client_connect(argv[1], argv[2]);
  }
#endif

  // Screen initialisation
  if(!InitScreen(config.tmp.video.width,
                 config.tmp.video.height,
                 32, //resolution in bpp
                 SDL_HWSURFACE))
  {
    std::cerr << "Unable to initialize SDL/ParaGUI: %s\n" << SDL_GetError() << std::endl;
    return false;
  }
  LoadTheme("default");

  // Set window caption
  std::string txt_version;
  txt_version = std::string("Wormux ") + std::string(VERSION);
  SetCaption(txt_version.c_str(), NULL);

  
  // Fonts initialisation
  if (TTF_Init()==-1) {
    std::cerr << "TTF_Init: "<< TTF_GetError() << std::endl;
    return false;
  }
  Font::InitAllFonts();

   // Full screen ?
  sdlwindow=GetScreen();

  if (config.tmp.video.fullscreen) {
    SDL_WM_ToggleFullScreen(sdlwindow);
  }

  // Load graphics resources XML file
#ifdef CL
  graphisme.Init();
#endif

  // Display a loading picture
#ifdef CL
  CL_Surface loading_image = CL_Surface("intro/loading", graphisme.LitRes());
#else
  //TODO->use ressource handler
  SDL_Surface* loading_image=IMG_Load( (config.data_dir+"/menu/img/loading.png").c_str());
#endif

  SDL_BlitSurface(loading_image,NULL,sdlwindow,NULL);

  txt_version = _("Version") + std::string(VERSION);

  huge_font.WriteCenter( config.tmp.video.width/2, 
			config.tmp.video.height/2 - 200, 
			_("Wormux launching..."), //"Chargement",
  			white_color);

  huge_font.WriteCenter( config.tmp.video.width/2, 
			 config.tmp.video.height/2 - 180 + huge_font.GetHeight(), 
			 txt_version,
			 white_color);
  
  SDL_UpdateRect(sdlwindow, 0, 0, 0, 0);
  SDL_Flip(sdlwindow);
  SDL_FreeSurface(loading_image);

  config.Applique();

  // Sound initialisation
  jukebox.Init();  
  
  return true;
}

//-----------------------------------------------------------------------------

void AppWormux::Fin()
{
  // Message de fin
  std::cout << std::endl
	    << "[ " << _("End of game") << " ]" << std::endl;
  
#ifdef CL
  config.Sauve();
#endif

  jukebox.End();
  SDL_Quit();

  std::cout << "o "
	    << _("Please tell us your opinion of Wormux via email:")
	    << std::endl
	    << "  " << EMAIL << std::endl;
}

//-----------------------------------------------------------------------------

int AppWormux::main (int argc, char **argv)
{
  menu_item choix;
  bool quitter = false;
  try
  {
    Prepare();
    if (!Init(argc, argv))
    {
      std::cout << std::endl << "Error during initialisation...";
      return 0;
    }
    do
    {
      main_menu.Init();
      choix = main_menu.Run();
      main_menu.FreeMem();

      switch (choix)
      {
        case menuPLAY:
	 jeu.LanceJeu();
	 break;
       case menuOPTIONS:
	 options_menu.Lance();
	 break;
        case menuQUIT:
                         quitter = true;
                         break;
        default:
                         break;
       }
     } while (!quitter);

    Fin();
  }
  
  catch (const std::exception &e)
  {
    std::cerr << std::endl
	      << _("C++ exception caught:") << std::endl
	      << e.what() << std::endl
	      << std::endl;
  }
  catch (std::string &e)
  {
     std::cerr << e << std::endl;
  } 
  catch (...)
  {
    std::cerr << std::endl
	      << _("Unexcepted exception caught...") << std::endl
	      << std::endl;
  }
  return 0;
}

int main (int argc, char **argv)
{
  app.main(argc,argv);
}
//-----------------------------------------------------------------------------
