/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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

#include "menu.h"

class ListBox;
class CheckBox;
class ComboBox;
class SpinButtonWithPicture;

class OptionMenu : public Menu
{
public:
   OptionMenu();
   ~OptionMenu();

private:

  /* If you need this, implement it (correctly)*/
   OptionMenu(const OptionMenu&);
   OptionMenu operator=(const OptionMenu&);
   /********************************************/

   /* Graphic options controllers */
   ComboBox *cbox_video_mode;
   ListBox *lbox_languages;
   CheckBox *opt_display_wind_particles;
   CheckBox *opt_display_energy;
   CheckBox *opt_display_name;
   CheckBox *full_screen;
   SpinButtonWithPicture *opt_max_fps;

   /* Sound options controllers */
   ComboBox *cbox_sound_freq;
   CheckBox *opt_music;
   CheckBox *opt_sound_effects;

   void SaveOptions();
   void OnClick(const Point2i &mousePosition, int button);
   void OnClickUp(const Point2i &mousePosition, int button);
   void Draw(const Point2i &mousePosition);

   bool signal_ok();
   bool signal_cancel();
};

#endif
