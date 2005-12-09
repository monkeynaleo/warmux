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
 * Options menu
 *****************************************************************************/

#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
//-----------------------------------------------------------------------------


// TODO: remove gui items from here as they are only nedded in .cpp file
//       and not needed in the .h interface file of the menu
class Button;
class ListBox;
class CheckBox;
class SpinButton;
class SDL_Surface;
class Sprite;
class Box;

class OptionMenu
{
 public:
   OptionMenu(); 
   ~OptionMenu();

   void Lance ();
   void Init ();
   void Reset ();

 private:
   bool fin_boucle;
   
   /* Actions buttons  */
   Button *valider; 
   Button *enregistrer;
   Button *annuler;
   Box *actions_buttons;

   /* Graphic options controllers */   
   ListBox *lboxVideoMode;
   CheckBox *option_display_wind_particles;
   CheckBox *option_affichage_energie;
   CheckBox *option_affichage_nom;
   CheckBox *full_screen;    
   Box *graphic_options;

   /* Sound options controllers */
   ListBox *lboxSoundFreq;
   CheckBox *opt_music;
   CheckBox *opt_sound_effects;
   Box *sound_options;

   /* Game options controllers */
   SpinButton *option_temps_tour;
   SpinButton *option_temps_fin_tour;
   SpinButton *option_nb_ver;
   SpinButton *option_energie_ini;
   Box *game_options;

   /* Team controllers */
   ListBox *lboxTeams;   

   /* Map controllers */
   ListBox *lboxMaps;
   Sprite *map_preview;  

   bool m_init;

   Sprite *fond_option; 
   SDL_Surface *fond_maps;
   SDL_Surface *fond_box;
   SDL_Surface *fond_box2;

   uint carte_x, carte_y, carte_larg, carte_haut;
   uint maps_x, maps_y;
   uint teams_x, teams_y;
   uint espace;

   void ChangeTerrain();
   void EnregistreOptions();
   void onClick ( int mouse_x, int mouse_y);
};

#endif
