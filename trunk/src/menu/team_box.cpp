/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 *  Teams selection box
 *****************************************************************************/

#include "ai/ai_stupid_player.h"
#include "gui/button.h"
#include "gui/label.h"
#include "gui/picture_widget.h"
#include "gui/spin_button.h"
#include "gui/text_box.h"
#include "menu/team_box.h"
#include "include/action_handler.h"
#include "network/network.h"
#include "team/team.h"
#include "team/custom_team.h"
#include "team/custom_teams_list.h"
#include "tool/resource_manager.h"

TeamBox::TeamBox(const std::string& _player_name, const Point2i& _size) :
  HBox(W_UNDEF, false, false)
{
  associated_team = NULL;
  is_local_ai = false;

  SetMargin(2);
  SetNoBorder();

  Profile *res = GetResourceManager().LoadXMLProfile( "graphism.xml", false);

  Box * tmp_logo_box = new VBox(W_UNDEF, false, false);
  tmp_logo_box->SetMargin(1);
  tmp_logo_box->SetNoBorder();

  team_logo = new PictureWidget(Point2i(38, 38));
  tmp_logo_box->AddWidget(team_logo);

  player_ai_surf = GetResourceManager().LoadImage(res, "menu/player_ai");
  player_local_surf = GetResourceManager().LoadImage(res, "menu/player_local");
  player_remote_surf = GetResourceManager().LoadImage(res, "menu/player_remote");

  player_type =  new PictureWidget(Point2i(38, 30));
  player_type->SetSurface(player_local_surf);
  tmp_logo_box->AddWidget(player_type);

  AddWidget(tmp_logo_box);

  Box * tmp_box = new VBox(W_UNDEF, false, false);
  tmp_box->SetMargin(2);
  tmp_box->SetNoBorder();
  previous_player_name = "team";
  team_name = new Label(previous_player_name, _size.x - 50,
                        Font::FONT_MEDIUM, Font::FONT_BOLD,
                        dark_gray_color, false, false);

  Box * tmp_player_box = new HBox(W_UNDEF, false, false);
  tmp_player_box->SetMargin(0);
  tmp_player_box->SetNoBorder();

  custom_team_list = GetCustomTeamsList().GetList();
  custom_team_current_id = 0;

  player_name = new TextBox(_player_name, 100,
                            Font::FONT_SMALL, Font::FONT_BOLD);

  if (custom_team_list.empty()) {
    tmp_player_box->AddWidget(new Label(_("Head commander"), _size.GetX()-50-100,
                                      Font::FONT_SMALL, Font::FONT_BOLD, dark_gray_color, false, false));

    tmp_player_box->AddWidget(player_name);

    next_custom_team = NULL;
    previous_custom_team = NULL;

  } else {
    tmp_player_box->AddWidget(new Label(_("Head commander"), _size.GetX()-60-100,
					Font::FONT_SMALL, Font::FONT_BOLD, dark_gray_color, false, false));

    next_custom_team = new Button(res, "menu/plus");

    previous_custom_team = new Button(res, "menu/minus");

    tmp_player_box->AddWidget(previous_custom_team);
    tmp_player_box->AddWidget(player_name);
    tmp_player_box->AddWidget(next_custom_team);
  }

  nb_characters = new SpinButton(_("Number of characters"), _size.GetX()-50,
                                 6,1,1,10,
                                 dark_gray_color, false);

  tmp_box->AddWidget(team_name);
  tmp_box->AddWidget(tmp_player_box);
  tmp_box->AddWidget(nb_characters);


  AddWidget(tmp_box);
}

void TeamBox::ClearTeam()
{
  associated_team = NULL;
  is_local_ai = false;

  NeedRedrawing();
}

Team* TeamBox::GetTeam() const
{
  return associated_team;
}

CustomTeam* TeamBox::GetCustomTeam()
{
  if (custom_team_list.empty()) {
    return NULL;
  }

  return GetCustomTeamsList().GetByName(player_name->GetText());
}

void TeamBox::Update(const Point2i &mousePosition,
                     const Point2i &lastMousePosition)
{
  Box::Update(mousePosition, lastMousePosition);
  if (need_redrawing) {
    Draw(mousePosition);
  }

  if (associated_team != NULL){
    WidgetList::Update(mousePosition);
  } else {
    RedrawBackground(*this);
  }

  if (associated_team != NULL && previous_player_name != player_name->GetText()) {
    previous_player_name = player_name->GetText();
    if (Network::GetInstance()->IsConnected()) {
      ValidOptions();
    }
  }

  need_redrawing = false;
}

