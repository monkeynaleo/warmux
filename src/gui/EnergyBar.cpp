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
 *Energy bar.
 *****************************************************************************/

#include "EnergyBar.h"

void EnergyBar::Actu (long real_energy){
  Color color;
  double r, g, b, a;
  long app_energy;

  /* update progress bar position*/
  BarreProg::Actu(real_energy);
  long max = GetMaxVal();

  /* get the real applied enargie value. It may be different from the
   * real_energy in case of under/over flow*/
  app_energy = BarreProg::GetVal();

  //Energy bar color
  if( app_energy < (max / 2) )
    {
      r = ( 2.0 * ((final_color.GetRed()   * ((max / 2) - app_energy)) + (app_energy * inter_color.GetRed()))) / max;
      g = ( 2.0 * ((final_color.GetGreen() * ((max / 2) - app_energy)) + (app_energy * inter_color.GetGreen()))) / max;
      b = ( 2.0 * ((final_color.GetBlue()  * ((max / 2) - app_energy)) + (app_energy * inter_color.GetBlue()))) / max;
      a = ( 2.0 * ((final_color.GetAlpha() * ((max / 2) - app_energy)) + (app_energy * inter_color.GetAlpha()))) / max;
    }
  else
    {
      r = ( 2.0 * ((init_color.GetRed() * (app_energy - (max / 2))) + (inter_color.GetRed() * (max - app_energy)))) / max;
      g = ( 2.0 * ((init_color.GetGreen() * (app_energy - (max / 2))) + (inter_color.GetGreen() * (max - app_energy)))) / max;
      b = ( 2.0 * ((init_color.GetBlue() * (app_energy - (max / 2))) + (inter_color.GetBlue() * (max - app_energy)))) / max;
      a = ( 2.0 * ((init_color.GetAlpha() * (app_energy - (max / 2))) + (inter_color.GetAlpha() * (max - app_energy)))) / max;
    }
  color.SetColor((uint)(r),
                 (uint)(g),
                 (uint)(b),
                 (uint)(a));
  SetValueColor(color);
}
