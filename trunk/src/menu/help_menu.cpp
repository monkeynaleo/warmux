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
#include "gui/tabs.h"
#include "gui/picture_widget.h"
#include "menu/help_menu.h"
#include "game/config.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

HelpMenu::HelpMenu()
  : Menu("help/background", vOk)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);

  int window_w = GetMainWindow().GetWidth();
  int window_h = GetMainWindow().GetHeight();

  int border   = 0.02 * window_w;
  int max_w    = window_w - 2*border;


  MultiTabs * tabs =
    new MultiTabs(Point2i(max_w, window_h -actions_buttons->GetSizeY() -border));
  tabs->SetPosition(border, border);

  m_keyboard = new PictureWidget(Point2i(max_w,
                                         tabs->GetSizeY() - tabs->GetHeaderHeight()),
                                 "help/shortkeys", PictureWidget::FIT_SCALING);

  tabs->AddNewTab("unused", _("Keyboard"), m_keyboard);
  widgets.AddWidget(tabs);

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

void HelpMenu::Draw(const Point2i& mousePosition)
{
  widgets.Draw(mousePosition);
  Point2f zoom = m_keyboard->GetScale();

  Text tmp("", dark_gray_color, 12*zoom.GetX(), Font::FONT_BOLD, false);
  tmp.SetMaxWidth(130*zoom.GetX());

  struct {
    const std::string string; // gcc does not support correctly a char* here.
    int x, y;
  } texts[] = {
    { _("Quit game"), 81, 12 },
    { _("High jump"), 439, 324 },
    { _("Jump backwards"), 439, 296 },
    { _("Jump backwards"), 439, 353 },
    { _("Drag&drop: Move camera"), 520, 394 },
    { _("Center camera on character"), 520, 422 },
    { _("Show/hide the interface"), 271, 42 },
    { _("Fullscreen / window"), 491, 42 },
    { _("Configuration menu"), 491, 72 },
    { _("Talk in network battles"), 92, 296 },
    { _("Change weapon category"), 81, 72 },
    { _("Change weapon countdown"), 618, 164 },
    { _("Change aim angle"), 618, 193 },
    { _("Move character"), 618, 286 },
    { _("On map: select a target"), 618, 225 },
    { _("On a character: select it"), 618, 256 },
    { _("Show weapons menu"), 618, 133 },
    { _("Smaller aim angle and walk step"), 92, 326 },
    { _("Move camera with mouse or arrows"), 386, 394 },
    { _("Weapon: Fire / Bonus box: drop"), 260, 326 },
    { _("Show/hide minimap"), 271, 72 },
    { _("Change active character"), 92, 356 },
    { _("Center camera on character"), 386, 422 },
    { _("Quickly quit game with Ctrl"), 81, 42 },
    { "", 0, 0 }
  };

  int i = 0;
  while (texts[i].x != 0 && texts[i].y != 0) {
    tmp.SetText(texts[i].string);
    Point2i pos = Point2i(zoom.x * texts[i].x, zoom.y * texts[i].y)
                + m_keyboard->GetPicturePosition();

    tmp.DrawCenter(pos);
    i++;
  }
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
