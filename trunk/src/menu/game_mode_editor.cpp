/******************************************************************************
 *  Warmux is a convivial mass murder game.
 *  Copyright (C) 2001-2011 Warmux Team.
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
 * Game mode editor
 *****************************************************************************/

#include <vector>

#include "menu/game_mode_editor.h"

#include "game/game.h"
#include "game/config.h"
#include "game/game_mode.h"
#include "gui/button.h"
#include "gui/combo_box.h"
#include "gui/null_widget.h"
#include "gui/picture_widget.h"
#include "gui/question.h"
#include "gui/select_box.h"
#include "gui/spin_button_picture.h"
#include "gui/text_box.h"
#include "gui/horizontal_box.h"
#include "gui/vertical_box.h"
#include "graphic/sprite.h"
#include "weapon/weapons_list.h"

const uint TPS_TOUR_MIN = 10;
const uint TPS_TOUR_MAX = 240;
static bool warned = false;

GameModeEditor::GameModeEditor(const Point2i& size, float zoom, bool _draw_border)
  : HBox(size.y, _draw_border)
{
  Profile *res  = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  static const Point2i option_size(110, 120);
  Font::font_size_t fmedium = Font::GetFixedSize(Font::FONT_MEDIUM*zoom+0.5f);
  Font::font_size_t fsmall  = Font::GetFixedSize(Font::FONT_SMALL*zoom+0.5f);
  // ################################################
  // ##  GAME OPTIONS
  // ################################################
  std::string selected_gamemode = Config::GetInstance()->GetGameMode();

  VBox *vbox = new VBox(fsmall*10+2*4, false);
  vbox->SetBorder(transparent_color, 4);
  vbox->SetSelfBackgroundColor(transparent_color);
  AddWidget(vbox);

  vbox->AddWidget(new Label(_("Game mode"), fsmall*10, fmedium, Font::FONT_BOLD, c_red));
  opt_game_mode = new ItemBox(Point2i(fsmall*10, size.y - 4*4 - 2*border_size - 2*fmedium - fsmall - 86));
  game_modes = GameMode::ListGameModes();
  for (uint i=0; i<game_modes.size(); i++) {
    opt_game_mode->AddLabelItem(game_modes[i].first==selected_gamemode,
                                game_modes[i].second, &game_modes[i].first, fsmall);
  }
  vbox->AddWidget(opt_game_mode);

  vbox->AddWidget(new Label(_("Filename"), fsmall*10, fmedium, Font::FONT_BOLD, c_red));

  filename = new TextBox("", fsmall*10, fsmall);
  vbox->AddWidget(filename);

  save = new Button(res, "menu/save", false); save->SetBorder(defaultOptionColorRect, 2);
  vbox->AddWidget(save);

  uint width = size.x - (fsmall*10 + 4*4 + 2*border_size);
  vbox = new VBox(width, false);
  vbox->SetBorder(transparent_color, 4);
  vbox->SetSelfBackgroundColor(transparent_color);
  vbox->SetMargin(4);
  AddWidget(vbox);

  vbox->AddWidget(new Label(_("Settings"), fsmall*10, fmedium, Font::FONT_BOLD, c_red));

  ScrollBox *sbox = new ScrollBox(Point2i(width - 2*4, option_size.y + 2*4 + 24), true, false, false);
  vbox->AddWidget(sbox);

  opt_duration_turn = new SpinButtonWithPicture(_("Duration of a turn"), "menu/timing_turn",
                                                option_size,
                                                TPS_TOUR_MIN, 10,
                                                TPS_TOUR_MIN, TPS_TOUR_MAX,
                                                fmedium, fmedium);
  sbox->AddWidget(opt_duration_turn);

  std::vector<std::pair<std::string, std::string> > character_selections;
  character_selections.push_back(std::pair<std::string, std::string>("always", _("Always")));
  character_selections.push_back(std::pair<std::string, std::string>("before_action", _("Before action")));
  character_selections.push_back(std::pair<std::string, std::string>("never", _("Never")));

  opt_allow_character_selection = new ComboBox(_("Character switching"), "menu/character_selection",
                                               option_size, character_selections, "always",
                                               fmedium, fmedium);
  sbox->AddWidget(opt_allow_character_selection);

  /* Characters energy */
  opt_energy_ini = new SpinButtonWithPicture(_("Initial energy"), "menu/init_energy",
                                             option_size, 10, 10, 10, 500,
                                             fmedium, fmedium);
  sbox->AddWidget(opt_energy_ini);

  opt_energy_max = new SpinButtonWithPicture(_("Max energy"), "menu/max_energy",
                                             option_size, 10, 10, 10, 500,
                                             fmedium, fmedium);
  sbox->AddWidget(opt_energy_max);

  /* some death mode options */

  opt_time_before_death_mode = new SpinButtonWithPicture(_("Duration before death mode"), "menu/timing_death",
                                                         option_size, 200, 50, 200, 3000,
                                                         fmedium, fmedium);
  sbox->AddWidget(opt_time_before_death_mode);

  opt_damage_during_death_mode = new SpinButtonWithPicture(_("Damage per turn during death mode"), "menu/death_energy",
                                                           option_size, 1, 1, 1, 20, fmedium, fmedium);
  sbox->AddWidget(opt_damage_during_death_mode);

  opt_gravity = new SpinButtonWithPicture(_("Gravity"), "menu/gravity",
                                          option_size, 10, 5, 10, 60, fmedium, fmedium);
  sbox->AddWidget(opt_gravity);

  vbox->AddWidget(new Label(_("Weapons"), width - 4*4, fmedium, Font::FONT_BOLD, c_red));

  opt_weapons_cfg = new ScrollBox(Point2i(width - 4*4, size.y - 9*4 - sbox->GetSizeY() - 2*fmedium));
  vbox->AddWidget(opt_weapons_cfg);

  LoadGameMode(true);
}

