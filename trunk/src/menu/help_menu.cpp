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
  { _("Quit game"), 81, 13 },
  { _("High jump"), 439, 326 },
  { _("Jump backwards"), 439, 297 },
  { _("Jump backwards"), 439, 356 },
  { _("Drag&drop: Move camera"), 520, 395 },
  { _("Center camera on character"), 520, 422 },
  { _("Show/hide the interface"), 271, 43 },
  { _("Fullscreen / window"), 491, 42 },
  { _("Configuration menu"), 491, 72 },
  { _("Talk in network battles"), 92, 296 },
  { _("Change weapon category"), 81, 72 },
  { _("Change weapon countdown"), 618, 164 },
  { _("Change aim angle"), 618, 194 },
  { _("Move character"), 618, 286 },
  { _("On map: select a target"), 618, 225 },
  { _("On a character: select it"), 618, 256 },
  { _("Show weapons menu"), 618, 134 },
  { _("Smaller aim angle and walk step"), 92, 326 },
  { _("Move camera with mouse or arrows"), 386, 394 },
  { _("Weapon: Fire / Bonus box: drop"), 260, 326 },
  { _("Show/hide minimap"), 271, 72 },
  { _("Change active character"), 92, 356 },
  { _("Center camera on character"), 386, 422 },
  { _("Quickly quit game with Ctrl"), 81, 43 },
};

HelpMenu::HelpMenu()
  : Menu("help/background", vNo)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);

  int window_w = GetMainWindow().GetWidth();
  int window_h = GetMainWindow().GetHeight();

  int border   = 0.02 * window_w;
  int max_w    = window_w - 2*border;

  MultiTabs * tabs =
    new MultiTabs(Point2i(max_w, window_h - 2*border));
  tabs->SetPosition(border, border);

  Widget *w = new FigureWidget(Point2i(max_w,
                                       tabs->GetSizeY() - tabs->GetHeaderHeight()),
                               "help/shortkeys",
                               captions, ARRAY_SIZE(captions),
                               12, PictureWidget::FIT_SCALING);
  tabs->AddNewTab("unused", _("Keyboard"), w);
  widgets.AddWidget(tabs);

  w = new ControlConfig(tabs->GetSize() - 10, true, false);
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
