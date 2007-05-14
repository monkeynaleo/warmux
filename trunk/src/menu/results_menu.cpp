/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Results menu
 *****************************************************************************/

#include "results_menu.h"

#include "../character/character.h"
#include "../include/app.h"
#include "../team/results.h"
#include "../team/macro.h"
#include "../tool/i18n.h"
#include "../tool/string_tools.h"

#define DEF_MARGIN    16
#define DEF_BORDER    8
#define DEF_SIZE      32
#define USE_MOST      1

const Point2i BorderSize(DEF_BORDER, DEF_BORDER);
const Vector2<double> Zoom(1.7321, 1.7321);
const Point2i DefSize(DEF_SIZE, DEF_SIZE);

class ResultBox : public HBox
{
private:
  Label *name_lbl;
  Label *score_lbl;
  PictureWidget *team_picture;
public:
  ResultBox(const Rectanglei &rect, bool _visible,
            const char* type_name, 
	    Font::font_size_t font_size,
	    Font::font_style_t font_style,
            const Point2i& type_size,
            const Point2i& name_size,
            const Point2i& score_size);
  // Hopefully no need for ~ResultBox() as it automatically
  // destroy child widgets by inheriting from HBox
  void SetResult(const std::string& name, int score, const Surface& team_logo);
  void SetNoResult();
};

ResultBox::ResultBox(const Rectanglei &rect, bool _visible,
                     const char *type_name, 
		     Font::font_size_t font_size,
		     Font::font_style_t font_style,
                     const Point2i& type_size,
                     const Point2i& name_size,
                     const Point2i& score_size)
  : HBox(rect, _visible)
{
  Point2i pos(0, 0);
  Point2i posZero(0,0);

  margin = DEF_MARGIN;
  border.SetValues(DEF_BORDER, DEF_BORDER);

  AddWidget(new Label(type_name, Rectanglei(pos, type_size), font_size, font_style));

  pos.SetValues(pos.GetX()+type_size.GetX(), pos.GetY());
  name_lbl = new Label("", Rectanglei(pos, name_size), font_size, font_style);
  AddWidget(name_lbl);

  pos.SetValues(pos.GetX()+name_size.GetX(), pos.GetY());
  score_lbl = new Label("", Rectanglei(pos, score_size), font_size, font_style);
  AddWidget(score_lbl);

  team_picture = new PictureWidget( Rectanglei(0,0,48,48) );
  AddWidget(team_picture);
}

void ResultBox::SetResult(const std::string& name, int score, const Surface& team_logo)
{
  char buffer[16];
  std::string copy_name(name);

  snprintf(buffer, 16, "%i", score);

  std::string score_str(buffer);

  name_lbl->SetText(copy_name);
  score_lbl->SetText(score_str);
  team_picture->SetSurface(team_logo);

  //ForceRedraw();
}

void ResultBox::SetNoResult()
{
  name_lbl->SetText(_("Nobody!"));
  score_lbl->SetText("0");
  team_picture->SetNoSurface();

  //ForceRedraw();
}


