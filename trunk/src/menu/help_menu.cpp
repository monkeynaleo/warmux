/******************************************************************************
 *  Warmux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Warmux Team.
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
 * Help Menu
 *****************************************************************************/

#include "include/app.h"
#include "graphic/text.h"
#include "graphic/sprite.h"
#include "graphic/video.h"
#include "gui/control_config.h"
#include "gui/figure_widget.h"
#include "gui/tabs.h"
#include "menu/help_menu.h"
#include "game/config.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

HelpMenu::HelpMenu()
  : Menu("help/background", vCancel)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);

  int window_w = GetMainWindow().GetWidth();
  int window_h = GetMainWindow().GetHeight();

  float factor = (window_w < 640) ? 0.02f : 0.05f;
  int border   = window_w * factor;
  int max_w    = window_w - 2*border;
  int max_h    = window_h - actions_buttons->GetSizeY() - border;

  MultiTabs * tabs = new MultiTabs(Point2i(max_w, max_h));
  tabs->SetPosition(border, border);

  tabs->AddNewTab("unused", _("Current controls"),
                  new ControlConfig(tabs->GetSize(), true));

  FigureWidget *w;
#ifndef ANDROID
  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/shortkeys", (Font::font_size_t)14);
  w->AddCaption(_("Quit game"), 81, 13, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("High jump"), 439, 326, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Jump backwards"), 439, 297, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Jump backwards"), 439, 356, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Drag&drop: Move camera"), 520, 395, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Center camera on character"), 520, 422, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Show/hide the interface"), 271, 43, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Toggle fullscreen"), 491, 42, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Configuration menu"), 491, 72, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Talk in network battles"), 92, 296, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Change weapon category"), 81, 72, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Change weapon countdown"), 618, 164, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Change aim angle"), 618, 194, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Move character"), 618, 286, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("On map: select a target"), 618, 225, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("On a character: select it"), 618, 256, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Show weapons menu"), 618, 134, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Smaller aim angle and walk step"), 92, 326, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Move camera with mouse or arrows"), 386, 394, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Weapon: Fire / Bonus box: drop"), 260, 326, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Toggle minimap"), 271, 72, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Change active character"), 92, 356, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Center camera on character"), 386, 422, 132); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Quickly quit game with Ctrl"), 81, 43, 132); // TRANSLATORS: please be imaginative and keep it short!
  tabs->AddNewTab("unused", _("Keyboard"), w);
#endif

  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/startgame_menu", Font::FONT_BIG);
  w->AddCaption(_("Select number of playing teams"), 220, 28, 432); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Change number of players per team"), 618, 34, 361); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Click/wheelmouse to change the team.\nYou can't select the same team several times."), 167, 176, 325);
  w->AddCaption(_("Click to let a player handle the team instead of the AI, and vice versa"), 576, 187, 445); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Map author information: name, email, nickname, country"), 511, 263, 557); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Click on either side of the central (selected) map or use wheelmouse to change the selected map."), 388, 324, 753);
  tabs->AddNewTab("unused", _("Game menu"), w);


  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/minimap", Font::FONT_BIG);
  w->AddCaption(_("Area currently displayed"), 205, 270, 405);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Position of the characters.\nA circle surrounds the active character."), 610, 335, 375);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Position of bonus boxes or medkits"), 205, 403, 405);// TRANSLATORS: please be imaginative and keep it short!
  tabs->AddNewTab("unused", _("Minimap"), w);

  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/game_mode", Font::FONT_BIG);
  w->AddCaption(_("Experiment new game rules"), 192, 71, 381);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Increase this if you are a slow player"), 193, 25, 381);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("When a character can be switched"), 607, 25, 381);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("More energy, more shots needed"), 607, 71, 379);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("If you like medkits,\nincrease this"), 191, 361, 379);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Increase this if you like long games"), 191, 407, 381);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Increase this for faster death mode end"), 606, 407, 381);// TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("The bigger the faster you'll fall!"), 607, 361, 381);// TRANSLATORS: please be imaginative and keep it short!
  tabs->AddNewTab("unused", _("Game mode"), w);

  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/ingame", Font::FONT_LARGE);
  w->AddCaption(_("Minimap, more details in another tab"), 313, 55, 595); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Interface and touch controls, more details in other tabs"), 399, 332, 737); // TRANSLATORS: please be imaginative and keep it short!
  tabs->AddNewTab("unused", _("Ingame display"), w);

  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/interface", Font::FONT_LARGE);
  w->AddCaption(_("Character name.\nCharacter energy.\nTeam name.\nPlayer name."), 163, 101, 302); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Time left in this turn.\nTime elapsed since game start."), 473, 101, 262); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Teams in decreasing order of energy"), 763, 103, 200); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Selected weapon and ammo left"), 352, 635, 255); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Wind strength and direction"), 616, 635, 245); // TRANSLATORS: please be imaginative and keep it short!
  w->AddCaption(_("Fire your weapon!"), 866, 635, 221); // TRANSLATORS: please be imaginative and keep it short!
  tabs->AddNewTab("unused", _("Interface"), w);

  GetResourceManager().UnLoadXMLProfile(res);
  widgets.AddWidget(tabs);
  widgets.Pack();
}
