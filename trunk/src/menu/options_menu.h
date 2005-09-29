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

#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include <string>
//-----------------------------------------------------------------------------

class OptionMenu
{

 public:
  Run();
  FreeMem();
}


/* class OptionMenu */
/* { */
/* private: */
/*   volatile bool fin_boucle; */
/*   Button valider, enregistrer, annuler; */
/* #ifdef BUGGY_CODE */
/*   ListBox lboxVideoMode; */
/* #endif */
/*   ListBox lboxMaps, lboxTeams,  lboxSoundFreq; */

/*   CheckBox option_affichage_energie, option_affichage_nom; */
/* #ifdef BUGGY_CODE */
/*   CheckBox full_screen */
/* #endif */
/* #ifdef USE_SDL */
/*   CheckBox use_sdl; */
/*   bool old_use_sdl; */
/* #endif */
/*   CheckBox opt_sound, opt_music, opt_sound_effects; */

/*   SpinButton option_temps_tour, option_temps_fin_tour, option_nb_ver, option_energie_ini; */
/*   bool m_init; */
/*   CL_Surface map_preview; */
/*   CL_Surface fond_option, fond_maps, fond_box, fond_box2; */
/*   uint carte_x, carte_y, carte_larg, carte_haut; */
/*   uint maps_x, maps_y; */
/*   uint teams_x, teams_y; */
/*   uint espace; */

/*   CL_Slot keyboard_slot, mouse_slot, slot_quit; */

/* public: */
/*   OptionMenu() ; */
/*   void Lance (); */
/*   void Init ();  */
/*   void Reset ();  */

/*  private: */
/*   void ChangeTerrain(); */
/*   void TraiteTouche (const CL_InputEvent &event); */
/*   void TraiteClic (const CL_InputEvent &event); */
/*   void SignalWM_QUIT (); */
/*   void RetourMenuPrincipal (); */
/*   void EnregistreOptions(); */
/* }; */

extern OptionMenu options_menu;
//-----------------------------------------------------------------------------
#endif
