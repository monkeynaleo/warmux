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

   void Run ();
   void Init ();
   void Reset ();

 private:
   bool close_menu;
   
   /* Actions buttons  */
   Button *b_ok;
   Button *b_record;
   Button *b_cancel;
   Box *actions_buttons;

   /* Graphic options controllers */   
   ListBox *lboxVideoMode;
   CheckBox *opt_display_wind_particles;
   CheckBox *opt_display_energy;
   CheckBox *opt_display_name;
   CheckBox *full_screen;    
   Box *graphic_options;

   /* Sound options controllers */
   ListBox *lboxSoundFreq;
   CheckBox *opt_music;
   CheckBox *opt_sound_effects;
   Box *sound_options;

   /* Game options controllers */
   SpinButton *opt_duration_turn;
   SpinButton *opt_duration_end_turn;
   SpinButton *opt_nb_characters;
   SpinButton *opt_energy_ini;
   Box *game_options;

   /* Team controllers */
   ListBox *lboxTeams;   

   /* Map controllers */
   ListBox *lboxMaps;
   Sprite *map_preview;  

   bool m_init;

   Sprite *bg_option;
   SDL_Surface *bg_long_box;
   SDL_Surface *bg_small_box;

   void ChangeMap();
   void SaveOptions();
   void onClick ( int mouse_x, int mouse_y);
};

#endif
