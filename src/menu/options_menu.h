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


// TODO: remove gui items from here as they are only nedded in .cpp file
//       and not needed in the .h interface file of the menu
class Button;
class ListBox;
class CheckBox;
class SpinButton;
class SDL_Surface;
class Sprite;

class OptionMenu
{
 public:
   OptionMenu(); 
   void Lance ();
   void Init ();
   void Reset ();

 private:
   bool fin_boucle;
   
   Button *valider; 
   Button *enregistrer;
   Button *annuler;

   ListBox *lboxVideoMode;
   ListBox *lboxMaps;
   ListBox *lboxTeams;
   ListBox *lboxSoundFreq;

   CheckBox *option_affichage_energie;
   CheckBox *option_affichage_nom;
   CheckBox *full_screen;
   CheckBox *opt_sound;
   CheckBox *opt_music;
   CheckBox *opt_sound_effects;

   SpinButton *option_temps_tour;
   SpinButton *option_temps_fin_tour;
   SpinButton *option_nb_ver;
   SpinButton *option_energie_ini;

   bool m_init;
#ifdef CL
   CL_Surface map_preview;
   CL_Surface fond_option, fond_maps, fond_box, fond_box2;
#else
   Sprite *map_preview;
   Sprite *fond_option; 
   SDL_Surface *fond_maps;
   SDL_Surface *fond_box;
   SDL_Surface *fond_box2;
#endif
   uint carte_x, carte_y, carte_larg, carte_haut;
   uint maps_x, maps_y;
   uint teams_x, teams_y;
   uint espace;

/* CL_Slot keyboard_slot, mouse_slot, slot_quit; */

   void ChangeTerrain();
   void EnregistreOptions();
#ifdef CL
   void TraiteClic (const CL_InputEvent &event);
#else
   void onClick ( int mouse_x, int mouse_y);
#endif
/*   void TraiteTouche (const CL_InputEvent &event); */
/*   void SignalWM_QUIT (); */
/*   void RetourMenuPrincipal (); */
};

extern OptionMenu options_menu;

#endif
