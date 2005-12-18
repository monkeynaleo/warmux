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
 * Affiche un message dans le jeu, puis pose une question dans le jeu ou
 * attend au moins la pression d'une touche.
 *****************************************************************************/

#include "question.h"
//-----------------------------------------------------------------------------
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include "../graphic/font.h"
#include "../graphic/video.h"
#include "../include/app.h" // SDL_Flip
#include "../map/map.h"
using namespace Wormux;

//-----------------------------------------------------------------------------

Question::Question()
{}

//-----------------------------------------------------------------------------

void Question::TraiteTouche (SDL_Event &event)
{
  // Teste les différents choix
  choix_iterator it=choix.begin(), fin=choix.end();
  for (; it != fin; ++it)
  {
    if (event.key.keysym.sym == it -> m_touche)
    {
      reponse = it -> m_val;
      m_fin_boucle = true;
      return;
    }
  }

  // Sinon, on utilise le choix par défaut ?
  if (choix_defaut.actif) 
  {
    reponse = choix_defaut.valeur;
    m_fin_boucle = true;
    return;
  }
}

//-----------------------------------------------------------------------------


// Ecrit un texte et l'encadre
void TexteEncadre (Font &police, int txt_x, int txt_y, 
		   const std::string &txt, uint espace=10)
{
  int x,y,larg,haut;
  larg = police.GetWidth(txt)+espace*2;
  x = txt_x - larg / 2;
  haut = police.GetHeight(txt)+espace*2;
  y = txt_y - haut / 2;
  txt_y -= police.GetHeight(txt)/2;

  boxRGBA(app.sdlwindow, x, y, x+larg, y+haut,
	  80,80,159,206);

  rectangleRGBA(app.sdlwindow, x, y, x+larg, y+haut,
		49, 32, 122, 255);  
  
// TODO  
//  CL_Display::fill_rect (CL_Rect(x, y, x+larg, y+haut), CL_Color(0, 0, 0, 255*7/10));
//  CL_Display::draw_rect (CL_Rect(x, y, x+larg, y+haut), CL_Color::red);

  world.ToRedrawOnScreen(Rectanglei(x, y, larg, haut));
  police.WriteCenterTop (txt_x, txt_y, txt, white_color);
}



void Question::Draw()
{
  TexteEncadre (big_font, 
		video.GetWidth()/2, video.GetHeight()/2,
		message);
  SDL_Flip( app.sdlwindow);
}

//-----------------------------------------------------------------------------

int Question::PoseQuestion ()
{
  SDL_Event event;
  
  m_fin_boucle = false;
  do
  {
    Draw();

    while( SDL_PollEvent( &event) ) 
    {      
      if ( event.type == SDL_QUIT || event.type == SDL_MOUSEBUTTONDOWN )
      {  
        reponse = choix_defaut.valeur;
        m_fin_boucle = true;
      }
      if (event.type == SDL_KEYUP)
      {
        TraiteTouche(event); 
      }
    }
  } while (!m_fin_boucle);
  
  return reponse;
}

//-----------------------------------------------------------------------------

void Question::Init (const std::string &pmessage, 
		     bool pchoix_defaut, int pvaleur)
{
  message = pmessage;
  choix_defaut.actif = pchoix_defaut;
  choix_defaut.valeur = pvaleur;
  //  m_attend_touche_debut = false;
}

//-----------------------------------------------------------------------------
