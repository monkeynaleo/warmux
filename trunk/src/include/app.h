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
 * Application Wormux.
 *****************************************************************************/

#ifndef APP_WORMUX_H
#define APP_WORMUX_H
//-----------------------------------------------------------------------------
#include "base.h"
#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <string>

#ifdef USE_SDL
#  include <ClanLib/sdl.h>
#endif

//-----------------------------------------------------------------------------

class AppWormux : public CL_ClanApplication
{
public:
  CL_DisplayWindow *clwindow;

private:
  int x, y;
  std::string titre_fenetre;
#ifdef USE_SDL
  CL_SetupSDL *setup_sdl;
#endif

private:
  void Prepare();
  void WelcomeMessage();
  void Init(int, char**);
  void Fin();

public:
	CL_SetupGL *setup_gl;
	AppWormux();
  int main( int argc, char ** argv );
};

extern AppWormux app;
//-----------------------------------------------------------------------------
#endif
