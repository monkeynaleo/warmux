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

#define DEF_KEY_CAPTIONS_PARAMS 130, (Font::font_size_t)14, DEF_CAPTIONS_PARAMS
  static const FigureWidget::Caption controls_captions[] = {
    { _("Quit game"), 81, 13, DEF_KEY_CAPTIONS_PARAMS },
    { _("High jump"), 439, 326, DEF_KEY_CAPTIONS_PARAMS },
    { _("Jump backwards"), 439, 297, DEF_KEY_CAPTIONS_PARAMS },
    { _("Jump backwards"), 439, 356, DEF_KEY_CAPTIONS_PARAMS },
    { _("Drag&drop: Move camera"), 520, 395, DEF_KEY_CAPTIONS_PARAMS },
    { _("Center camera on character"), 520, 422, DEF_KEY_CAPTIONS_PARAMS },
    { _("Show/hide the interface"), 271, 43, DEF_KEY_CAPTIONS_PARAMS },
    { _("Toggle fullscreen"), 491, 42, DEF_KEY_CAPTIONS_PARAMS },
    { _("Configuration menu"), 491, 72, DEF_KEY_CAPTIONS_PARAMS },
    { _("Talk in network battles"), 92, 296, DEF_KEY_CAPTIONS_PARAMS },
    { _("Change weapon category"), 81, 72, DEF_KEY_CAPTIONS_PARAMS },
    { _("Change weapon countdown"), 618, 164, DEF_KEY_CAPTIONS_PARAMS },
    { _("Change aim angle"), 618, 194, DEF_KEY_CAPTIONS_PARAMS },
    { _("Move character"), 618, 286, DEF_KEY_CAPTIONS_PARAMS },
    { _("On map: select a target"), 618, 225, DEF_KEY_CAPTIONS_PARAMS },
    { _("On a character: select it"), 618, 256, DEF_KEY_CAPTIONS_PARAMS },
    { _("Show weapons menu"), 618, 134, DEF_KEY_CAPTIONS_PARAMS },
    { _("Smaller aim angle and walk step"), 92, 326, DEF_KEY_CAPTIONS_PARAMS },
    { _("Move camera with mouse or arrows"), 386, 394, DEF_KEY_CAPTIONS_PARAMS },
    { _("Weapon: Fire / Bonus box: drop"), 260, 326, DEF_KEY_CAPTIONS_PARAMS },
    { _("Toggle minimap"), 271, 72, DEF_KEY_CAPTIONS_PARAMS },
    { _("Change active character"), 92, 356, DEF_KEY_CAPTIONS_PARAMS },
    { _("Center camera on character"), 386, 422, DEF_KEY_CAPTIONS_PARAMS },
    { _("Quickly quit game with Ctrl"), 81, 43, DEF_KEY_CAPTIONS_PARAMS },
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
    { _("Click to change team selected"), 293, 141, DEF_STARTGAME_CAPTIONS_PARAMS },
    { _("Edit player name"), 541, 141, DEF_STARTGAME_CAPTIONS_PARAMS },
    { _("Go to game rules editing submenu"), 788, 141, DEF_STARTGAME_CAPTIONS_PARAMS },
    { _("Click to select AI/human team"), 325, 275, DEF_STARTGAME_CAPTIONS_PARAMS },
    { _("Change per team number of players"), 605, 275, DEF_STARTGAME_CAPTIONS_PARAMS },
    { _("Change number of playing teams"), 146, 334, DEF_STARTGAME_CAPTIONS_PARAMS },
    { _("Click/wheelmouse to change map"), 466, 334, DEF_STARTGAME_CAPTIONS_PARAMS },
  };
  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/startgame_menu",
                       startgame_captions, ARRAY_SIZE(startgame_captions),
                       PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Game menu"), w);

# define DEF_GAMEMODE_CAPTIONS_PARAMS 252, Font::FONT_LARGE, DEF_CAPTIONS_PARAMS
  static const FigureWidget::Caption gamemode_captions[] = {
    { _("Change some game rules"), 206, 48, DEF_GAMEMODE_CAPTIONS_PARAMS },
    { _("How long you have to play per turn"), 483, 48, DEF_GAMEMODE_CAPTIONS_PARAMS },
    { _("When a character can be selected"), 761, 48, DEF_GAMEMODE_CAPTIONS_PARAMS },
    { _("Initial life points"), 1038, 48, DEF_GAMEMODE_CAPTIONS_PARAMS },
    { _("Maximum energy for one character"), 206, 688, DEF_GAMEMODE_CAPTIONS_PARAMS },
    { _("Duration until death mode"), 482, 688, DEF_GAMEMODE_CAPTIONS_PARAMS },
    { _("Damage per turn during death mode"), 761, 688, DEF_GAMEMODE_CAPTIONS_PARAMS },
    { _("The bigger the faster you'll fall!"), 1042, 688, DEF_GAMEMODE_CAPTIONS_PARAMS },
  };
  w = new FigureWidget(Point2i(max_w,
                               tabs->GetSizeY() - tabs->GetHeaderHeight()),
                       "help/game_mode",
                       gamemode_captions, ARRAY_SIZE(gamemode_captions),
                       PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Game mode"), w);

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
