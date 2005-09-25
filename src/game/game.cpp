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

#ifdef CL
#include "../interface/mouse.h"
#endif

#include "../graphic/fps.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "time.h"
#include "../weapon/weapons_list.h"
#include "../sound/jukebox.h"
#include "game_mode.h"
#include "../graphic/graphism.h"
#include "../interface/game_msg.h"
#include "../tool/i18n.h"
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

#ifdef CL
  CL_Display::clear (CL_Color::black);
  police_grand.WriteCenterTop (video.GetWidth()/2, video.GetHeight()/2, 
			    _("Load game data..."));
  CL_Display::flip();
#else
   
   std::cout << "Loading game... => Splashscreen is TODO" << std::endl;

#endif
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
#ifdef CL
     jukebox.Play("victory");
#else
     jukebox.Play("share","victory");
#endif
   else
    txt += _("The game has ended as a draw.");
  std::cout << txt << std::endl;

  question.Init (txt, true, 0);
  PoseQuestion();
}

//-----------------------------------------------------------------------------

#ifdef CL
void Jeu::SignalWM_QUIT () 
{ 
#ifdef DEBUG_VERBOSE
  COUT_DEBUG << "Signal 'WM_QUIT' intercepté." << endl;
#endif
  fin_partie = true; 
}
#endif

//-----------------------------------------------------------------------------

#ifdef CL
void Jeu::SignalPAINT (const CL_Rect &rect) 
{ 
#ifdef DEBUG_VERBOSE
  COUT_DEBUG << "Signal 'PAINT' intercepté." << endl;
#endif
}
#endif

//-----------------------------------------------------------------------------

#ifdef CL
void Jeu::SignalRESIZE (int larg, int haut)
{ 
#ifdef DEBUG_VERBOSE
  COUT_DEBUG << "Signal 'WM_RESIZE' intercepté." << endl;
#endif
}
#endif

//-----------------------------------------------------------------------------

int Jeu::PoseQuestion (bool dessine)
{
  temps.Pause();

  if (dessine) game_loop.Draw ();
  question.PoseQuestion ();

  temps.Reprend(); 
  return question.reponse;
}

//-----------------------------------------------------------------------------

void Jeu::LanceJeu()
{
  bool err=true;
  std::string err_msg;

#ifdef CL
  clavier.InstallePilote();
  mouse.InstallePilote();
#ifdef PORT_CL07
  slot_quit = CL_DisplayWindow::sig_window_close().connect(this, &Jeu::SignalWM_QUIT);
  slot_paint = CL_DisplayWindow::sig_paint().connect(this, &Jeu::SignalPAINT);
  slot_resize = CL_DisplayWindow::sig_resize().connect(this, &Jeu::SignalRESIZE);
#endif
#endif
   
  try
  {
    InitGame ();

    bool fin;
    do
    {
      jeu_lance = true;
      image_par_seconde.Reset();
      game_loop.Run();
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
#ifdef CL
	   question.choix.push_back ( Question::choix_t(CL_KEY_A + (int)key_x - 'a', 1) );
#else
	   //TODO
#endif
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

  monde.FreeMem();
  jukebox.StopAll();
#ifdef CL
  clavier.DesinstallePilote();
  mouse.DesinstallePilote();
#ifdef PORT_CL07
  CL_DisplayWindow::sig_window_close().disconnect(slot_quit);
  CL_DisplayWindow::sig_resize().disconnect(slot_resize);
  CL_DisplayWindow::sig_paint().disconnect(slot_paint);
#endif
#endif
   
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
