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
 * Progress bar for GUI.
 *****************************************************************************/

#include "progress_bar.h"
//-----------------------------------------------------------------------------
#include "../tool/math_tools.h"
#ifdef CL
# include <ClanLib/display.h>
#else
#include <SDL.h>
#include "../include/app.h"
#endif
//-----------------------------------------------------------------------------

#ifdef CL
BarreProg::BarreProg()
  : border_color(CL_Color::black),
    value_color(CL_Color::white),
    background_color(CL_Color::darkgray)
{
  x = y = larg = haut = 0;
  val = min = max = 0;
  m_use_ref_val = false;
}
#else
BarreProg::BarreProg()
{   
   border_color.r = 0;
   border_color.g = 0;
   border_color.b = 0;
   value_color.r = 255;
   value_color.g = 255;
   value_color.b = 255;
   background_color.r = 100;
   background_color.g = 100;
   background_color.b = 100;
   x = y = larg = haut = 0;
   val = min = max = 0;
   m_use_ref_val = false;
   image = NULL;
}

void BarreProg::SetBorderColor( unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   border_color.r = r;
   border_color.g = g;
   border_color.b = b;
   border_color.unused = a;   
}

void BarreProg::SetBackgroundColor( unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   background_color.r = r;
   background_color.g = g;
   background_color.b = b;   
   background_color.unused = a;   
}

void BarreProg::SetValueColor( unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   value_color.r = r;
   value_color.g = g;
   value_color.b = b;
   value_color.unused = a;   
}
#endif

//-----------------------------------------------------------------------------

void BarreProg::InitPos (uint px, uint py, uint plarg, uint phaut)
{
  assert (3 <= plarg);
  assert (3 <= phaut);
  x = px;
  y = py;
  larg = plarg;
  haut = phaut;

  if ( image != NULL)
  {
    SDL_FreeSurface( image);
  }
   
  image = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, larg, haut, 32,
				0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
}

//-----------------------------------------------------------------------------

void BarreProg::InitVal (long pval, long pmin, long pmax)
{
  assert (pmin != pmax);
  assert (pmin < pmax);
  val = pval;
  min = pmin;
  max = pmax;
  val_barre = CalculeValBarre(val);
}

//-----------------------------------------------------------------------------

void BarreProg::Actu (long pval)
{
  val = CalculeVal(pval);
  val_barre = CalculeValBarre(val);
}

//-----------------------------------------------------------------------------

uint BarreProg::CalculeVal (long val) const
{ 
  return BorneLong(val, min, max); 
}

//-----------------------------------------------------------------------------

uint BarreProg::CalculeValBarre (long val) const
{ 
  return ( CalculeVal(val) -min)*(larg-2)/(max-min);
}

//-----------------------------------------------------------------------------

void BarreProg::Draw() const
{
  DrawXY (x,y);
}

//-----------------------------------------------------------------------------
 
// TODO pass SDL_Surface as parameter
 
void BarreProg::DrawXY (uint px, uint py) const
{ 
  int left, right;
   
  // Bordure
#ifdef CL
  CL_Display::draw_rect (CL_Rect(px, py, px+larg, py+haut), border_color);
#else
  SDL_FillRect( image, NULL, SDL_MapRGBA( image->format, border_color.r, border_color.g, border_color.b, border_color.unused));
#endif
   
  // Fond
#ifdef CL
  CL_Display::fill_rect (CL_Rect(px+1, py+1, px+larg-1, py+haut-1), background_color);
#else
  SDL_Rect r_back = {1, 1, larg-2, haut-2};
  SDL_FillRect( image, &r_back, SDL_MapRGBA( image->format, background_color.r, background_color.g, background_color.b,background_color.unused));   
#endif
   
  // Valeur
#ifdef CL
  if (m_use_ref_val) {
    int ref = CalculeValBarre (m_ref_val);
    if (val < m_ref_val) {
      left = px+1+val_barre;
      right = px+1+ref;
    } else {
      left = px+1+ref;
      right = px+1+val_barre;
    }
  } else {
    left = px+1;
    right = px+1+val_barre;
  }  
  CL_Display::fill_rect (CL_Rect(left, py+1, right, py+haut-1), value_color);
#else
  if (m_use_ref_val) {
    int ref = CalculeValBarre (m_ref_val);
    if (val < m_ref_val) {
      left = 1+val_barre;
      right = 1+ref;
    } else {
      left = 1+ref;
      right = 1+val_barre;
    }
  } else {
    left = 1;
    right = 1+val_barre;
  }  

  SDL_Rect r_value = {left, 1, right-left, haut-2};
  SDL_FillRect( image, &r_value, SDL_MapRGBA( image->format, value_color.r, value_color.g, value_color.b, value_color.unused));
#endif
   
  if (m_use_ref_val) {
    int ref = CalculeValBarre (m_ref_val);
#ifdef CL
     CL_Display::draw_line (px+1+ref, py+1, 
			    px+1+ref, py+haut-1, 
			    border_color);
#else
  SDL_Rect r_ref = {1+ref, 1, 1, haut-2};
  SDL_FillRect( image, &r_ref, SDL_MapRGBA( image->format, border_color.r, border_color.g, border_color.b, border_color.unused));           
#endif
  }

  // Marqueurs
  marqueur_it_const it=marqueur.begin(), fin=marqueur.end();
  for (; it != fin; ++it)
  {
#ifdef CL
    int x = px+1+it -> val;
     CL_Display::draw_line (x, py+1, x, py+haut-2,
			    it -> color);
#else
  SDL_Rect r_marq = {1+it->val, 1, 1, haut-2};
  SDL_FillRect( image, &r_marq, SDL_MapRGBA( image->format, border_color.r, border_color.g, border_color.b, border_color.unused));           
#endif
  }
 
#ifndef CL
  // Blit internal surface to destination
  SDL_Rect d = {px, py, larg, haut};
  SDL_BlitSurface( image, NULL, app.sdlwindow, &d);
#endif
}

//-----------------------------------------------------------------------------

// Ajoute/supprime un marqueur
#ifdef CL
BarreProg::marqueur_it BarreProg::AjouteMarqueur (long val, const CL_Color& color)
#else
BarreProg::marqueur_it BarreProg::AjouteMarqueur (long val, const SDL_Color& color)
#endif
{
  marqueur_t m;
  m.val = CalculeValBarre (val);
  m.color = color;
  marqueur.push_back (m);
  return --marqueur.end();
}

BarreProg::marqueur_it BarreProg::AjouteMarqueur (long val, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  marqueur_t m;
  m.val = CalculeValBarre (val);
  m.color.r = r;
  m.color.g = g;
  m.color.b = b;
  marqueur.push_back (m);
  return --marqueur.end();
}


//-----------------------------------------------------------------------------

void BarreProg::SupprimeMarqueur (marqueur_it it)
{ marqueur.erase (it); }

//-----------------------------------------------------------------------------

void BarreProg::Reset_Marqueur() { marqueur.clear(); }

//-----------------------------------------------------------------------------

void BarreProg::SetReferenceValue (bool use, long value)
{
  m_use_ref_val = use;
  m_ref_val = CalculeVal(value);
}

//-----------------------------------------------------------------------------
