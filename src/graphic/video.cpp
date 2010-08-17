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
 *  MERCHANTABILITY or FITNESS FOR A ARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU GeneralPublic License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *****************************************************************************/

#include <algorithm>
#include <iostream>
#include <SDL_image.h>

#include "game/config.h"
#include "graphic/video.h"
#include "include/app.h"
#include "include/constant.h"
#include "map/camera.h"


Video::Video()
{
  SetMaxFps (50);
  fullscreen = false;
  SDLReady = false;
  hardware = false;
  icon = NULL;

  InitSDL();

  window.SetSurface(NULL, false);
  window.SetAutoFree(false);

  Config * config = Config::GetInstance();
  SetMaxFps(config->GetMaxFps());

  ComputeAvailableConfigs();
  SetConfig((int)(config->GetVideoWidth()),
            (int)(config->GetVideoHeight()),
            config->IsVideoFullScreen());  // Add the current resolution

  if (window.IsNull()) {
    Error(Format("Unable to initialize SDL window: %s", SDL_GetError()));
    exit (1);
  }
  AddUniqueConfigSorted(window.GetWidth(), window.GetHeight());

  SetWindowCaption(std::string("Wormux ") + Constants::WORMUX_VERSION);
  // The icon must be larger then 32x32 pixels as some desktops display larger icons.
  // For example on a mac system the icon got displayed in a resolution of 64x64 pixels.
  // The even higher resolution allows the system to scale the icon down to an anti-aliased version.
#ifndef WIN32
  SetWindowIcon(config->GetDataDir() + "wormux_128x128.xpm");
#elif !defined(ANDROID)
  // The SDL manual of SDL_WM_SetIcon states that "Win32 icons must be 32x32.":
  SetWindowIcon(config->GetDataDir() + "wormux_32x32.xpm");
#endif
}

Video::~Video()
{
  if (icon)
    SDL_FreeSurface(icon);
  if (SDLReady)
    SDL_Quit();
  SDLReady = false;
}

void Video::SetMaxFps(uint max_fps)
{
  m_max_fps = max_fps;
  if (0 < m_max_fps)
    m_max_delay = 1000/m_max_fps;
  else
    m_max_delay = 0;
}

static bool CompareConfigs(const Point2i& a, const Point2i& b)
{
  return (a.x < b.x) || ((a.x == b.x) && (a.y < b.y));
}

void Video::AddUniqueConfigSorted(int w, int h)
{
  Point2i p(w, h);

  std::list<Point2i>::iterator res  = available_configs.begin(),
                               end  = available_configs.end();

  for (std::list<Point2i>::iterator res = available_configs.begin();
       res != available_configs.end();
       res++) {
    // Are they identical ?
    if (p == *res)
      return;

    // Is it bigger?
    if (!CompareConfigs(*res, p)) {
      available_configs.insert(res, p);
      return;
    }
  }
  available_configs.push_back(p);
}

void Video::ComputeAvailableConfigs()
{
  //Generate video mode list
  SDL_Rect **modes;

  // Get available fullscreen/hardware modes
  modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

  // Check is there are any modes available
  if (modes != NULL){
    // We also had the current window resolution if it is not already in the list!
    for (int i=0;modes[i];++i) {
      if (modes[i]->w>=480 && modes[i]->h>=320) {
        AddUniqueConfigSorted(modes[i]->w, modes[i]->h);
      }
    }
  }

  // If biggest resolution is big enough, we propose standard resolutions
  // such as 1600x1200, 1280x1024, 1024x768, 800x600.
  for (std::list<Point2i>::iterator res = Config::GetInstance()->GetResolutionAvailable().begin();
       res != Config::GetInstance()->GetResolutionAvailable().end();
       res++) {
    AddUniqueConfigSorted((*res).GetX(), (*res).GetY());
  }
}

bool Video::__SetConfig(const int width, const int height, const bool _fullscreen, const bool _hardware)
{
  bool __fullscreen = _fullscreen;

#ifdef ANDROID
  window.SetSurface(SDL_SetVideoMode(width, height, 16, SDL_SWSURFACE));
#else
#  ifdef __APPLE__
  __fullscreen = false; // Never set fullscreen with OSX, as it's buggy
#  endif

  int flags = (__fullscreen) ? SDL_FULLSCREEN : 0;

  if (_hardware) {
    flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
  } else {
    flags |= SDL_SWSURFACE;
  }
  window.SetSurface(SDL_SetVideoMode(width, height, 32, flags));
#endif

  if (window.IsNull())
    return false;

  if (!_hardware)
      std::cerr << "WARNING: Video not using hardware acceleration!" << std::endl;

  fullscreen = __fullscreen;
  hardware = _hardware;

  return true;
}

bool Video::SetConfig(const int width, const int height, const bool _fullscreen)
{
  bool r;
  bool window_was_null = window.IsNull();

  if (!window_was_null
      && width == window.GetWidth()
      && height == window.GetHeight()
      && fullscreen == _fullscreen)
    return true; // nothing to change :-)

  int old_width, old_height;
  bool old_fullscreen, old_hw;
  if (window_was_null) {
    old_width = available_configs.begin()->GetX();
    old_height = available_configs.begin()->GetY();
    old_hw = true;
  } else {
    old_width = window.GetWidth();
    old_height = window.GetHeight();
    old_hw = hardware;
  }
  old_fullscreen = fullscreen;

  // Trying with hardware acceleration
  r = __SetConfig(width, height, _fullscreen, true);
  if (!r) {
    fprintf(stderr,
            "WARNING: Fail to initialize main window with the following configuration: %dx%d,\n"
            "fullscreen: %d, WITH hardware acceleration\n",
            old_width, old_height, _fullscreen);

    // Trying previous configuration
    if (! __SetConfig(old_width, old_height, old_fullscreen, old_hw)) {

      // previous configuration fails !?!

      // let's have another try without hw acceleration and without fullscreen
      if (! __SetConfig(old_width, old_height, false, false)) {
        Error(Format("ERROR: Fail to initialize main window with the following configuration: %dx%d, "
                     "no fullscreen, no hardware acceleration\n", old_width, old_height));
        exit(EXIT_FAILURE);
      }
    }
  }

  Camera::GetInstance()->SetSize(width, height);

  // refresh all the map when switching to higher resolution
  if (!window_was_null)
    AppWormux::GetInstance()->RefreshDisplay();

  return r;
}

void Video::ToggleFullscreen()
{
#ifndef WIN32
#  ifndef __APPLE__ // Prevent buggy fullscreen under OSX
  SDL_WM_ToggleFullScreen(window.GetSurface());
  fullscreen = !fullscreen;
#  endif
#else
  SetConfig(window.GetWidth(), window.GetHeight(), !fullscreen);
  AppWormux::GetInstance()->RefreshDisplay();
#endif
}

void Video::SetWindowCaption(const std::string& caption) const
{
  SDL_WM_SetCaption(caption.c_str(), NULL);
}

void Video::SetWindowIcon(const std::string& filename)
{
  icon = IMG_Load(filename.c_str());
  SDL_WM_SetIcon(icon, NULL);
}

void Video::InitSDL()
{
  if (SDLReady)
    return;

  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
    Error(Format("Unable to initialize SDL library: %s", SDL_GetError()));
    exit (EXIT_FAILURE);
  }

  SDL_EnableUNICODE(1);
  SDLReady = true;
}

void Video::Flip()
{
  window.Flip();
}

Surface& GetMainWindow()
{
  return AppWormux::GetInstance()->video->window;
}

void SwapWindowClip(Rectanglei& r)
{
  AppWormux::GetInstance()->video->window.SwapClipRect(r);
}