Widget* TeamBox::ClickUp(const Point2i &mousePosition, uint button)
{
  if (associated_team != NULL) {

    Widget* w = WidgetList::ClickUp(mousePosition, button);

    if (!associated_team->IsLocal())
      return NULL; // it's not a local team, we can't configure it !!

    if (w == nb_characters) {
      if (Network::GetInstance()->IsConnected()) {
              ValidOptions();
      }
      return w;
    }
    if (w == player_name) {
      return w;
    }

    if (w == NULL) {
      return w;
    }

    if (!custom_team_list.empty()) {

      if (w == next_custom_team) {
        player_name->SetText(custom_team_list[custom_team_current_id]->GetName());

        if(custom_team_current_id == custom_team_list.size()-1) {
          custom_team_current_id = 0;
        } else {
          custom_team_current_id++;
        }
      }

      if (w == previous_custom_team) {

        player_name->SetText(custom_team_list[custom_team_current_id]->GetName());

        if(custom_team_current_id == 0) {
          custom_team_current_id = custom_team_list.size()-1;
        } else {
          custom_team_current_id--;
        }
      }
    }
  }
  return NULL;
}

Widget* TeamBox::Click(const Point2i &/*mousePosition*/, uint /*button*/)
{
  return NULL;
}

void TeamBox::SetTeam(Team& _team, bool read_team_values)
{
  Team* old_team = associated_team;

  associated_team = &_team;

  if (_team.IsRemote()) {
    team_name->SetFont(dark_gray_color, Font::FONT_MEDIUM, Font::FONT_BOLD, false, false);

    // translators: this is the team listing and will expand in a context like "OOo team - Remote"
    team_name->SetText(Format(_("%s Team - Remote"), _team.GetName().c_str()));

    if (previous_custom_team) {
      previous_custom_team->SetVisible(false);
      next_custom_team->SetVisible(false);
    }
    player_type->SetSurface(player_remote_surf);
  } else {
    team_name->SetFont(primary_red_color, Font::FONT_MEDIUM, Font::FONT_BOLD, true, false);
    team_name->SetText(Format(_("%s Team"), _team.GetName().c_str()));

    if (previous_custom_team) {
      previous_custom_team->SetVisible(true);
      next_custom_team->SetVisible(true);
    }

    if (is_local_ai) {
      player_type->SetSurface(player_ai_surf);
    } else {
      player_type->SetSurface(player_local_surf);
    }
  }
  team_logo->SetSurface(_team.GetFlag());

  if (read_team_values) {
    player_name->SetText(_team.GetPlayerName());
    nb_characters->SetValue(_team.GetNbCharacters());
  } else if (old_team) {
    UpdateTeam(old_team->GetId());
  }
  previous_player_name = player_name->GetText();

  NeedRedrawing();
}

void TeamBox::UpdateTeam(const std::string& old_team_id) const
{
  // set the number of characters
  associated_team->SetNbCharacters(uint(nb_characters->GetValue()));

  // set the player name
  associated_team->SetPlayerName(player_name->GetText());

  // change only for local teams...
  if (associated_team->IsLocal()) {

    if (is_local_ai) {
      associated_team->SetAI(new AIStupidPlayer(associated_team));
    } else {
      associated_team->SetHuman();
    }

    // send team configuration to the remote clients
    if (Network::GetInstance()->IsConnected()) {
      Action* a = new Action(Action::ACTION_GAME_UPDATE_TEAM);
      a->Push(int(Network::GetInstance()->GetPlayer().GetId()));
      a->Push(old_team_id);
      a->Push(associated_team->GetId());
      a->Push(associated_team->GetPlayerName());
      a->Push(int(associated_team->GetNbCharacters()));
      ActionHandler::GetInstance()->NewAction (a);
    }
  }
}

void TeamBox::ValidOptions() const
{
  UpdateTeam(associated_team->GetId());
}

bool TeamBox::IsLocal() const
{
  if (associated_team != NULL && associated_team->IsLocal()) {
    return true;
  }

  return false;
}

void TeamBox::SwitchPlayerType()
{
  if (!associated_team)
    return;

  if (associated_team->IsLocal()) {
    is_local_ai = !is_local_ai;

    if (is_local_ai) {
      player_type->SetSurface(player_ai_surf);
    } else {
      player_type->SetSurface(player_local_surf);
    }
  }
}