ResultsMenu::ResultsMenu(const std::vector<TeamResults*>* v)
  : Menu("menu/bg_play", vOk)
  , results(v)
  , index(0)
  , max_height(DEF_SIZE+3*DEF_BORDER)
  , team_size(360, 40)
  , type_size(200, 40)
  , name_size(150, 40)
  , score_size(40, 40)
{
  Team* winning_team = NULL;
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml",false);
  Point2i pos (0, 0);

  uint x = 60;
  uint y = 60;

  // And the winner is :
  FOR_EACH_TEAM(team)
  {
    // Determine winner
    if (0 < (**team).NbAliveCharacter())
    {
      winning_team = *team;
      break;
    }
  }

  // And the winner is :
  if (winning_team) {
    jukebox.Play("share","victory");

    winner_box = new VBox(Rectanglei(x, y, 180, 0), true);
    winner_box->AddWidget(new Label(_("Winner"), Rectanglei(0,0, 180,1), Font::FONT_LARGE, Font::FONT_BOLD));
    PictureWidget* winner_logo = new PictureWidget( Rectanglei(0,0,96,96));
    winner_logo->SetSurface(winning_team->flag, true);
    winner_box->AddWidget(winner_logo);
    winner_box->AddWidget(new Label(winning_team->GetName(), Rectanglei(0,0, 180,1), Font::FONT_MEDIUM, Font::FONT_NORMAL));

    std::string tmp = _("Controlled by: ") + winning_team->GetPlayerName();
    winner_box->AddWidget(new Label(tmp, Rectanglei(0,0, 180,1), Font::FONT_MEDIUM, Font::FONT_NORMAL));
    
    widgets.AddWidget(winner_box);
  }
  x+=200;
				   
  //Team selection
  team_box = new HBox(Rectanglei(x, y, 0, max_height), true);
  team_box->SetMargin(DEF_MARGIN);
  team_box->SetBorder(Point2i(DEF_BORDER, DEF_BORDER));

  bt_prev_team = new Button(Rectanglei(pos, Point2i(DEF_SIZE, DEF_SIZE)),
                            res, "menu/arrow-left");
  team_box->AddWidget(bt_prev_team);

  pos.SetValues(pos.GetX()+DEF_SIZE, pos.GetY());

  HBox* tmp_box = new HBox( Rectanglei(pos, team_size), false);
  team_logo = new PictureWidget( Rectanglei(0,0,48,48) );
  tmp_box->AddWidget(team_logo);

  pos.SetValues(pos.GetX()+team_logo->GetSizeX(),pos.GetY());
  team_name = new Label("", Rectanglei(pos, team_size-48), Font::FONT_BIG, Font::FONT_NORMAL);
  tmp_box->AddWidget(team_name);

  team_box->AddWidget(tmp_box);
  pos.SetValues(pos.GetX()+team_size.GetX(), pos.GetY());
  bt_next_team = new Button(Rectanglei(pos, Point2i(DEF_SIZE, DEF_SIZE)),
                            res, "menu/arrow-right");
  team_box->AddWidget(bt_next_team);

  widgets.AddWidget(team_box);

  resource_manager.UnLoadXMLProfile(res);

  //Results
  statistics_box = new VBox(Rectanglei(x, y+int(1.5*max_height), 510, 0), true);

  most_violent = new ResultBox(Rectanglei(0,0,0, max_height),
                               false, _("Most violent"), Font::FONT_BIG, Font::FONT_NORMAL,
                               type_size, name_size, score_size);
  statistics_box->AddWidget(most_violent);

  most_useful = new ResultBox(Rectanglei(0,0,0, max_height),
                               false, _("Most useful"), Font::FONT_BIG, Font::FONT_NORMAL,
                               type_size, name_size, score_size);
  statistics_box->AddWidget(most_useful);

  most_useless = new ResultBox(Rectanglei(0,0,0, max_height),
                               false, _("Most useless"), Font::FONT_BIG, Font::FONT_NORMAL,
                               type_size, name_size, score_size);
  statistics_box->AddWidget(most_useless);

  biggest_traitor = new ResultBox(Rectanglei(0,0,0, max_height),
                                  false, _("Most sold-out"), Font::FONT_BIG, Font::FONT_NORMAL,
                                  type_size, name_size, score_size);
  statistics_box->AddWidget(biggest_traitor);

  widgets.AddWidget(statistics_box);

  SetResult(0);
}

ResultsMenu::~ResultsMenu()
{
}

void ResultsMenu::SetResult(int i)
{
  const Character* player = NULL;
  const TeamResults* res = NULL;
  std::string name;

  DrawBackground();
  b_ok->ForceRedraw();
  winner_box->ForceRedraw();

  index = i;
  if (index < 0) index = results->size()-1;
  else if (index>(int)results->size()-1) index = 0;
  res = (*results)[index];
  assert(res);

  //Team header
  name = res->getTeamName();
  if (res->getTeamLogo() == NULL) {
    team_logo->SetNoSurface();
  }  else  {
    team_logo->SetSurface( *(res->getTeamLogo()) );
  }

  team_name->SetText(name);
  team_box->ForceRedraw();

  //Most violent
  player = res->getMostViolent();
  if(player)
    most_violent->SetResult(player->GetName(), player->GetDamageStats().GetMostDamage(), player->GetTeam().flag);
  else
    most_violent->SetNoResult();

  //Most useful
  player = res->getMostUseful();
  if(player)
    most_useful->SetResult(player->GetName(), player->GetDamageStats().GetOtherDamage(), player->GetTeam().flag);
  else
    most_useful->SetNoResult();

  //Most useless
  player = res->getMostUseless();
  if(player)
    most_useless->SetResult(player->GetName(), player->GetDamageStats().GetOtherDamage(), player->GetTeam().flag);
  else
    most_useless->SetNoResult();

  // Biggest sold-out
  player = res->getBiggestTraitor();
  if(player)
    biggest_traitor->SetResult(player->GetName(), player->GetDamageStats().GetOwnDamage(), player->GetTeam().flag);
  else
    biggest_traitor->SetNoResult();

  statistics_box->ForceRedraw();
}

void ResultsMenu::OnClickUp(const Point2i &mousePosition, int button)
{
  if (bt_prev_team->Contains(mousePosition))
    SetResult(index-1);
  else if ( bt_next_team->Contains(mousePosition))
    SetResult(index+1);
}

void ResultsMenu::OnClick(const Point2i &mousePosition, int button)
{
  // Do nothing if user has not released the button
}

void ResultsMenu::Draw(const Point2i &mousePosition)
{
}

