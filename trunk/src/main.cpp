/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 *  Starting file. (the 'main' function is here.)
 *****************************************************************************/

#include "include/app.h"
#include <algorithm>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <getopt.h>
#ifndef WIN32
#include <signal.h>
#endif
using namespace std;

#include <SDL.h>
#include "game/config.h"
#include "game/time.h"
#include "graphic/sprite.h"
#include "graphic/font.h"
#include "graphic/video.h"
#include "graphic/text.h"
#include "include/action_handler.h"
#include "include/constant.h"
#include "map/maps_list.h"
#include "menu/credits_menu.h"
#include "menu/game_menu.h"
#include "menu/main_menu.h"
#include "menu/network_connection_menu.h"
#include "menu/network_menu.h"
#include "menu/options_menu.h"
#include "network/download.h"
#include "sound/jukebox.h"
#include "team/team_config.h"
#include "team/teams_list.h"
#include "tool/debug.h"
#include "tool/i18n.h"
#include "tool/random.h"
#include "tool/stats.h"


static menu_item choice = menuNULL;
static bool skip_menu = false;
static NetworkConnectionMenu::network_menu_action_t net_action = NetworkConnectionMenu::NET_BROWSE_INTERNET;

AppWormux *AppWormux::singleton = NULL;

AppWormux *AppWormux::GetInstance()
{
  if (singleton == NULL)
    {
      singleton = new AppWormux();
    }
  return singleton;
}

AppWormux::AppWormux():
  video(NULL)
{
}

AppWormux::~AppWormux()
{
  if (video)
    delete video;
}

int AppWormux::Main(int argc, char *argv[])
{
  bool quit = false;

  try
  {
    ParseArgs(argc, argv);
    Init();
    do
      {
        MainMenu main_menu;

        if (choice == menuNULL) {
          StatStart("Main:Menu");
          choice = main_menu.Run();
          StatStop("Main:Menu");
        }

        ActionHandler::GetInstance()->Flush();

        switch (choice)
          {
          case menuPLAY:
            {
              GameMenu game_menu;
              game_menu.Run(skip_menu);
              break;
            }
          case menuNETWORK:
            {
              NetworkConnectionMenu network_connection_menu;
              network_connection_menu.SetAction(net_action);
              network_connection_menu.Run(skip_menu);
              break;
            }
          case menuOPTIONS:
            {
              OptionMenu options_menu;
              options_menu.Run();
              break;
            }
          case menuCREDITS:
            {
              CreditsMenu credits_menu;
              credits_menu.Run();
              break;
            }
          case menuQUIT:
            quit = true;
          default:
            break;
          }
        choice = menuNULL;
        skip_menu = false;
        net_action = NetworkConnectionMenu::NET_BROWSE_INTERNET;
      }
    while (!quit);

    End();
  }
  catch(const exception & e)
  {
    cerr << endl
      << "C++ exception caught:" << endl
      << e.what() << endl << endl;
    WakeUpDebugger();
  }
  catch(...)
  {
    cerr << endl
      << "Unexpected exception caught..." << endl << endl;
    WakeUpDebugger();
  }

  return 0;
}

void AppWormux::Init()
{
  video = new Video();

#ifndef WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

  Config::GetInstance();  // init config first, because it initializes i18n

  InitFonts();
  DisplayWelcomeMessage();

  teams_list.LoadList();

  DisplayLoadingPicture();

  jukebox.Init();

  cout << "[ " << _("Run game") << " ]" << endl;
}

