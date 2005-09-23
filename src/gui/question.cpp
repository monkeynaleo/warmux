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
#ifdef CL
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/Core/System/error.h>
#else
#include <SDL.h>
#endif
#include "../graphic/graphism.h"
#include "../graphic/video.h"
using namespace Wormux;

//-----------------------------------------------------------------------------

Question::Question()
{}

//-----------------------------------------------------------------------------

#ifdef CL
void Question::TraiteClic (const CL_InputEvent &event)
{
  // Sinon, on utilise le choix par défaut ?
  if (choix_defaut.actif) 
  {
    reponse = choix_defaut.valeur;
    m_fin_boucle = true;
    return;
  }
}
#else
// TODO oder paragui ?
#endif
//-----------------------------------------------------------------------------

// Traite une touche du clavier
#ifdef CL
void Question::TraiteTouche (const CL_InputEvent &event)
{
  // Teste les différents choix
  choix_iterator it=choix.begin(), fin=choix.end();
  for (; it != fin; ++it)
  {
    if (event.id == it -> m_touche)
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
#endif

//-----------------------------------------------------------------------------

#ifdef CL
void Question::Draw()
{
  TexteEncadre (police_grand, 
		video.GetWidth()/2, video.GetHeight()/2,
		message);
  CL_Display::update(CL_Rect(0,0,CL_Display::get_width(), CL_Display::get_height()));
}
#else
void Question::Draw()
{
 /* TexteEncadre (police_grand, 
		video.GetWidth()/2, video.GetHeight()/2,
		message);
  CL_Display::update(CL_Rect(0,0,CL_Display::get_width(), CL_Display::get_height()));*/
   
   //TODO
}

#endif

//-----------------------------------------------------------------------------


#ifdef CL
int Question::PoseQuestion ()
{
  m_keyboard_slot = CL_Keyboard::sig_key_up().
    connect(this, &Question::TraiteTouche);
  m_mouse_slot = CL_Mouse::sig_key_up().
    connect(this, &Question::TraiteClic);

  Draw();

  // Boucle en attendant qu'un choix soit fait
  m_fin_boucle = false;
  do
  {
    CL_System::keep_alive (10);
    CL_Display::flip();
  } while (!m_fin_boucle);

  CL_Keyboard::sig_key_up().disconnect(m_keyboard_slot);
  CL_Keyboard::sig_key_up().disconnect(m_mouse_slot);
  return reponse;
}

#else

int Question::PoseQuestion ()
{

  Draw();

   return reponse;
}


#endif

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