Widget *GameModeEditor::ClickUp(const Point2i & mousePosition, uint button)
{
  Widget *w = HBox::ClickUp(mousePosition, button);
  if (opt_game_mode->Contains(mousePosition))
    LoadGameMode();
  return w;
}

class WeaponCfgBox : public HBox
{
  Weapon        *weapon;
  PictureWidget *pw;
  SpinButtonWithPicture *ammo;

public:
  WeaponCfgBox(Weapon *w, uint height)
    : HBox(height), weapon(w)
  {
    Point2i s(height+10, height);
    SetNoBorder();
    SetMargin(0);
 
    VBox *vbox = new VBox(80);
    vbox->SetNoBorder();
    AddWidget(vbox);
    vbox->AddWidget(new NullWidget(Point2i(80, 14)));

    Sprite& spr = w->GetIcon();
    spr.RefreshSurface();
    pw = new PictureWidget(spr.GetSurface().GetSize());
    pw->SetSurface(spr.GetSurface(), PictureWidget::NO_SCALING);
    vbox->AddWidget(pw);

    vbox->AddWidget(new Label(w->GetName(), 80, Font::FONT_SMALL, Font::FONT_BOLD,
                              dark_gray_color, Text::ALIGN_CENTER));

    int count = weapon->ReadInitialNbAmmo();
    ammo = new SpinButtonWithPicture(_("Ammos"), "menu/music_enable", s,
                                     count, 1, -1, (count>10) ? count : 10);
    AddWidget(ammo);

    SetBackgroundColor(transparent_color);
    Pack();
  }
};

void GameModeEditor::LoadGameMode(bool force)
{
  std::string* mode = (std::string*)opt_game_mode->GetSelectedValue();
  if (Config::GetInstance()->GetGameMode() == *mode && !force)
    return;
  Config::GetInstance()->SetGameMode(*mode);
  filename->SetText(*mode);
  GameMode * game_mode = GameMode::GetInstance();
  game_mode->Load();

  if (game_mode->allow_character_selection == GameMode::ALWAYS) {
    opt_allow_character_selection->SetChoice(0); // "always"
  } else if (game_mode->allow_character_selection == GameMode::BEFORE_FIRST_ACTION) {
    opt_allow_character_selection->SetChoice(1); // "before_action"
  } else if (game_mode->allow_character_selection == GameMode::NEVER) {
    opt_allow_character_selection->SetChoice(2); // "never"
  } else {
    ASSERT(false);
  }

  opt_duration_turn->SetValue(game_mode->duration_turn, true);
  opt_energy_ini->SetValue(game_mode->character.init_energy, true);
  opt_energy_max->SetValue(game_mode->character.max_energy, true);
  opt_time_before_death_mode->SetValue(game_mode->duration_before_death_mode, true);
  opt_damage_during_death_mode->SetValue(game_mode->damage_per_turn_during_death_mode, true);
  opt_gravity->SetValue((int)(game_mode->gravity), true);

  // Refill weapon list
  opt_weapons_cfg->Clear();
  WeaponsList tmp(game_mode->GetWeaponsXml());
  const WeaponsList::weapons_list_type& wlist = tmp.GetList();
  for (WeaponsList::iterator it = wlist.begin(); it != wlist.end(); ++it) {
    opt_weapons_cfg->AddWidget(new WeaponCfgBox(*it, 100));
  }
  opt_weapons_cfg->Pack();

  NeedRedrawing();

  if (!warned && *mode == "blitz") {
    Question puppy_attention_span(Question::WARNING);
    puppy_attention_span.Set(_("The blitz mode does not reset the time between each team turn. "
                               "A team looses when it has no players left or its time has ran out."),
                             true, 0);
    puppy_attention_span.Ask();
    warned = true;
  }
}

void GameModeEditor::ValidGameMode() const
{
  GameMode * game_mode = GameMode::GetInstance();
  game_mode->Load();

  if (opt_allow_character_selection->GetValue() == "always") {
    game_mode->allow_character_selection = GameMode::ALWAYS;
  } else if (opt_allow_character_selection->GetValue() == "before_action") {
    game_mode->allow_character_selection = GameMode::BEFORE_FIRST_ACTION;
  } else if (opt_allow_character_selection->GetValue() == "never") {
    game_mode->allow_character_selection = GameMode::NEVER;
  } else {
    ASSERT(false);
  }

  game_mode->duration_turn = opt_duration_turn->GetValue();
  game_mode->character.init_energy = opt_energy_ini->GetValue();
  game_mode->character.max_energy = opt_energy_max->GetValue();
  game_mode->duration_before_death_mode = opt_time_before_death_mode->GetValue();
  game_mode->damage_per_turn_during_death_mode = opt_damage_during_death_mode->GetValue();
  game_mode->gravity = opt_gravity->GetValue();
}
