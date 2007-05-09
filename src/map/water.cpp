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
 * Refresh water that may be placed in bottom of the ground.
 *****************************************************************************/

#include "water.h"
#include <SDL.h>
#include "camera.h"
#include "map.h"
#include "maps_list.h"
#include "../game/time.h"
#include "../include/app.h"
#include "../interface/interface.h"
#include "../tool/resource_manager.h"

const uint WAVE_TIME=10;
const uint WAVE_STEP=1;
const uint WAVE_HEIGHT = 5;

const uint GO_UP_TIME = 1; // min
const uint GO_UP_STEP = 15; // pixels
const uint GO_UP_OSCILLATION_TIME = 30; // seconds
const uint GO_UP_OSCILLATION_NBR = 30; // amplitude
const float t = (GO_UP_OSCILLATION_TIME*1000.0);
const float a = GO_UP_STEP/t;
const float b = 1.0;

void Water::Init(){
   Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
   surface = resource_manager.LoadImage(res, "gfx/water");
   surface.SetAlpha(0, 0);
   pattern.NewSurface(Point2i(pattern_width, surface.GetHeight() + 40),
                      SDL_SWSURFACE|SDL_SRCALPHA, true);
   /* Convert the pattern into the same format than surface. This allow not to need
    * conversions on fly and thus saves CPU */
   pattern.SetSurface(
       SDL_ConvertSurface(pattern.GetSurface(),
                          surface.GetSurface()->format,
                          SDL_SWSURFACE|SDL_SRCALPHA),
       true /* free old one */);
   shift1 = 0;
   resource_manager.UnLoadXMLProfile(res);
}

void Water::Reset(){
  actif = ActiveMap().UseWater();
  if(!actif) return;
  Init();
  hauteur_eau = WATER_INITIAL_HEIGHT;
  vague = 0;
  temps_eau = 0;
  temps_montee = GO_UP_TIME * 60 * 1000;
  Refresh(); // Calculate first height position
}

void Water::Free(){
  if(!actif)
    return;
  surface.Free();
  pattern.Free();
}

void Water::Refresh(){
  if (!actif)
    return;

  height_mvt = 0;

  ////////  Height Calculation:
  Time * global_time = Time::GetInstance();
  if (temps_montee < global_time->Read())
  {
    if(temps_montee + GO_UP_OSCILLATION_TIME * 1000 > global_time->Read()){
      uint dt=global_time->Read()- temps_montee;
      height_mvt = GO_UP_STEP + (uint)(((float)GO_UP_STEP * sin(((float)(dt*(GO_UP_OSCILLATION_NBR-0.25))/GO_UP_OSCILLATION_TIME/1000.0)*2*M_PI))/(a*dt+b));
    }
    else{
      temps_montee += GO_UP_TIME * 60 * 1000;
      hauteur_eau += GO_UP_STEP;
    }
  }

  ////////  Wave calculation:
  // Fill ground with water
  if (WAVE_TIME < (global_time->Read() - temps_eau))
  {
    temps_eau = global_time->Read();
    vague += WAVE_STEP;
    if (surface.GetWidth() <= vague)
		vague=0;
  }


  double decree = static_cast<double>(2*M_PI/360.0);

  double angle1 = 0;
  double angle2 = shift1;

  for (int x = -surface.GetWidth() + vague; x < pattern_width; x++)
  {
    double y_pos = sin(angle1)*10 + sin(angle2)*10;

    if (0 <= x)
      height[x] = (int)y_pos;

    angle1 += 2*decree;
    angle2 += 4*decree;
  }

  shift1 += 4*decree;

  /* Now the wave has changed, we need to build the new image pattern */
  pattern.SetAlpha( 0, 0);
  pattern.Fill(0x00000000);

  SDL_LockSurface(surface.GetSurface());
  SDL_LockSurface(pattern.GetSurface());

  uint bpp = surface.GetSurface()->format->BytesPerPixel;
  for (uint x = 0; x < pattern_width; x++)
    for (uint y=0; y<(uint)surface.GetHeight(); y++)
      {
        memcpy((Uint8*)pattern.GetSurface()->pixels + x * bpp
               + (height[x]+20+y) * pattern.GetSurface()->pitch,
               (Uint8*)surface.GetSurface()->pixels + y * bpp, bpp);
      }

  SDL_UnlockSurface(pattern.GetSurface());
  SDL_UnlockSurface(surface.GetSurface());

  pattern.SetAlpha(SDL_SRCALPHA, 0);
}

void Water::Draw(){
  if (!actif)
    return;

  int x0 = camera.GetPosition().x % pattern_width;

  for(int x=camera.GetPosition().x-x0; x<camera.GetPosition().x+camera.GetSize().x; x+=pattern_width)
    for(int y=world.GetHeight()-(hauteur_eau + height_mvt)-20; y<(int)camera.GetPosition().y+(int)camera.GetSize().y; y+=surface.GetSize().y)
      AbsoluteDraw(pattern, Point2i(x, y));
}

int Water::GetHeight(int x){
  if (IsActive())
    return height[x % pattern_width] + world.GetHeight()-(hauteur_eau + height_mvt);
  else
    return world.GetHeight();
}

bool Water::IsActive(){
  return actif;
}

