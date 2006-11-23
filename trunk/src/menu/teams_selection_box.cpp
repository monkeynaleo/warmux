/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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

#include "teams_selection_box.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"

TeamBox::TeamBox(std::string _player_name, const Rectanglei& rect) : 
  HBox(rect, false)
{
  associated_team=NULL;

  SetMargin(2);

  team_logo = new PictureWidget( Rectanglei(0,0,48,48) );
  AddWidget(team_logo);

  Box * tmp_box = new VBox(Rectanglei(0, 0, rect.GetSizeX()-80, 80), false);
  tmp_box->SetMargin(2);
  tmp_box->SetBorder(Point2i(0,0));
  team_name = new Label(" ", Rectanglei(0,0,rect.GetSizeX()-80,0),
			*Font::GetInstance(Font::FONT_NORMAL, Font::BOLD), dark_gray_color, false, false);

  Box * tmp_player_box = new HBox(Rectanglei(0,0,0,Font::GetInstance(Font::FONT_SMALL)->GetHeight()), false);
  tmp_player_box->SetMargin(0);
  tmp_player_box->SetBorder(Point2i(0,0));
  tmp_player_box->AddWidget(new Label(_("Head commander"), Rectanglei(0,0,(rect.GetSizeX()-80)-100,0),
				      *Font::GetInstance(Font::FONT_SMALL), dark_gray_color, false, false));
  player_name = new TextBox(_player_name, Rectanglei(0,0,100,0),
			    *Font::GetInstance(Font::FONT_SMALL));
  tmp_player_box->AddWidget(player_name);

  nb_characters = new SpinButton(_("Number of characters"), Rectanglei(0,0,0,0),
				 6,1,2,10,
				 dark_gray_color, false);

  tmp_box->AddWidget(team_name);
  tmp_box->AddWidget(tmp_player_box);
  tmp_box->AddWidget(nb_characters);

  AddWidget(tmp_box);
}

void TeamBox::SetTeam(Team& _team, bool read_team_values)
{
  associated_team=&_team;

  team_logo->SetSurface(_team.flag);
  team_name->SetText(_team.GetName());
  team_logo->SetSurface(_team.flag);

  if (read_team_values) {
    player_name->SetText(_team.GetPlayerName());
    nb_characters->SetValue(_team.GetNbCharacters());
  }

  ForceRedraw();
}

void TeamBox::ClearTeam()
{
  associated_team=NULL;

  ForceRedraw();
}

Team* TeamBox::GetTeam() const
{
  return associated_team;
}

void TeamBox::Update(const Point2i &mousePosition,
		     const Point2i &lastMousePosition,
		     Surface& surf)
{
  Box::Update(mousePosition, lastMousePosition, surf);
  if (need_redrawing) {
    Draw(mousePosition, surf);
  }

  if (associated_team != NULL){
    WidgetList::Draw(mousePosition, surf);
  } else {
    Redraw(*this, surf);
  }

  need_redrawing = false;
}

Widget* TeamBox::Clic (const Point2i &mousePosition, uint button)
{
  if (associated_team != NULL) {

    Widget* w = WidgetList::Clic(mousePosition, button);

    if ( w == nb_characters ||  w == player_name ) {
      return w;
    }
  }
  return NULL;
}

void TeamBox::ValidOptions() const
{
  // set the number of characters
  associated_team->SetNbCharacters(uint(nb_characters->GetValue()));

  // set the player name
  associated_team->SetPlayerName(player_name->GetText());

  if (player_name->GetText() == "AI-stupid")
    associated_team->SetLocalAI();
  else 
    associated_team->SetLocal();
}

// -----------------------------------------------
// -----------------------------------------------

TeamsSelectionBox::TeamsSelectionBox(const Rectanglei &rect) : HBox(rect, true)
{
  AddWidget(new PictureWidget(Rectanglei(0,0,38,150), "menu/teams_label"));

  // How many teams ?
  teams_nb = new SpinButtonBig(_("Number of teams:"), Rectanglei(0, 0, 130, 30),
			       2, 1,
			       2, MAX_NB_TEAMS);
  AddWidget(teams_nb);

  Box * top_n_bottom_team_options = new VBox( Rectanglei(0, 0,
							 rect.GetSizeX() - teams_nb->GetSizeX() - 60, 0)
					      ,false);
  top_n_bottom_team_options->SetBorder(Point2i(5,0));
  top_n_bottom_team_options->SetMargin(10);
  Box * top_team_options = new HBox ( Rectanglei(0, 0, 0, rect.GetSizeY()/2 - 20), false);
  Box * bottom_team_options = new HBox ( Rectanglei(0, 0, 0, rect.GetSizeY()/2 - 20), false);
  top_team_options->SetBorder(Point2i(0,0));
  bottom_team_options->SetBorder(Point2i(0,0));

  // Initialize teams
  uint team_w_size= top_n_bottom_team_options->GetSizeX() * 2 / MAX_NB_TEAMS;

  for (uint i=0; i < MAX_NB_TEAMS; i++) {
    std::string player_name = _("Player") ;
    char num_player[4];
    sprintf(num_player, " %d", i+1);
    player_name += num_player;
    teams_selections[i] = new TeamBox(player_name, Rectanglei(0,0,team_w_size,rect.GetSizeY()/2));
    if ( i%2 == 0)
      top_team_options->AddWidget(teams_selections[i]);
    else
      bottom_team_options->AddWidget(teams_selections[i]);
  }

  top_n_bottom_team_options->AddWidget(top_team_options);
  top_n_bottom_team_options->AddWidget(bottom_team_options);

  AddWidget(top_n_bottom_team_options); 

  // Load Teams' list
  teams_list.full_list.sort(compareTeams);

  TeamsList::iterator
    it=teams_list.playing_list.begin(),
    end=teams_list.playing_list.end();

  uint j=0;
  for (; it != end; ++it, j++)
    {
      teams_selections[j]->SetTeam((**it), true);
    }
  
  if (j < 2) {
    SetNbTeams(2);
    teams_nb->SetValue(2);
  } else {
    teams_nb->SetValue(j);
  }
}

