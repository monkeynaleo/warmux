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
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <algorithm>
#include <exception>
#include "menu/main_menu.h"
#include "menu/options_menu.h"
#include "tool/i18n.h"
#include "menu/infos_menu.h"
#include "game/game.h"
#include "graphic/graphism.h"
#include "include/constant.h"
#include "include/action_handler.h"
#include "tool/random.h"
#include "game/config.h"
#include "network/network.h"
#include "graphic/video.h"
#include "sound/jukebox.h"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

#include "map/wind.h"

using namespace Wormux;
//-----------------------------------------------------------------------------
#define MSG_CHARGEMENT_X (config.ecran.larg/2)
#define MSG_CHARGEMENT_Y (config.ecran.haut/2)
//-----------------------------------------------------------------------------
AppWormux app;
//-----------------------------------------------------------------------------

AppWormux::AppWormux()
{
  clwindow = NULL;
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
  InitRandom();
  WelcomeMessage();
  action_handler.Init();
  config.Charge();
}

//-----------------------------------------------------------------------------

void AppWormux::Init(int argc, char **argv)
{
  // Network
  if ((argc == 3) && (strcmp(argv[1],"server")==0)) {
	// wormux server <port>
	network.server_start (argv[2]);
  } else if (argc == 3) {
	// wormux <server_ip> <server_port>
	network.client_connect(argv[1], argv[2]);
  }

#ifdef USE_SDL
  if (config.use_sdl) {
    setup_gl = NULL;
    setup_sdl = new CL_SetupSDL();
  } else {
    setup_gl = new CL_SetupGL();
    setup_sdl = NULL;
  }
#else
  setup_gl = new CL_SetupGL();
#endif

  // Open a new window
  clwindow = new CL_DisplayWindow(std::string("Wormux ")+VERSION,
				config.tmp.video.width,
				config.tmp.video.height,
				config.tmp.video.fullscreen);
  CL_Display::clear (CL_Color::black);

  // Load graphics resources XML file
  graphisme.Init();

  // Display a loading picture
  CL_Surface loading_image = CL_Surface("intro/loading", graphisme.LitRes());
  loading_image.draw (CL_Rect(0, 0, video.GetWidth(), video.GetHeight()));

  // Message in window
  std::ostringstream ss;
  ss << _("Wormux launching...") << std::endl;
  ss << _("Version") << " " << VERSION;
  police_grand.WriteCenter (video.GetWidth()/2, 200*video.GetHeight()/loading_image.get_height(), ss.str());
  CL_Display::flip();

  // Charge le son
  jukebox.Init();

  config.Applique();
}

//-----------------------------------------------------------------------------

void AppWormux::Fin()
{
  // Message de fin
  std::cout << std::endl
	    << "[ " << _("End of game") << " ]" << std::endl;
  config.Sauve();

  jukebox.End();
  delete clwindow;
  CL_SetupDisplay::deinit();
  CL_SetupCore::deinit();

  delete setup_gl;
#ifdef USE_SDL
  delete setup_sdl;
#endif

  std::cout << "o "
	    << _("Please tell us your opinion of Wormux via email:")
	    << std::endl
	    << "  " << EMAIL << std::endl;
}

//-----------------------------------------------------------------------------

int AppWormux::main (int argc, char **argv)
{
#ifdef WIN32
  // Create a console window for text-output if not available
  CL_ConsoleWindow console("Console",80,1000);
  console.redirect_stdio();
#endif
  menu_item choix;
  bool quitter = false;
  try
  {
    Prepare();
    CL_SetupCore setupCore;
    CL_SetupDisplay setupDisplay;
    CL_SetupNetwork setupNetwork;
    Init(argc, argv);
    do
    {
      Menu *menu = new Menu;
      menu->ChargeImage();
      choix = menu->Lance();

      switch (choix)
      {
        case menuJOUER:   delete menu;
		          jeu.LanceJeu();
	                  break;
        case menuOPTIONS: delete menu;
			  menu_option.Lance();
	                  break;
        case menuINFOS:   delete menu;
			  menu_infos.Lance();
	                  break;
        case menuQUITTER: delete menu;
		          quitter = true;
	                  break;
        default:          ;
      }
      
    } while (!quitter);

    Fin();
  }
  catch (const CL_Error &err)
  {
    std::cerr << std::endl
	      << _("ClanLib error :") << std::endl
	      << err.message << std::endl
	      << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cerr << std::endl
	      << _("C++ exception caught:") << std::endl
	      << e.what() << std::endl
	      << std::endl;
  }
  catch (...)
  {
    std::cerr << std::endl
	      << _("Unexcepted exception caught...") << std::endl
	      << std::endl;
  }
  return 0;
}

//-----------------------------------------------------------------------------
