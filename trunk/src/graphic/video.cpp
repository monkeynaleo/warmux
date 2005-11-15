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
 *  MERCHANTABILITY or FITNESS FOR A ARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU GeneralPublic License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Refresh de la carte vidéo.
 *****************************************************************************/

#include "video.h"
//-----------------------------------------------------------------------------
#ifdef CL
# include <ClanLib/display.h>
#endif
#include <iostream>
#include <algorithm>
#include <sstream>
#include "../tool/i18n.h"
#include "../include/app.h"

//Diasbled -> Don't works because of clanlib bug..

//#define BUGGY_CODE
//-----------------------------------------------------------------------------
Video video;
//-----------------------------------------------------------------------------

const int WIDTH_MIN=800;
const int HEIGHT_MIN=600;

#ifdef BUGGY_CODE
Video::Video()
	:  m_mode(-1)
{
  SetMaxFps (50);
  const std::vector<CL_DisplayMode> modes=CL_DisplayMode::get_display_modes();
  ConstIteratorMode mode=modes.begin();
  ConstIteratorMode end=modes.end();

  for(;mode!=end;++mode)
    {
      CL_Size resolution=mode->get_resolution();
      if(resolution.width>=WIDTH_MIN && resolution.height>=HEIGHT_MIN)
	{
	  m_modes.push_back(*mode);
	}
    }
}
#else
Video::Video() 
{
  SetMaxFps (50);
  fullscreen = false;
}
#endif

#ifdef BUGGY_CODE

void Video::SetSize(int width, int height)
{
  bool fs=IsFullScreen();
  delete app.clwindow;
  CL_SetupDisplay::deinit();
//  app.setup_gl=new CL_SetupGL();
  app.clwindow=new CL_DisplayWindow(std::string("Wormux ")+VERSION,
				width,
				height,
				fs);
  //app.clwindow->set_size(width,height);
}

void Video::ChangeMode(int mode, bool fullScreen)
{
  assert(mode >= 0 && mode < static_cast<int>( m_modes.size() ) );

  CL_Size resolution=m_modes[mode].get_resolution();
  SetSize(resolution.width,resolution.height);

  m_mode=mode;

  SetFullScreen(fullScreen);
}

void Video::SetFullScreen(bool fullScreen)
{
  if(fullScreen) app.clwindow->set_fullscreen();
  else app.clwindow->set_windowed();
}

const std::vector<CL_DisplayMode>& Video::GetModes() const
{
  return m_modes;
}

int Video::GetCurrentMode(void)
{
  if(m_mode!=-1) return m_mode;

  m_mode=0; // Default Value
  for(int mode=0;mode<static_cast<int>(m_modes.size());++mode)
    {
      CL_Size resolution=m_modes[mode].get_resolution();
      if(resolution.width==GetWidth() && resolution.height==GetHeight())
	{
	  m_mode=mode;
	  break;
	}
    }

  return m_mode;
}
#endif

void Video::SetMaxFps(uint max_fps)
{
	m_max_fps = max_fps;
	if (0 < m_max_fps)
		m_sleep_max_fps = 1000/m_max_fps;
	else
		m_sleep_max_fps = 0;
}


uint Video::GetMaxFps()
{ return m_max_fps; }

uint Video::GetSleepMaxFps()
{ return m_sleep_max_fps; }

//-----------------------------------------------------------------------------


int Video::GetWidth(void) const
{
#ifdef CL
  return app.clwindow->get_width();
#else
  return app.sdlwindow->w;
#endif
}
//-----------------------------------------------------------------------------

int Video::GetHeight(void) const
{
#ifdef CL
  return app.clwindow->get_height();
#else
  return app.sdlwindow->h;
#endif
}
//-----------------------------------------------------------------------------

bool Video::IsFullScreen(void) const
{
#ifdef CL
  return app.clwindow->is_fullscreen();
#else
  return fullscreen;
#endif
}

//-----------------------------------------------------------------------------

bool Video::SetConfig(int width, int height, bool _fullscreen)
{
  // initialize the main window
  if ((app.sdlwindow == NULL) || 
      (width != app.sdlwindow->w || height != app.sdlwindow->h)) {
    app.sdlwindow = SDL_SetVideoMode(width,
				     height,
				     16, //resolution in bpp
				     SDL_HWSURFACE| SDL_HWACCEL |SDL_DOUBLEBUF);
    if (app.sdlwindow == NULL) 
      app.sdlwindow = SDL_SetVideoMode(width,
				       height,
				       16, //resolution in bpp
				       SDL_SWSURFACE);

    if (app.sdlwindow == NULL) return false;
  }

  // fullscreen ?
  if (fullscreen != _fullscreen) {
    SDL_WM_ToggleFullScreen(app.sdlwindow);
    fullscreen = _fullscreen;
  }
  return true;
}

//-----------------------------------------------------------------------------
