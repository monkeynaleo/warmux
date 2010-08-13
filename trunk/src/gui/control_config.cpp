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
#include "gui/question.h"
#include "gui/select_box.h"
#include "interface/keyboard.h"

#include "control_config.h"

#define MIN_ACTION_WIDTH  276
#define MIN_KEY_WIDTH      70
#define SPACING_WIDTH       8
#define MODIFIERS_WIDTH    86
#define CHECKBOX_WIDTH     18

class ControlItem : public HBox
{
  ManMachineInterface::Key_t key_action;
  int   key_value;
  bool  read_only;

  Label *label_action, *label_key;
  CheckBox *shift_box, *alt_box, *ctrl_box;

public:
  ControlItem(int action, bool ro, uint height)
    : HBox(height, false, true /* use full height */)
    , key_action((ManMachineInterface::Key_t)action)
    , read_only(ro)
  {
    SetMargin(0);
    SetBorder(0, 0);

    // Action name
    const Keyboard *kbd = Keyboard::GetConstInstance();
    label_action = new Label(kbd->GetHumanReadableActionName(key_action),
                             MIN_ACTION_WIDTH, (Font::font_size_t)14, Font::FONT_BOLD,
                             dark_gray_color, Text::ALIGN_RIGHT_CENTER);
    AddWidget(label_action);

    // First spacing
    AddWidget(new NullWidget(Point2i(SPACING_WIDTH, height)));

    // Modifiers
    int key_code = kbd->GetKeyAssociatedToAction(key_action);
    ctrl_box  = new CheckBox("", CHECKBOX_WIDTH,
                             kbd->HasControlModifier(key_code));
    AddWidget(ctrl_box);

    alt_box   = new CheckBox("", CHECKBOX_WIDTH,
                             kbd->HasAltModifier(key_code));
    AddWidget(alt_box);

    shift_box = new CheckBox("", CHECKBOX_WIDTH,
                             kbd->HasShiftModifier(key_code));
    AddWidget(shift_box);

    // Second spacing
    AddWidget(new NullWidget(Point2i(SPACING_WIDTH, height)));

    // Actual key
    key_value = kbd->GetRawKeyCode(key_code);
    label_key = new Label((key_value) ? kbd->GetKeyNameFromKey(key_value) : _("None"),
                          MIN_KEY_WIDTH, Font::FONT_SMALL, Font::FONT_NORMAL,
                          c_black, Text::ALIGN_LEFT_CENTER);
    AddWidget(label_key);
  }

  virtual Widget* ClickUp(const Point2i &mousePosition, uint button)
  {
    WidgetList::ClickUp(mousePosition, button);
    return NULL;
  }

  virtual bool SendKey(const SDL_keysym & key)
  {
    if (read_only)
      return false;

    // Ignore modifiers-only key presses
    if (key.sym >= SDLK_NUMLOCK && key.sym <= SDLK_COMPOSE)
      return true;

    SDLMod mod_bits = SDL_GetModState();
    const Keyboard *kbd = Keyboard::GetConstInstance();
    ManMachineInterface::Key_t tmp =
      kbd->GetRegisteredAction(key.sym, mod_bits & KMOD_CTRL,
                               mod_bits & KMOD_ALT, mod_bits & KMOD_SHIFT);

    // Check and warn if key already attributed
    if (tmp != ManMachineInterface::KEY_NONE) {
      Question question(Question::WARNING);
      question.Set(Format(_("This key has already been attributed to '%s'"),
                          kbd->GetHumanReadableActionName(tmp).c_str()),
                   true, 0);

      // React only to key press, not releases, as one key is being pressed now
      question.Ask(false);
      return true;
    }

    key_value = key.sym;
    label_key->SetText(Keyboard::GetConstInstance()->GetKeyNameFromKey(key_value));

    shift_box->SetValue(mod_bits & KMOD_SHIFT);
    alt_box->SetValue(mod_bits & KMOD_ALT);
    ctrl_box->SetValue(mod_bits & KMOD_CTRL);

    // A simple NeedRedraw would reset the packing
    Pack();
    return true;
  }

  virtual void Pack()
  {
    // First this so that HBox::Pack does not reset label_key width back
    // to its minimal value
    label_key->SetSizeX(size.x - MIN_ACTION_WIDTH -
                        2*SPACING_WIDTH - 3*CHECKBOX_WIDTH);

    // Call first HBox::Pack to set positions
    HBox::Pack();

    // Set proper height now
    int height = size.y-2*border.y;
    for (wit it = widget_list.begin(); it != widget_list.end(); ++it)
      (*it)->SetSizeY(height);
  }

  void SaveAction(Keyboard *kbd)
  {
    kbd->SaveKeyEvent(key_action, key_value,
                      ctrl_box->GetValue(),
                      alt_box->GetValue(),
                      shift_box->GetValue());
  }
};

class HeaderItem : public HBox
{
  Label *label_action, *label_key, *label_mods;

public:
  HeaderItem(uint height)
    : HBox(height, false, true)
  {
    SetMargin(0);
    SetBorder(0, 0);

    // Action name
    label_action = new Label(_("Action"), MIN_ACTION_WIDTH,
                             Font::FONT_MEDIUM, Font::FONT_NORMAL,
                             c_black, Text::ALIGN_RIGHT_CENTER);
    AddWidget(label_action);

    // Modifiers
    label_mods = new Label(_("Modifiers"), MODIFIERS_WIDTH,
                           Font::FONT_MEDIUM, Font::FONT_NORMAL,
                           c_black, Text::ALIGN_CENTER);
    AddWidget(label_mods);

    // Actual key
    label_key = new Label(_("Key"), MIN_KEY_WIDTH,
                          Font::FONT_MEDIUM, Font::FONT_NORMAL,
                          c_black, Text::ALIGN_LEFT_CENTER);
    AddWidget(label_key);

    SetBackgroundColor(light_gray_color);
  }

  virtual void Pack()
  {
    // First this so that HBox::Pack does not reset label_key width back
    // to its minimal value
    label_key->SetSizeX(size.x - MIN_ACTION_WIDTH - MODIFIERS_WIDTH);

    // Call first HBox::Pack to set positions
    HBox::Pack();

    // Set proper height now
    int height = size.y-2*border.y;
    for (wit it = widget_list.begin(); it != widget_list.end(); ++it)
      (*it)->SetSizeY(height);
  }
};

ControlConfig::ControlConfig(const Point2i& size, bool readonly)
  : WidgetList(size)
  , read_only(readonly)
{
  header = new HeaderItem(32);
  AddWidget(header);

  box = new SelectBox(size, false, true /* full box width used*/);
  box->SetBorder(c_black, 0);
  for (int i=0; i<ManMachineInterface::KEY_NONE; i++) {
    ControlItem *item = new ControlItem(i, readonly, 32);
    items.push_back(item);
    box->AddWidget(item);
  }
  AddWidget(box);
}

void ControlConfig::SaveControlConfig() const
{
  if (read_only)
    return;

  Keyboard *kbd = Keyboard::GetInstance();
  kbd->ClearKeyAction();
  for (uint i=0; i<items.size(); i++) {
    items[i]->SaveAction(kbd);
  }
}

void ControlConfig::Pack()
{
  //printf("Set widget at (%i,%i) as %ix%i\n", position.x, position.y, size.x, size.y);
  header->SetPosition(position);
  header->SetSizeX(size.x - 2*border_size);
  header->Pack();
  box->SetPosition(position.x + border_size,
                   position.y + header->GetSizeY() + border_size);
  box->SetSize(size.x - 2*border_size,
               size.y - header->GetSizeY() - 2*border_size);
  box->Pack();
}
