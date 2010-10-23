/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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

#define DEF_KEY_CAPTIONS_PARAMS 132, (Font::font_size_t)14, DEF_CAPTIONS_PARAMS
  static const FigureWidget::Caption controls_captions[] = {
    { _("Quit game"), 81, 13, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("High jump"), 439, 326, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Jump backwards"), 439, 297, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Jump backwards"), 439, 356, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Drag&drop: Move camera"), 520, 395, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Center camera on character"), 520, 422, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Show/hide the interface"), 271, 43, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Toggle fullscreen"), 491, 42, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Configuration menu"), 491, 72, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Talk in network battles"), 92, 296, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Change weapon category"), 81, 72, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Change weapon countdown"), 618, 164, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Change aim angle"), 618, 194, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Move character"), 618, 286, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("On map: select a target"), 618, 225, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("On a character: select it"), 618, 256, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Show weapons menu"), 618, 134, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Smaller aim angle and walk step"), 92, 326, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Move camera with mouse or arrows"), 386, 394, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Weapon: Fire / Bonus box: drop"), 260, 326, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Toggle minimap"), 271, 72, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Change active character"), 92, 356, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Center camera on character"), 386, 422, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Quickly quit game with Ctrl"), 81, 43, DEF_KEY_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
  };
  Widget *w = new FigureWidget(Point2i(max_w,
                                       tabs->GetSizeY() - tabs->GetHeaderHeight()),
                               "help/shortkeys",
                               controls_captions, ARRAY_SIZE(controls_captions),
                               PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Keyboard"), w);
  widgets.AddWidget(tabs);

# define DEF_STARTGAME_CAPTIONS_PARAMS 238, (Font::font_size_t)22, DEF_CAPTIONS_PARAMS
  static const FigureWidget::Caption startgame_captions[] = {
    { _("Click to change team selected"), 293, 141, DEF_STARTGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Edit player name"), 541, 141, DEF_STARTGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Go to game rules editing submenu"), 788, 141, DEF_STARTGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Click to select AI/human team"), 325, 275, DEF_STARTGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Change per team number of players"), 605, 275, DEF_STARTGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Change number of playing teams"), 146, 334, DEF_STARTGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Click/wheelmouse to change map"), 466, 334, DEF_STARTGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
  };
  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/startgame_menu",
                       startgame_captions, ARRAY_SIZE(startgame_captions),
                       PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Game menu"), w);

# define DEF_GAMEMODE_CAPTIONS_PARAMS 252, Font::FONT_LARGE, DEF_CAPTIONS_PARAMS
  static const FigureWidget::Caption gamemode_captions[] = {
    { _("Experiment new game rules"), 206, 45, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Increase this if you are a slow player"), 483, 45, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("When a character can be switched"), 761, 45, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("More energy, more shots needed"), 1038, 45, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("If you like medkits, increase this"), 206, 668, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Increase this if you like long games"), 482, 668, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Increase this for faster death mode end"), 761, 668, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("The bigger the faster you'll fall!"), 1042, 668, DEF_GAMEMODE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
  };
  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/game_mode",
                       gamemode_captions, ARRAY_SIZE(gamemode_captions),
                       PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Game mode"), w);

#define INGAME_CAPTIONS_PARAMS Font::FONT_LARGE, DEF_CAPTIONS_PARAMS
  static const FigureWidget::Caption ingame_captions[] = {
    {
#ifdef ANDROID
      _("To jump, press space or trackball or dpad center.\n"
        "To aim or move more slowly, use shift/vol+.\n"
        "To pause, click the clock or press escape/back key.\n"
        "Click a character from your team to select it.\n"),
      // TRANSLATORS: please keep this help for Android smartphones short!
#else
      _("To jump, press space.\n"
        "To aim or move more slowly, use shift.\n"
        "To pause, click the clock or press escape.\n"
        "Click a character from your team to select it.\n"),
      // TRANSLATORS: please be imaginative and keep it short!
#endif
      197, 337, 289, INGAME_CAPTIONS_PARAMS },
    { _("Minimap, more details in another tab"), 543, 132, 330, INGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Gauge to set speed of some projectiles"), 619, 494, 411, INGAME_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Interface, more details in another tab"), 1003, 437, 209, INGAME_CAPTIONS_PARAMS } // TRANSLATORS: please be imaginative and keep it short!
  };
  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/ingame",
                       ingame_captions, ARRAY_SIZE(ingame_captions),
                       PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Ingame display"), w);

#define INTERFACE_CAPTIONS_PARAMS Font::FONT_LARGE, DEF_CAPTIONS_PARAMS
  static const FigureWidget::Caption interface_captions[] = {
    { _("Character name.\nCharacter energy.\nTeam name.\nPlayer name.\n"),
      163, 101, 302, INTERFACE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Time left in this turn.\nTime elapsed since game start.\n"),
      473, 101, 262, INTERFACE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Team in ordre of decreasing energy"), 763, 103, 200, INTERFACE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Selected weapon and ammo left"), 352, 635, 255, INTERFACE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Wind strength and direction"), 616, 635, 245, INTERFACE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
    { _("Fire your weapon!"), 866, 635, 221, INTERFACE_CAPTIONS_PARAMS }, // TRANSLATORS: please be imaginative and keep it short!
  };
  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/interface",
                       interface_captions, ARRAY_SIZE(interface_captions),
                       PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Interface"), w);

  w = new ControlConfig(tabs->GetSize(), true);
  tabs->AddNewTab("unused", _("Current controls"), w);

  GetResourceManager().UnLoadXMLProfile(res);
  widgets.Pack();
}

HelpMenu::~HelpMenu()
{

}

bool HelpMenu::signal_ok()
{
  return true;
}

bool HelpMenu::signal_cancel()
{
  return true;
}

void HelpMenu::Draw(const Point2i& /*mousePosition*/)
{
}

void HelpMenu::OnClick(const Point2i &mousePosition, int button)
{
  widgets.Click(mousePosition, button);
}

void HelpMenu::OnClickUp(const Point2i &mousePosition, int button)
{
  widgets.ClickUp(mousePosition, button);
}

//-----------------------------------------------------------------------------
