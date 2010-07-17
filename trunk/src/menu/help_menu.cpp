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

  float zoomx = GetMainWindow().GetWidth() * 0.95f / tmp.GetWidth();
  float zoomy = GetMainWindow().GetHeight() * 0.95f / tmp.GetHeight();
  zoom = std::min(zoomx, zoomy);
  img_keyboard = new Sprite(tmp.RotoZoom(0, zoom, zoom), true);

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

  Point2i offset = (GetMainWindow().GetSize()-img_keyboard->GetSize()) / 2;
  img_keyboard->Blit(GetMainWindow(), offset);

  const uint MIDDLE_X = 64;
  const uint MIDDLE_Y = 13;

  Text tmp("", dark_gray_color, 12*sqrtf(zoom), Font::FONT_BOLD, false);
  tmp.SetMaxWidth(132*zoom + 0.5f);

  struct {
    const char* string;
    int         x, y;
  } texts[] = {
    { _("Quit game"), 15, 1 },
    { _("High jump"), 373, 313 },
    { _("Jump backwards"), 373, 285 },
    { _("Jump backwards"), 373, 342 },
    { _("Drag&drop: Move camera"), 454, 383 },
    { _("Center camera on character"), 454, 411 },
    { _("Show/hide the interface"), 205, 31 },
    { _("Fullscreen / window"), 425, 31 },
    { _("Configuration menu"), 425, 61 },
    { _("Talk in network battles"), 26, 285 },
    { _("Change weapon category"), 15, 61 },
    { _("Change weapon countdown"), 552, 153 },
    { _("Change aim angle"), 552, 182 },
    { _("Move character"), 552, 275 },
    { _("On map: select a target"), 552, 214 },
    { _("On a character: select it"), 552, 245 },
    { _("Show weapons menu"), 552, 122 },
    { _("Smaller aim angle and walk step"), 26, 315 },
    { _("Move camera with mouse or arrows"), 320, 383 },
    { _("Weapon: Fire / Bonus box: drop"), 194, 315 },
    { _("Show/hide minimap"), 205, 61 },
    { _("Change active character"), 26, 345 },
    { _("Center camera on character"), 320, 411 },
    { _("Quickly quit game with Ctrl"), 15, 31 },
    { NULL, 0, 0 }
  };

  int i = 0;
  while (texts[i].string) {
    tmp.SetText(texts[i].string);
    tmp.DrawCenter(Point2i((texts[i].x+MIDDLE_X)*zoom + offset.x + 0.5f,
                           (texts[i].y+MIDDLE_Y)*zoom + offset.y + 0.5f));
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
