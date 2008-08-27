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
 * Help Menu
 *****************************************************************************/

#include "include/app.h"
#include "graphic/text.h"
#include "graphic/sprite.h"
#include "graphic/video.h"
#include "menu/help_menu.h"
#include "game/config.h"
#include "tool/i18n.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

static const uint BORDER        = 20;
static const uint CHECKBOX_SIZE = 50;

HelpMenu::HelpMenu()  :
  Menu("help/background", vOk)
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  img_keyboard = new Sprite(resource_manager.LoadImage(res, "help/shortkeys"), true);
  img_keyboard->cache.EnableLastFrameCache();
  resource_manager.UnLoadXMLProfile(res);
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

void HelpMenu::DrawBackground()
{
  Menu::DrawBackground();

  img_keyboard->Blit(GetMainWindow(), BORDER, BORDER);

  Text tmp(_("Quit game"), dark_gray_color, Font::FONT_SMALL, Font::FONT_NORMAL, false);
  tmp.SetMaxWidth(130);

  tmp.DrawTopLeft(Point2i(12+BORDER, 31));
  tmp.Set(_("Show/hide interface")); tmp.DrawTopLeft(Point2i(147+BORDER, 31));
  tmp.Set(_("Fullscreen / window")); tmp.DrawTopLeft(Point2i(311+BORDER, 31));
  tmp.Set(_("High jump")); tmp.DrawTopLeft(Point2i(459+BORDER, 31));
  tmp.Set(_("Talk in network battle")); tmp.DrawTopLeft(Point2i(12+BORDER, 59));
  tmp.Set(_("Change weapon category")); tmp.DrawTopLeft(Point2i(147+BORDER, 59));
  tmp.Set(_("Configuration menu")); tmp.DrawTopLeft(Point2i(311+BORDER, 59));
  tmp.Set(_("Jump")); tmp.DrawTopLeft(Point2i(459+BORDER, 59));
  tmp.Set(_("Drag&drop: Move camera")); tmp.DrawTopLeft(Point2i(539+BORDER, 108));
  tmp.Set(_("Click: Center camera on character")); tmp.DrawTopLeft(Point2i(539+BORDER, 137));
  tmp.Set(_("Change weapon countdown")); tmp.DrawTopLeft(Point2i(539+BORDER, 166));
  tmp.Set(_("Change aim angle")); tmp.DrawTopLeft(Point2i(539+BORDER, 195));
  tmp.Set(_("Move character")); tmp.DrawTopLeft(Point2i(539+BORDER, 224));
  tmp.Set(_("On map: Select a target")); tmp.DrawTopLeft(Point2i(539+BORDER, 253));
  tmp.Set(_("On a character: Changes active one")); tmp.DrawTopLeft(Point2i(539+BORDER, 281));
  tmp.Set(_("Show weapons menu")); tmp.DrawTopLeft(Point2i(539+BORDER, 310));
  tmp.Set(_("Smaller aim angle and walk step")); tmp.DrawTopLeft(Point2i(25+BORDER, 271));
  tmp.Set(_("Jump backwards")); tmp.DrawTopLeft(Point2i(188+BORDER, 271));
  tmp.Set(_("Pause")); tmp.DrawTopLeft(Point2i(338+BORDER, 271));
  tmp.Set(_("Move camera with mouse or arrows")); tmp.DrawTopLeft(Point2i(25+BORDER, 308));
  tmp.Set(_("Weapon: Fire Bonus box: falls fast")); tmp.DrawTopLeft(Point2i(188+BORDER, 308));
  tmp.Set(_("Show/hide minimap")); tmp.DrawTopLeft(Point2i(338+BORDER, 308));
  tmp.Set(_("Change active character")); tmp.DrawTopLeft(Point2i(25+BORDER, 336));
  tmp.Set(_("Center camera to character")); tmp.DrawTopLeft(Point2i(188+BORDER, 336));
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
