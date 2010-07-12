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
#include "menu/help_menu.h"
#include "game/config.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

HelpMenu::HelpMenu()
  : Menu("help/background", vOk)
  , zoom(1.0f)
  , img_keyboard(NULL)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  Surface tmp  = GetResourceManager().LoadImage(res, "help/shortkeys", true);
  if (GetMainWindow().GetWidth() < tmp.GetWidth()+10 ||
      GetMainWindow().GetHeight() < tmp.GetHeight()+10) {
    float zoomx = GetMainWindow().GetWidth() / float(tmp.GetWidth()+10);
    float zoomy = GetMainWindow().GetHeight() / float(tmp.GetHeight()+10);
    zoom = std::min(zoomx, zoomy);
    img_keyboard = new Sprite(tmp.RotoZoom(0, zoom, zoom), true);
  } else {
    img_keyboard = new Sprite(tmp, true);
  }
  img_keyboard->cache.EnableLastFrameCache();
  GetResourceManager().UnLoadXMLProfile(res);
}

HelpMenu::~HelpMenu()
{
  if (NULL != img_keyboard) {
    delete img_keyboard;
  }
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

  int border_x = int(GetMainWindow().GetWidth() - img_keyboard->GetWidth()) / 2;
  if (border_x < 0)
    border_x = 0;

  int border_y = int(GetMainWindow().GetHeight() - img_keyboard->GetHeight()) / 2;
  if (border_y < 0)
    border_y = 0;

  img_keyboard->Blit(GetMainWindow(), border_x, border_y);

  const uint MIDDLE_X = 64;
  const uint MIDDLE_Y = 13;

  Text tmp("", dark_gray_color, Font::FONT_TINY, Font::FONT_BOLD, false);
  tmp.SetMaxWidth(130*zoom + 0.5f);

  struct {
    const char* string;
    int         x, y;
  } texts[] = {
    { _("Quit game"), 15, 1 },
    { _("High jump"), 373, 313 },
    { _("Jump backwards"), 373, 284 },
    { _("Jump backwards"), 373, 342 },
    { _("Drag&drop: Move the camera"), 454, 380 },
    { _("Click: Center the camera on the character"), 454, 410 },
    { _("Show/hide the interface"), 205, 31 },
    { _("Fullscreen / window"), 425, 30 },
    { _("Configuration menu"), 425, 59 },
    { _("Talk in network battles"), 26, 284 },
    { _("Change the weapon category"), 15, 60 },
    { _("Change the weapon countdown"), 552, 153 },
    { _("Change the aim angle"), 552, 182 },
    { _("Move the character"), 552, 274 },
    { _("On map: Select a target"), 552, 213 },
    { _("On a character: Select it"), 552, 244 },
    { _("Show the weapons menu"), 552, 121 },
    { _("Smaller aim angle and walk step"), 26, 314 },
    { _("Move the camera with mouse or arrows"), 320, 380 },
    { _("Weapon: Fire / Bonus box: fall fast"), 194, 313 },
    { _("Show/hide the minimap"), 205, 60 },
    { _("Change the active character"), 26, 343 },
    { _("Center the camera on the character"), 320, 410 },
    { _("Quickly quit game with Ctrl"), 15, 30 },
    { NULL, 0, 0 }
  };

  int i = 0;
  while (texts[i].string) {
    tmp.SetText(texts[i].string);
    tmp.DrawCenter(Point2i((texts[i].x+MIDDLE_X)*zoom + 0.5f + border_x,
                           (texts[i].y+MIDDLE_Y)*zoom + 0.5f + border_y));
    i++;
  }
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
