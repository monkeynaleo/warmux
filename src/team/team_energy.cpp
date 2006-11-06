/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
 * energy bar of each team
 *****************************************************************************/

#include "team_energy.h"
#include <sstream>
#include <math.h>
#include "../map/camera.h"
#include "../game/time.h"
#include "../graphic/text.h"
#include "team.h"
#include "../include/app.h"

const uint BARRE_LARG = 13;
const uint BARRE_HAUT = 50;
const uint ESPACEMENT = 3;

const uchar ALPHA = 127;
const uchar ALPHA_FOND = 0;

const float DUREE_MVT = 750.0;

TeamEnergy::TeamEnergy(Team * _team)
{
  dx = 0;
  dy = 0;
  tps_debut_mvt = 0;
  max_value = 0;
  status = EnergyStatusOK;
  energy_bar.InitPos(0, 0, BARRE_LARG, BARRE_HAUT);

  energy_bar.SetInitColor(Color(R_INIT, G_INIT, B_INIT, ALPHA));
  energy_bar.SetInterColor(Color(R_INTER, G_INTER, B_INTER, ALPHA));
  energy_bar.SetFinalColor(Color(R_FINAL, G_FINAL, B_FINAL, ALPHA));

  energy_bar.SetBorderColor(Color(255, 255, 255, ALPHA));
  energy_bar.SetBackgroundColor(Color(255*6/10, 255*6/10, 255*6/10, ALPHA_FOND));

  team = _team;
}

void TeamEnergy::Config(uint _current_energy,
                        uint _max_energy)
{
  max_value = _max_energy;

  value = _current_energy;
  new_value = _current_energy;
  assert(max_value != 0)
      energy_bar.InitVal(value, 0, max_value, BarreProg::PROG_BAR_VERTICAL);
}

void TeamEnergy::Refresh()
{
  switch(status)
  {
    // energy value from one team have changed
    case EnergyStatusValueChange:
      if(new_value > value)
        value = new_value;
      if(value > new_value)
        --value;
      if(value == new_value)
        status = EnergyStatusWait;
      break;

    // ranking is changing
    case EnergyStatusRankChange:
      Move();
      break;

    // Currently no move
    case EnergyStatusOK:
      if( value != new_value && !IsMoving())
        status = EnergyStatusValueChange;
      else
        if( rank != new_rank )
          status = EnergyStatusRankChange;
      break;

    // This energy bar wait others bar before moving
    case EnergyStatusWait:
      break;
  }
}

void TeamEnergy::Draw(const Point2i& pos)
{
  energy_bar.Actu(value);
  Point2i tmp = pos + Point2i(rank * (BARRE_LARG + 50) + dx, ESPACEMENT + dy);
  energy_bar.DrawXY(tmp);
  AppWormux::GetInstance()->video.window.Blit(team->flag, tmp);
}

void TeamEnergy::SetValue(uint new_energy)
{
  new_value = new_energy;
}

void TeamEnergy::SetRanking(uint _rank)
{
  rank = _rank;
  new_rank = _rank;
}

void TeamEnergy::NewRanking(uint _new_rank)
{
  new_rank = _new_rank;
}

// Move energy bar (change in ranking)
void TeamEnergy::Move()
{
  if( value != new_value && !IsMoving()) {
    // Other energy bar are moving so waiting for others to move
    status = EnergyStatusWait;
    return;
  }

  if( rank == new_rank && !IsMoving()) {
    // Others energy bar are moving
    status = EnergyStatusWait;
    return;
  }

  // teams ranking have changed
  Time * global_time = Time::GetInstance();
  if( rank != new_rank )
  {
    if(tps_debut_mvt == 0)
      tps_debut_mvt = global_time->Read();

    dy = (int)(( (BARRE_HAUT+ESPACEMENT) * ((float)new_rank - rank))
        * ((global_time->Read() - tps_debut_mvt) / DUREE_MVT));

    // displacement in arc of circle only when losing place ranking
    if( new_rank > rank )
      dx = (int)(( 3.0 * (BARRE_HAUT+ESPACEMENT) * ((float)rank - new_rank))
          * sin( M_PI * ((global_time->Read() - tps_debut_mvt) /DUREE_MVT)));

    // End of movement ?
    if( (global_time->Read() - tps_debut_mvt) > DUREE_MVT )
    {
      dy = 0;
      dx = 0;
      rank = new_rank;
      tps_debut_mvt = 0;
      status = EnergyStatusWait;
      return;
    }
  } else {
    // While moving, it came back to previous place in ranking
    dy = (int)((float)dy - ((global_time->Read() - tps_debut_mvt) /DUREE_MVT) * dy);
    dx = (int)((float)dx - ((global_time->Read() - tps_debut_mvt) /DUREE_MVT) * dx);
  }
}

bool TeamEnergy::IsMoving () const
{
  if( dx != 0 || dy != 0 )
    return true;
  return false;
}
