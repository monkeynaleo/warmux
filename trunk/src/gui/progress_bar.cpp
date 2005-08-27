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
#include <ClanLib/display.h>
//-----------------------------------------------------------------------------

BarreProg::BarreProg()
  : border_color(CL_Color::black),
    value_color(CL_Color::white),
    background_color(CL_Color::darkgray)
{
  x = y = larg = haut = 0;
  val = min = max = 0;
  m_use_ref_val = false;
}

//-----------------------------------------------------------------------------

void BarreProg::InitPos (uint px, uint py, uint plarg, uint phaut)
{
  assert (3 <= plarg);
  assert (3 <= phaut);
  x = px;
  y = py;
  larg = plarg;
  haut = phaut;
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
{ return BorneLong(val, min, max); }

//-----------------------------------------------------------------------------

uint BarreProg::CalculeValBarre (long val) const
{ return ( CalculeVal(val) -min)*(larg-2)/(max-min); }

//-----------------------------------------------------------------------------

void BarreProg::Draw() const
{
  DrawXY (x,y);
}

//-----------------------------------------------------------------------------

void BarreProg::DrawXY (uint px, uint py) const
{
  int left, right;

  // Bordure
  CL_Display::draw_rect (CL_Rect(px, py, px+larg, py+haut), border_color);

  // Fond
  CL_Display::fill_rect (CL_Rect(px+1, py+1, px+larg-1, py+haut-1), background_color);

  // Valeur
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

  if (m_use_ref_val) {
    int ref = CalculeValBarre (m_ref_val);
    CL_Display::draw_line (px+1+ref, py+1, 
			   px+1+ref, py+haut-1, 
			   border_color);

  }

  // Marqueurs
  marqueur_it_const it=marqueur.begin(), fin=marqueur.end();
  for (; it != fin; ++it)
  {
    int x = px+1+it -> val;
    CL_Display::draw_line (x, py+1, x, py+haut-2,
			   it -> color);
  }
}

//-----------------------------------------------------------------------------

// Ajoute/supprime un marqueur
BarreProg::marqueur_it BarreProg::AjouteMarqueur (long val, const CL_Color& color)
{
  marqueur_t m;
  m.val = CalculeValBarre (val);
  m.color = color;
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
