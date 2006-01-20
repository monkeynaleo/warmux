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
 * Refresh du temps qui passe. Le temps du jeu peut être mise en pause.
 *****************************************************************************/

#include "time.h"
//-----------------------------------------------------------------------------
#include "../tool/math_tools.h"
#include "../interface/game_msg.h"

#include "../graphic/video.h"

#include <SDL.h>
#include <sstream>
#include <iomanip>
#include <iostream>
//-----------------------------------------------------------------------------

const double VITESSE_MIN = 0.01;
const double VITESSE_MAX = 50;

//-----------------------------------------------------------------------------
namespace Wormux
{

Time global_time;
//-----------------------------------------------------------------------------

Time::Time()
  : dt_pause(0), mode_pause(false)
{
//   Reset();
}

//-----------------------------------------------------------------------------

void Time::Reset()
{
  dt_pause = SDL_GetTicks();
  mode_pause = false;
}

//-----------------------------------------------------------------------------

uint Time::Read() const
{   
  return SDL_GetTicks() - dt_pause;
}

//-----------------------------------------------------------------------------

void Time::Pause()
{
  if (mode_pause) return;
  //assert (!mode_pause);
  debut_pause = SDL_GetTicks();
  mode_pause = true;
}

//-----------------------------------------------------------------------------

void Time::Continue()
{
  assert (mode_pause);
  dt_pause += SDL_GetTicks() - debut_pause;
  mode_pause = false;
}

//-----------------------------------------------------------------------------

uint Time::Clock_Sec()
{
  uint clock_sec = Read()/1000;
  clock_sec %= 60;
  if (clock_sec == 60)
    return clock_sec = 0;
  return clock_sec;
}

//-----------------------------------------------------------------------------

uint Time::Clock_Min()
{
  uint clock_min = Read()/60000;
  if (clock_min > 59)
    return clock_min = 0;
  return clock_min;
}

//-----------------------------------------------------------------------------

std::string Time::GetString()
{
  //if (!affiche) return;
  std::ostringstream ss;
  ss << Clock_Min() << ":" << std::setfill('0') << std::setw(2) << Clock_Sec();
  return ss.str();
}

//-----------------------------------------------------------------------------
}