Widget* TeamsSelectionBox::Clic (const Point2i &mousePosition, uint button)
{
  if (!Contains(mousePosition)) return NULL;

  if (teams_nb->Clic(mousePosition, button)){
    SetNbTeams(teams_nb->GetValue());

  } else {
    for (uint i=0; i<MAX_NB_TEAMS ; i++) {

      if ( teams_selections[i]->Contains(mousePosition) ) {
	
	Widget * w = teams_selections[i]->Clic(mousePosition, button);

	if ( w == NULL ) {
	  if ( button == SDL_BUTTON_LEFT || button == SDL_BUTTON_WHEELDOWN ) {
	    NextTeam(i);
	  } else if ( button == SDL_BUTTON_RIGHT || button == SDL_BUTTON_WHEELUP ) {
	    PrevTeam(i);
	  }
	} else {
	  return w;
	}
	break;
      }
    }
  }

  return NULL;
}

void TeamsSelectionBox::PrevTeam(int i)
{
  if (teams_selections[i]->GetTeam() == NULL) return;

  bool to_continue;
  Team* tmp;
  int previous_index = -1, index;  

  teams_list.FindById(teams_selections[i]->GetTeam()->GetId(), previous_index);

  index = previous_index-1;

  do 
    {
      to_continue = false;

      // select the last team if we are outside list
      if ( index < 0 )
	index = int(teams_list.full_list.size())-1;

      // Get the team at current index
      tmp = teams_list.FindByIndex(index);
      
      // Check if that team is already selected
      for (int j = 0; j < teams_nb->GetValue(); j++) {
	if (j!= i && tmp == teams_selections[j]->GetTeam()) {
	  index--;
	  to_continue = true;
	  break;
	}
      }
      
      // We have found a team which is not selected
      if (tmp != NULL && !to_continue)
	teams_selections[i]->SetTeam(*tmp);
    } while ( index != previous_index && to_continue);
}

void TeamsSelectionBox::NextTeam(int i)
{
  if (teams_selections[i]->GetTeam() == NULL) return;

  bool to_continue;
  Team* tmp;
  int previous_index = -1, index;

  teams_list.FindById(teams_selections[i]->GetTeam()->GetId(), previous_index);

  index = previous_index+1;

  do 
    {
      to_continue = false;

      // select the first team if we are outside list
      if ( index >= int(teams_list.full_list.size()) )
	index = 0;

      // Get the team at current index
      tmp = teams_list.FindByIndex(index);
      
      // Check if that team is already selected
      for (int j = 0; j < teams_nb->GetValue(); j++) {
	if (j!= i && tmp == teams_selections[j]->GetTeam()) {
	  index++;
	  to_continue = true;
	  break;
	}
      }
      
      // We have found a team which is not selected
      if (tmp != NULL && !to_continue)
	teams_selections[i]->SetTeam(*tmp);
    } while ( index != previous_index && to_continue);
}

void TeamsSelectionBox::SetNbTeams(uint nb_teams)
{
  // we hide the useless teams selector
  for (uint i=nb_teams; i<MAX_NB_TEAMS; i++) {
    teams_selections[i]->ClearTeam();
  }

  for (uint i=0; i<nb_teams;i++) {
    if (teams_selections[i]->GetTeam() == NULL) {
      // we should find an available team
      teams_selections[i]->SetTeam(*(teams_list.FindByIndex(i)));
      NextTeam(i);
    }
  }
}

void TeamsSelectionBox::ValidTeamsSelection()
{
  std::list<uint> selection;

  uint nb_teams=0;
  for (uint i=0; i < MAX_NB_TEAMS; i++) {
    if (teams_selections[i]->GetTeam() != NULL)
      nb_teams++;
  }

  if (nb_teams >= 2) {
    std::list<uint> selection;

    for (uint i=0; i < MAX_NB_TEAMS; i++) {
      if (teams_selections[i]->GetTeam() != NULL) {
	int index = -1;
	teams_selections[i]->ValidOptions();
	teams_list.FindById(teams_selections[i]->GetTeam()->GetId(), index);
	if (index > -1)
	  selection.push_back(uint(index));
      }
    }
    teams_list.ChangeSelection (selection);
  }


}
