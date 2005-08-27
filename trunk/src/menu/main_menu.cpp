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
 * Menu du jeu permettant de lancer une partie, modifier les options, d'obtenir
 * des informations, ou encore quitter le jeu.
 *****************************************************************************/

#include "main_menu.h"
//-----------------------------------------------------------------------------
#include <ClanLib/core.h>
#include <string.h>
#include "../include/app.h"
#include "../include/constant.h" // VERSION
#include "../tool/i18n.h"
#include "../graphic/graphism.h"
#include "../graphic/video.h"
#include "../sound/jukebox.h"
#include "../tool/file_tools.h"

#ifndef WIN32
#include <dirent.h>
#endif

using namespace Wormux;
using namespace std;
//-----------------------------------------------------------------------------

// Position du texte de la version
const int VERSION_DY = -40;

const int DEFAULT_SCREEN_WIDTH = 800 ;
const int DEFAULT_SCREEN_HEIGHT = 600 ;
const int BUTTON_WIDTH = 282 ;  // Button width at default screen resolution
const int BUTTON_HEIGHT = 57 ;  // Button height at default screen resolution

//-----------------------------------------------------------------------------
Menu menu;
//-----------------------------------------------------------------------------

Menu::Menu()
{}

//-----------------------------------------------------------------------------

void Menu::ChargeImage()
{
  int x_button, button_width, button_height ;
  double y_scale ;

  background = CL_Surface("intro/fond", graphisme.LitRes());

  x_button = (int)((double)474 / DEFAULT_SCREEN_WIDTH * video.GetWidth()) ;
  y_scale = (double)1 / DEFAULT_SCREEN_HEIGHT * video.GetHeight() ;

  button_width = (int)((double)BUTTON_WIDTH / DEFAULT_SCREEN_WIDTH * video.GetWidth()) ;
  button_height = (int)((double)BUTTON_HEIGHT / DEFAULT_SCREEN_HEIGHT * video.GetHeight()) ;

  jouer.SetPos (x_button,(int)(192 * y_scale));
  jouer.SetText (_("Play"));
  jouer.SetFont (&police_grand);
  jouer.SetImage ("intro/jouer", graphisme.LitRes());
  jouer.SetSize(button_width, button_height);

  network.SetPos (x_button, (int)(261 * y_scale));
  network.SetText (_("Network Game"));
  network.SetFont (&police_grand);
  network.SetImage ("intro/jouer", graphisme.LitRes());
  network.SetSize(button_width, button_height);

  options.SetPos(x_button,(int)(329 * y_scale));
  options.SetText (_("Options"));
  options.SetFont (&police_grand);
  options.SetImage ("intro/options", graphisme.LitRes());
  options.SetSize(button_width, button_height);

  infos.SetPos (x_button,(int)(397 * y_scale));
  infos.SetText (_("Info"));
  infos.SetFont (&police_grand);
  infos.SetImage ("intro/infos", graphisme.LitRes());
  infos.SetSize(button_width, button_height);

  quitter.SetPos (x_button,(int)(465 * y_scale));
  quitter.SetText (_("Quit"));
  quitter.SetFont (&police_grand);
  quitter.SetImage ("intro/quitter", graphisme.LitRes());
  quitter.SetSize(button_width, button_height);
}

//-----------------------------------------------------------------------------

void Menu::SignalWM_QUIT ()
{
  fin_boucle = true;
}

//-----------------------------------------------------------------------------

menu_item Menu::Boucle ()
{
  string txt_version;
  int x,y;
  txt_version = string("Version ") + string(VERSION);

  choix = menuNULL;
  fin_boucle = false;
  while (!fin_boucle)
  {
    CL_Display::clear(CL_Color::black);

    // Affiche le logo
    background.draw(CL_Rect(0, 0, video.GetWidth()-1, video.GetHeight()-1));

    // Lit la position de la souris
    x = CL_Mouse::get_x();
    y = CL_Mouse::get_y();

    jouer.Draw (x, y);
    network.Draw (x, y);
    options.Draw (x, y);
    infos.Draw (x, y);
    quitter.Draw (x, y);

    // Affiche le numero de version
	police_grand.WriteCenter (video.GetWidth()/2,
			         video.GetHeight()+VERSION_DY,
			         txt_version);

    CL_Display::flip();
    CL_System::sleep(10);
    CL_System::keep_alive (50);

    // Un choix a été fait au clavier ?
    fin_boucle |= (choix != menuNULL);
  }

  if (choix != menuQUITTER) jukebox.Play("menu/ok");
  return choix;
}

//-----------------------------------------------------------------------------

// Renvoie true s'il faut quitter
menu_item Menu::Lance ()
{
  CL_System::keep_alive ();

  // Installe le pilote clavier
  keyboard_slot = CL_Keyboard::sig_key_up().connect(this, &Menu::TraiteTouche);
  mouse_slot = CL_Mouse::sig_key_up().connect(this, &Menu::TraiteClic);
  slot_quit = app.clwindow -> sig_window_close().connect(this, &Menu::SignalWM_QUIT);

  // Boucle du menu
  menu_item choix = Boucle();

  // Désinstalle le pilote
  CL_Keyboard::sig_key_up().disconnect(keyboard_slot);
  CL_Mouse::sig_key_up().disconnect(mouse_slot);
  app.clwindow -> sig_window_close().disconnect(slot_quit);

  // Renvoie le choix
  return choix;
}

//-----------------------------------------------------------------------------

// Traitement d'une touche clavier relachée
void Menu::TraiteClic (const CL_InputEvent& event)
{
  // Clic gauche ?
  if (event.id == CL_MOUSE_LEFT)
  {
    int x = CL_Mouse::get_x();
    int y = CL_Mouse::get_y();
    if (jouer.Test (x, y)) { choix = menuJOUER; return; }
    if (options.Test (x, y)) { choix = menuOPTIONS; return; }
    if (infos.Test (x, y)) { choix = menuINFOS; return; }
    if (quitter.Test (x, y)) { choix = menuQUITTER; return; }
  }
}

//-----------------------------------------------------------------------------

void Menu::TraiteTouche (const CL_InputEvent &touche)
{
  if (touche.id == CL_KEY_ENTER){ choix = menuJOUER; return; }
  if (touche.id == CL_KEY_ESCAPE) { choix = menuQUITTER; return; }
}

//-----------------------------------------------------------------------------
