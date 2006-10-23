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
 * EnergyBar.
 *****************************************************************************/

#ifndef __ENERGYBAR_H__
#define __ENERGYBAR_H__

#include "../gui/progress_bar.h"

const uchar R_INIT = 0;   //Color R when 100%
const uchar G_INIT = 255; //Color G when 100%
const uchar B_INIT = 0;   //Couleur B when 100%
const uchar A_INIT = 255; //Alpha when 100%

const uchar R_INTER = 255;    //Color R when 50%
const uchar G_INTER = 255;    //Color G when 50%
const uchar B_INTER = 0;      //Color B when 50%
const uchar A_INTER = 255;    //Alpha when 50%

const uchar R_FINAL = 255;    //Color R when 0%
const uchar G_FINAL = 0;      //Color G when 0%
const uchar B_FINAL = 0;      //Color B when 0%
const uchar A_FINAL = 255;    //Alpha when 0%


class EnergyBar : public BarreProg {
    Color init_color, inter_color, final_color;
  public:
    EnergyBar() :
      BarreProg()
    {
      init_color.SetColor (R_INIT,  G_INIT,  B_INIT,  A_INIT);
      inter_color.SetColor(R_INTER, G_INTER, B_INTER, A_INTER);
      final_color.SetColor(R_FINAL, G_FINAL, B_FINAL, A_FINAL);
    };
    inline void SetInitColor(const Color &color)
     {
       init_color = color;
     };
    inline void SetInterColor(const Color &color)
     {
       inter_color = color;
     };
    inline void SetFinalColor(const Color &color)
     {
       final_color = color;
     };


  void Actu (long val);
};

#endif
