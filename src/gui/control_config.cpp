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
#include "interface/keyboard.h"

#include "control_config.h"

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
                             280, (Font::font_size_t)14, Font::FONT_BOLD,
                             dark_gray_color, Text::ALIGN_RIGHT_CENTER);
    AddWidget(label_action);

    // Spacing
    AddWidget(new NullWidget(Point2i(12, height)));

    int key_code = kbd->GetKeyAssociatedToAction(action);
    key_value = kbd->GetRawKeyCode(key_code);
    shift = kbd->HasShiftModifier(key_code);
    alt = kbd->HasAltModifier(key_code);
    ctrl = kbd->HasControlModifier(key_code);

    // Actual key
    label_key = new Label((key_value) ? kbd->GetKeyNameFromKey(key_value) : _("None"),
                          64, Font::FONT_SMALL, Font::FONT_NORMAL,
                          c_black, Text::ALIGN_LEFT_CENTER);
    AddWidget(label_key);

    // Modifiers
    shift_box = new CheckBox("", 18, shift); AddWidget(shift_box);
    alt_box   = new CheckBox("", 18, alt); AddWidget(alt_box);
    ctrl_box  = new CheckBox("", 18, ctrl); AddWidget(ctrl_box);
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
    HBox::Pack();
    int height = size.y-2*border.y;
    label_action->SetSizeY(height);
    label_key->SetSizeY(height);
    shift_box->SetSizeY(height);
    alt_box->SetSizeY(height);
    ctrl_box->SetSizeY(height);
  }
};

ControlConfig::ControlConfig(const Point2i& size, bool readonly,
                             bool force_widget_size)
  : SelectBox(size, false, force_widget_size, true)
  , read_only(readonly)
{
  SetBackgroundColor(transparent_color);
  for (int i=0; i<ManMachineInterface::KEY_NONE; i++) {
    AddWidget(new ControlItem((ManMachineInterface::Key_t)i, 32));
  }
}

bool ControlConfig::SendKey(const SDL_keysym & key)
{
  if (read_only)
    return SelectBox::SendKey(key);
  if (selected_item == -1)
    return false;
  return m_items[selected_item]->SendKey(key);
}

void ControlConfig::SaveControlConfig() const
{
  if (read_only)
    return;

  Keyboard *kbd = Keyboard::GetInstance();
  kbd->ClearKeyAction();
  for (uint i=0; i<m_items.size(); i++) {
    const ControlItem* item = static_cast<ControlItem*>(m_items[i]);
    kbd->SaveKeyEvent(item->key_action, item->key_value,
                      item->shift, item->alt, item->ctrl);
  }
}
