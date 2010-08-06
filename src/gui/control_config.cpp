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
#include "gui/horizontal_box.h"
#include "gui/label.h"
#include "interface/keyboard.h"

#include "control_config.h"

class ControlItem : public HBox
{
  Label *label_action, *label_key;

public:
  ManMachineInterface::Key_t key_action;
  int  key_value;

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
                             320, Font::FONT_MEDIUM, Font::FONT_BOLD);
    //label_action->SetBorder(black_color, 1);
    AddWidget(label_action);

    // Actual key
    key_value = kbd->GetKeyAssociatedToAction(action);
    label_key = new Label(kbd->GetKeyNameFromKey(key_value),
                          70, Font::FONT_SMALL, Font::FONT_NORMAL);
    //label_key->SetBorder(black_color, 1);
    AddWidget(label_key);
  }

  virtual bool SendKey(const SDL_keysym & key)
  {
    //printf("Received key %i\n", key.sym);
    key_value = key.sym;
    label_key->SetText(Keyboard::GetConstInstance()->GetKeyNameFromKey(key_value));
    NeedRedrawing();
    return HBox::SendKey(key);
  }

  virtual void Pack()
  {
    HBox::Pack();
    label_action->SetSizeY(size.y-2*border.y);
    label_key->SetSizeY(size.y-2*border.y);
  }
};

ControlConfig::ControlConfig(const Point2i& size, bool readonly,
                             bool force_widget_size)
  : SelectBox(size, !readonly, force_widget_size)
  , read_only(readonly)
{
  for (int i=0; i<ManMachineInterface::KEY_NONE; i++) {
    AddWidget(new ControlItem((ManMachineInterface::Key_t)i, 24, false));
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
    kbd->SetKeyAction(item->key_value, item->key_action);
  }
}