void AppWormux::ParseArgs(int argc, char * argv[]) const
{
  char c;
  int option_index = 0;
  struct option long_options[] =
    {
      {"help",    no_argument,       NULL, 'h'},
      {"version", no_argument,       NULL, 'v'},
      {"play",    no_argument,       NULL, 'p'},
      {"internet",no_argument,       NULL, 'i'},
      {"client",  optional_argument, NULL, 'c'},
      {"server",  no_argument,       NULL, 's'},
      {"debug",   required_argument, NULL, 'd'},
      {NULL,      no_argument,       NULL,  0 }
    };

  while ((c = getopt_long (argc, argv, "hvpic::sd:",
                           long_options, &option_index)) != -1)
    {
      switch (c)
        {
        case 'h':
          printf("usage: %s [-h|--help] [-v|--version] [-p|--play]"
                 " [-i|--internet] [-s|--server] [-c|--client [ip]]"
                 " [-d|--debug debug_masks]\n", argv[0]);
          exit(0);
          break;
        case 'v':
          DisplayWelcomeMessage();
          exit(0);
          break;
        case 'p':
          choice = menuPLAY;
          skip_menu = true;
          break;
        case 'c':
          choice = menuNETWORK;
          net_action = NetworkConnectionMenu::NET_CONNECT_LOCAL;
          if (optarg)
            {
              Config::GetInstance()->SetNetworkHost(optarg);
            }
          skip_menu = true;
          break;
        case 'd':
          printf("Debug: %s\n", optarg);
          AddDebugMode(optarg);
          break;
        case 's':
          choice = menuNETWORK;
          net_action = NetworkConnectionMenu::NET_HOST;
          skip_menu = true;
          break;
        case 'i':
          choice = menuNETWORK;
          net_action = NetworkConnectionMenu::NET_BROWSE_INTERNET;
          skip_menu = true;
          break;
        }
    }
}

void AppWormux::DisplayLoadingPicture()
{
  Config *config = Config::GetInstance();

  string txt_version =
    _("Version") + string(" ") + Constants::VERSION;
  string filename = config->GetDataDir() + PATH_SEPARATOR + "menu"
                         + PATH_SEPARATOR + "loading.png";

  Surface surfaceLoading = Surface(filename.c_str());
  Sprite loading_image = Sprite(surfaceLoading);

  loading_image.cache.EnableLastFrameCache();
  loading_image.ScaleSize(video->window.GetSize());
  loading_image.Blit(video->window, 0, 0);

  Time::GetInstance()->Reset();

  Text text1(_("Wormux launching..."), white_color,
             Font::FONT_HUGE, Font::FONT_NORMAL, true);
  Text text2(txt_version, white_color, Font::FONT_HUGE, Font::FONT_NORMAL,
             true);

  Point2i windowCenter = video->window.GetSize() / 2;

  text1.DrawCenter(windowCenter);
  text2.DrawCenter(windowCenter
                   + Point2i(0, (*Font::GetInstance(Font::FONT_HUGE, Font::FONT_NORMAL)).GetHeight() + 20));

  video->window.Flip();
}

void AppWormux::InitFonts() const
{
  if (TTF_Init() == -1) {
    Error(Format("Initialisation of TTF library failed: %s", TTF_GetError()));
    exit(1);
  }
}

void AppWormux::End() const
{
  cout << endl << "[ " << _("Quit Wormux") << " ]" << endl;

  Config::GetInstance()->Save();
  jukebox.End();
  delete Config::GetInstance();
  delete Time::GetInstance();
  delete Constants::GetInstance();
  TTF_Quit();

#ifdef ENABLE_STATS
  SaveStatToXML("stats.xml");
#endif
  cout << "o " << _("If you found a bug or have a feature request "
                    "send us a email (in english, please):")
    << " " << Constants::EMAIL << endl;
}

void AppWormux::DisplayWelcomeMessage() const
{
  cout << "=== " << _("Wormux version ") << Constants::VERSION << endl;
  cout << "=== " << _("Authors:") << ' ';
  for (vector < string >::iterator it = Constants::AUTHORS.begin(),
       fin = Constants::AUTHORS.end(); it != fin; ++it)
    {
      if (it != Constants::AUTHORS.begin())
        cout << ", ";
      cout << *it;
    }
  cout << endl
    << "=== " << _("Website: ") << Constants::WEB_SITE << endl
    << endl;

  // Affiche l'absence de garantie sur le jeu
  cout << "Wormux version " << Constants::VERSION
    << ", Copyright (C) 2001-2006 Wormux Team" << endl
    << "Wormux comes with ABSOLUTELY NO WARRANTY." << endl
    << "This is free software, and you are welcome to redistribute it" << endl
    << "under certain conditions." << endl << endl
    << "Read COPYING file for details." << endl << endl;

#ifdef DEBUG
  cout << "This program was compiled in DEBUG mode (development version)"
       << endl << endl;
#endif
}

int main(int argc, char *argv[])
{
  AppWormux::GetInstance()->Main(argc, argv);
  delete AppWormux::GetInstance();
  exit(EXIT_SUCCESS);
}
