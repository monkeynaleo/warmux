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
 * Refresh de l'eau pouvant apparaitre en bas du terrain.
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
   pattern.NewSurface( Point2i(180, surface.GetHeight() + 40), SDL_SWSURFACE|SDL_SRCALPHA, true);
   shift1 = 0;
}

void Water::Reset(){
  actif = ActiveMap().UseWater();
  if(!actif) return;
  Init();
  hauteur_eau = WATER_INITIAL_HEIGHT;
  vague = 0;
  temps_eau = 0;
  temps_montee = GO_UP_TIME * 60 * 1000;
  height.clear();
  height.assign(180, 0);
  Refresh(); // Calculate first height position
}

void Water::Free(){
  if(!actif)
    return;
  surface.Free();
  pattern.Free();
  height.clear();
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
  // on rempli le sol avec de l'eau
  if (WAVE_TIME < (global_time->Read() - temps_eau))
  {
    temps_eau = global_time->Read();
    vague += WAVE_STEP;
    if (surface.GetWidth() <= vague)
		vague=0;
  }

  int x = -surface.GetWidth() + vague;
  int y = world.GetHeight()-(hauteur_eau + height_mvt);

  double decree = (double) 2*M_PI/360;

  double angle1 = 0;
  double angle2 = shift1;
  do
  {
    int offset=0;
    double y_pos = y + sin(angle1)*10 + sin(angle2)*10;

    if (0<=x+offset)
	 height.at(x+offset) = (int)y_pos;

    angle1 += 2*decree;
    angle2 += 4*decree;
    x++;
  } while ((uint)x < 180);

  shift1 += 4*decree;
}

void Water::Draw(){
  if (!actif)
    return;

  // Compute 1 pattern:
  pattern.SetAlpha( 0, 0);
  pattern.Fill(0x00000000);

  int y0 = world.GetHeight()-(hauteur_eau + height_mvt)-20;

  for(uint x=0; x<180; x++){
    Point2i dst(x, height.at(x) - y0);
    pattern.Blit(surface, dst);
  }
  pattern.SetAlpha(SDL_SRCALPHA, 0);

  int x0 = camera.GetPosition().x;
  while(x0<0)
    x0+=180;
  while(x0>180)
    x0-=180;

  for(int x=camera.GetPosition().x-x0; x<camera.GetPosition().x+camera.GetSize().x; x+=180)
    for(int y=y0; y<(int)camera.GetPosition().y+(int)camera.GetSize().y; y+=surface.GetSize().y)
      AbsoluteDraw(pattern, Point2i(x, y));
}

int Water::GetHeight(int x){
  if (IsActive()){
    while(x<0)
      x += 180;
    while(x>=180)
      x -= 180;
    return height.at(x);
  }
  else
    return world.GetHeight();
}

bool Water::IsActive(){
  return actif;
}

