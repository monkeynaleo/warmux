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
#include "../graphic/graphism.h"
#include "../graphic/font.h"

#include <ClanLib/core.h>
#include <sstream>
#include <iomanip>
//-----------------------------------------------------------------------------

const double VITESSE_MIN = 0.01;
const double VITESSE_MAX = 50;

//-----------------------------------------------------------------------------
namespace Wormux
{

Temps temps;
//-----------------------------------------------------------------------------

Temps::Temps()
{
  Reset();
}

//-----------------------------------------------------------------------------

void Temps::Reset()
{
  dt_pause = CL_System::get_time(); //Needed to have time set to 0, on a reset
  mode_pause = false;
  ChangeVitesse (1.0, false);
}

//-----------------------------------------------------------------------------

void Temps::ChangeVitesse (double pvitesse, bool message=false)
{ 
  vitesse = BorneDouble (pvitesse, VITESSE_MIN, VITESSE_MAX); 
  if (message)
  {
    std::ostringstream ss;
    ss << "Vitesse du temps : " << std::setprecision(3) << vitesse*100 << " %";
    game_messages.Add (ss.str());
  }
}

//-----------------------------------------------------------------------------

uint Temps::Lit() const
{ 
  double t = CL_System::get_time() - dt_pause;
  t *= vitesse;
  FORCE_ASSERT (t < UINT_MAX);
  return (uint)t;
}

//-----------------------------------------------------------------------------

void Temps::Pause()
{
  if (mode_pause) return;
  //assert (!mode_pause);
  debut_pause = CL_System::get_time();
  mode_pause = true;
}

//-----------------------------------------------------------------------------

void Temps::Reprend()
{
  assert (mode_pause);
  dt_pause += CL_System::get_time() - debut_pause;
  mode_pause = false;
}

//-----------------------------------------------------------------------------

uint Temps::Horloge_Sec()
{
  uint horloge_sec = Lit()/1000;
  horloge_sec %= 60;
  if (horloge_sec == 60)
    return horloge_sec = 0;
  return horloge_sec;
}

//-----------------------------------------------------------------------------

uint Temps::Horloge_Min()
{
  uint horloge_min = Lit()/60000;
  if (horloge_min > 59)
    return horloge_min = 0;
  return horloge_min;
}

//-----------------------------------------------------------------------------

void Temps::Draw()
{
  //if (!affiche) return;
  std::ostringstream ss;
  ss << Horloge_Min() << ":" << std::setfill('0') << std::setw(2) << Horloge_Sec();
  police_grand.WriteCenterTop (video.GetWidth()/2, 10, ss.str());
}

//-----------------------------------------------------------------------------
}
