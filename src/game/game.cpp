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
 * Classe principale qui gêre le jeu : initialisation, dessin, gestion
 * des différents composants, et boucle de jeu.
 *****************************************************************************/

#include "game.h"
//-----------------------------------------------------------------------------
#include "game_loop.h"
#include "../interface/cursor.h"
#include "../team/macro.h"
#include "../graphic/video.h"
#include "../interface/keyboard.h"
#include <SDL.h>

#include "../graphic/fps.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "time.h"
#include "../weapon/weapons_list.h"
#include "../sound/jukebox.h"
#include "game_mode.h"
#include "../interface/game_msg.h"
#include "../tool/i18n.h"
#include "../object/bonus_box.h"
#include <sstream>
#include <iostream>

using namespace Wormux;
//-----------------------------------------------------------------------------

#ifdef DEBUG
  // Mode "bavard gros lourdo" ?
//# define DEBUG_VERBOSE
#endif

# define COUT_DEBUG cout << "[Jeu] "

//-----------------------------------------------------------------------------

// Une seconde ...
const uint UNE_SECONDE = 1000; // ms

//-----------------------------------------------------------------------------
Jeu jeu;
//-----------------------------------------------------------------------------

Jeu::Jeu()
{
  initialise = false;
  jeu_lance = false;
}

//-----------------------------------------------------------------------------

// Fin d'une partie = tous les vers d'une equipe sont morts
bool Jeu::EstFinPartie()
{
  uint n=0;

  // Calcule le nombre d'équipe où il reste des vers
  POUR_CHAQUE_EQUIPE(equipe)
  {
    if ((**equipe).NbAliveCharacter() != 0) n++;
  }
  return (n <= 1);
}

//-----------------------------------------------------------------------------

void Jeu::MsgChargement()
{
  std::cout << std::endl;
  std::cout << "[ " << _("Starting a new game") << " ]" << std::endl;
  std::cout << "Loading game... => Splashscreen is TODO" << std::endl;
}

//-----------------------------------------------------------------------------

void Jeu::MsgFinPartie()
{
  // Message de fin
  bool gagnant_trouve;
  std::string txt(_("End of the game!\n"));

  gagnant_trouve = false;
  POUR_CHAQUE_EQUIPE(equipe)
  {
    if (0 < (**equipe).NbAliveCharacter())
    {
      gagnant_trouve = true;
      txt += Format(_("%s team has won."), (**equipe).GetName().c_str());
      break;
    }
  }
  if (gagnant_trouve) 
    jukebox.Play("share","victory");
  else
    txt += _("The game has ended as a draw.");
  std::cout << txt << std::endl;

  question.Init (txt, true, 0);
  PoseQuestion();
}

//-----------------------------------------------------------------------------

int Jeu::PoseQuestion (bool dessine)
{
  global_time.Pause();

  if (dessine) game_loop.Draw ();
  question.PoseQuestion ();

  global_time.Continue(); 
  return question.reponse;
}

//-----------------------------------------------------------------------------

void Jeu::LanceJeu()
{
  bool err=true;
  std::string err_msg;

  try
  {
    InitGame ();

    bool fin;
    do
    {
      jeu_lance = true;
      image_par_seconde.Reset();
      game_loop.Run();
      std::cout << "Quitte run" << std::endl;
      jeu_lance = false;
      
      if (!EstFinPartie()) 
      {
        const char *msg = _("Do you really want to quit? (Y/N)");
        question.Init (msg, true, 0);
	
        {
          /* Tiny fix by Zygmunt Krynicki <zyga@zyga.dyndns.org> */
          /* Let's find out what the user would like to press ... */
          char *key_x_ptr = strchr (msg, '/');
          char key_x;
          if (key_x_ptr && key_x_ptr > msg) /* it's there and it's not the first char */
            key_x = tolower(key_x_ptr[-1]);
          else
            abort();
          if (!isalpha(key_x)) /* sanity check */
            abort();
	   question.choix.push_back ( Question::choix_t(SDLK_a + (int)key_x - 'a', 1) );
	}
	
        fin = (PoseQuestion() == 1);
      } else {
	fin = true;
      }
    } while (!fin);
    err = false;
  }
  catch (const std::exception &e)
  {
    err_msg = e.what();
  }

  if (!err)
  {
    if (EstFinPartie()) MsgFinPartie();
  }

  world.FreeMem();
  caisse.FreeMem();
  jukebox.StopAll();
   
  if (err)
  {
    std::string txt = Format(_("Error:\n%s"), err_msg.c_str());
    std::cout << std::endl << txt << std::endl;
    question.Init (txt, true, 0);
    PoseQuestion (false);
  }
}

//-----------------------------------------------------------------------------

void Jeu::Pause()
{
  question.Init (_("Pause"), true, 0);
  PoseQuestion();
}

//-----------------------------------------------------------------------------
