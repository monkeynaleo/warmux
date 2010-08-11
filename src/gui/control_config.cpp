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
 * Widget to configure controls
 *****************************************************************************/

#include "game/config.h"
#include "gui/check_box.h"
#include "gui/horizontal_box.h"
#include "gui/label.h"
#include "gui/null_widget.h"
#include "gui/select_box.h"
#include "interface/keyboard.h"

#include "control_config.h"

#define MIN_ACTION_WIDTH  276
#define MIN_KEY_WIDTH      70
#define SPACING_WIDTH       8
#define CHECKBOX_WIDTH     18

class ControlItem : public HBox
{
  Label *label_action, *label_key;
  CheckBox *shift_box, *alt_box, *ctrl_box;

public:
  ManMachineInterface::Key_t key_action;
  int  key_value;
  bool shift;
  bool alt;
  bool ctrl;

  ControlItem(ManMachineInterface::Key_t action,
              uint height, bool force_widget_size = true)
    : HBox(height, false, force_widget_size)
    , key_action(action)
  {
    SetMargin(0);
    SetBorder(0, 0);

    // Action name
    const Keyboard *kbd = Keyboard::GetConstInstance();
    label_action = new Label(kbd->GetHumanReadableActionName(action),
                             MIN_ACTION_WIDTH, (Font::font_size_t)14, Font::FONT_BOLD,
                             dark_gray_color, Text::ALIGN_RIGHT_CENTER);
    AddWidget(label_action);

    // First spacing
    AddWidget(new NullWidget(Point2i(SPACING_WIDTH, height)));

    int key_code = kbd->GetKeyAssociatedToAction(action);
    key_value = kbd->GetRawKeyCode(key_code);
    shift = kbd->HasShiftModifier(key_code);
    alt = kbd->HasAltModifier(key_code);
    ctrl = kbd->HasControlModifier(key_code);

    // Actual key
    label_key = new Label((key_value) ? kbd->GetKeyNameFromKey(key_value) : _("None"),
                          MIN_KEY_WIDTH, Font::FONT_SMALL, Font::FONT_NORMAL,
                          c_black, Text::ALIGN_LEFT_CENTER);
    AddWidget(label_key);

    // Second spacing
    AddWidget(new NullWidget(Point2i(SPACING_WIDTH, height)));

    // Modifiers
    shift_box = new CheckBox("", CHECKBOX_WIDTH, shift);
    AddWidget(shift_box);

    alt_box   = new CheckBox("", CHECKBOX_WIDTH, alt);
    AddWidget(alt_box);

    ctrl_box  = new CheckBox("", CHECKBOX_WIDTH, ctrl);
    AddWidget(ctrl_box);
  }

  virtual bool SendKey(const SDL_keysym & key)
  {
    //printf("Received key %i\n", key.sym);
    key_value = key.sym;
    label_key->SetText(Keyboard::GetConstInstance()->GetKeyNameFromKey(key_value));

    SDLMod sdl_modifier_bits = SDL_GetModState();
    shift_box->SetValue(sdl_modifier_bits & KMOD_SHIFT);
    alt_box->SetValue(sdl_modifier_bits & KMOD_ALT);
    ctrl_box->SetValue(sdl_modifier_bits & KMOD_CTRL);

    // A simple NeedRedraw would reset the packing
    Pack();
    return HBox::SendKey(key);
  }

  virtual void Pack()
  {
    // Call first HBox::Pack to set positions
    HBox::Pack();

    // Set proper height now
    int height = size.y-2*border.y;
    for (wit it = widget_list.begin(); it != widget_list.end(); ++it)
      (*it)->SetSizeY(height);
  }
};

class HeaderItem : public HBox
{
  Label *label_action, *label_key, *label_mods;

public:
  HeaderItem(uint height, bool force_widget_size = true)
    : HBox(height, false, force_widget_size)
  {
    SetMargin(0);
    SetBorder(0, 0);

    // Action name
    label_action = new Label(_("Action"),
                             MIN_ACTION_WIDTH, Font::FONT_MEDIUM, Font::FONT_BOLD,
                             light_gray_color, Text::ALIGN_RIGHT_CENTER);
    AddWidget(label_action);

    // Spacing
    AddWidget(new NullWidget(Point2i(SPACING_WIDTH, height)));

    // Actual key
    label_key = new Label(_("Key"),
                          MIN_KEY_WIDTH, Font::FONT_MEDIUM, Font::FONT_NORMAL,
                          c_black, Text::ALIGN_LEFT_CENTER);
    AddWidget(label_key);

    // Spacing
    AddWidget(new NullWidget(Point2i(SPACING_WIDTH, height)));

    // Modifiers
    label_mods = new Label(_("Modifiers"),
                           90, Font::FONT_MEDIUM, Font::FONT_NORMAL,
                           c_black, Text::ALIGN_LEFT_CENTER);
    AddWidget(label_mods);

    SetBackgroundColor(light_gray_color);
  }

  virtual void Pack()
  {
    // Call first HBox::Pack to set positions
    HBox::Pack();

    // Set proper height now
    int height = size.y-2*border.y;
    for (wit it = widget_list.begin(); it != widget_list.end(); ++it)
      (*it)->SetSizeY(height);
  }
};

ControlConfig::ControlConfig(const Point2i& size, bool readonly,
                             bool force_widget_size)
  : WidgetList(size)
  , read_only(readonly)
{
  header = new HeaderItem(32, force_widget_size);
  AddWidget(header);

  box = new SelectBox(size, false, force_widget_size, true);
  for (int i=0; i<ManMachineInterface::KEY_NONE; i++) {
    ControlItem *item = new ControlItem((ManMachineInterface::Key_t)i, 32);
    items.push_back(item);
    box->AddWidget(item);
  }
  AddWidget(box);
  //SetBackgroundColor(transparent_color);
}

bool ControlConfig::SendKey(const SDL_keysym & key)
{
  if (read_only)
    return WidgetList::SendKey(key);
  Widget *sel = box->GetSelectedWidget();
  if (sel)
    return sel->SendKey(key);
  return false;
}

void ControlConfig::SaveControlConfig() const
{
  if (read_only)
    return;

  Keyboard *kbd = Keyboard::GetInstance();
  kbd->ClearKeyAction();
  for (uint i=0; i<items.size(); i++) {
    const ControlItem* item = static_cast<ControlItem*>(items[i]);
    kbd->SaveKeyEvent(item->key_action, item->key_value,
                      item->shift, item->alt, item->ctrl);
  }
}

void ControlConfig::Pack()
{
  //printf("Set widget at (%i,%i) as %ix%i\n", position.x, position.y, size.x, size.y);
  header->SetPosition(position);
  header->SetSizeX(size.x);
  header->Pack();
  box->SetPosition(position.x, position.y + header->GetSizeY());
  box->SetSize(size.x, size.y - header->GetSizeY());
  box->Pack();
}
