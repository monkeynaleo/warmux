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

static const FigureWidget::Caption captions[] = {
  { _("Quit game"), 81, 13, DEF_CAPTIONS_PARAMS },
  { _("High jump"), 439, 326, DEF_CAPTIONS_PARAMS },
  { _("Jump backwards"), 439, 297, DEF_CAPTIONS_PARAMS },
  { _("Jump backwards"), 439, 356, DEF_CAPTIONS_PARAMS },
  { _("Drag&drop: Move camera"), 520, 395, DEF_CAPTIONS_PARAMS },
  { _("Center camera on character"), 520, 422, DEF_CAPTIONS_PARAMS },
  { _("Show/hide the interface"), 271, 43, DEF_CAPTIONS_PARAMS },
  { _("Fullscreen / window"), 491, 42, DEF_CAPTIONS_PARAMS },
  { _("Configuration menu"), 491, 72, DEF_CAPTIONS_PARAMS },
  { _("Talk in network battles"), 92, 296, DEF_CAPTIONS_PARAMS },
  { _("Change weapon category"), 81, 72, DEF_CAPTIONS_PARAMS },
  { _("Change weapon countdown"), 618, 164, DEF_CAPTIONS_PARAMS },
  { _("Change aim angle"), 618, 194, DEF_CAPTIONS_PARAMS },
  { _("Move character"), 618, 286, DEF_CAPTIONS_PARAMS },
  { _("On map: select a target"), 618, 225, DEF_CAPTIONS_PARAMS },
  { _("On a character: select it"), 618, 256, DEF_CAPTIONS_PARAMS },
  { _("Show weapons menu"), 618, 134, DEF_CAPTIONS_PARAMS },
  { _("Smaller aim angle and walk step"), 92, 326, DEF_CAPTIONS_PARAMS },
  { _("Move camera with mouse or arrows"), 386, 394, DEF_CAPTIONS_PARAMS },
  { _("Weapon: Fire / Bonus box: drop"), 260, 326, DEF_CAPTIONS_PARAMS },
  { _("Show/hide minimap"), 271, 72, DEF_CAPTIONS_PARAMS },
  { _("Change active character"), 92, 356, DEF_CAPTIONS_PARAMS },
  { _("Center camera on character"), 386, 422, DEF_CAPTIONS_PARAMS },
  { _("Quickly quit game with Ctrl"), 81, 43, DEF_CAPTIONS_PARAMS },
};

HelpMenu::HelpMenu()
  : Menu("help/background", vCancel)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);

  int window_w = GetMainWindow().GetWidth();
  int window_h = GetMainWindow().GetHeight();

  int border_x = 0.02 * window_w;
  int border_y = 0.02 * window_h;
  int max_w    = window_w - 2*border_x;
  int max_h    = window_h - actions_buttons->GetSizeY() - border_y;

  MultiTabs * tabs = new MultiTabs(Point2i(max_w, max_h));
  tabs->SetPosition(border_x, border_y);

  Widget *w = new FigureWidget(Point2i(max_w,
                                       tabs->GetSizeY() - tabs->GetHeaderHeight()),
                               "help/shortkeys",
                               captions, ARRAY_SIZE(captions),
                               PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Keyboard"), w);
  widgets.AddWidget(tabs);

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
