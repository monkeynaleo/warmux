/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 * Results menu
 *****************************************************************************/

#include "results_menu.h"

#include "../team/results.h"
#include "../team/character.h"
#include "../include/app.h"
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
  Box   *type_box;
  Box   *name_box;
  Label *name_lbl;
  Box   *score_box;
  Label *score_lbl;

public:
  ResultBox(const Rectanglei &rect, bool _visible,
            const char* type_name, Font& font,
            const Point2i& type_size,
            const Point2i& name_size,
            const Point2i& score_size);
  // Hopefully no need for ~ResultBox() as it automatically
  // destroy child widgets by inheriting from HBox
  void SetResult(const std::string& name, int score);
};

ResultBox::ResultBox(const Rectanglei &rect, bool _visible,
                     const char *type_name, Font& font,
                     const Point2i& type_size,
                     const Point2i& name_size,
                     const Point2i& score_size)
  : HBox(rect, _visible)
{
  Point2i pos(0, 0);
  Point2i posZero(0,0);

  margin = DEF_MARGIN;
  border.SetValues(DEF_BORDER, DEF_BORDER);

  type_box = new HBox( Rectanglei(pos, type_size), true);
  type_box->AddWidget(new Label(type_name, Rectanglei(pos, type_size), font));
  AddWidget(type_box);

  pos.SetValues(pos.GetX()+type_size.GetX(), pos.GetY());
  name_box = new HBox( Rectanglei(pos, name_size), true);
  name_lbl = new Label("", Rectanglei(pos, name_size), font);
  name_box->AddWidget(name_lbl);
  AddWidget(name_box);

  pos.SetValues(pos.GetX()+name_size.GetX(), pos.GetY());
  score_box = new HBox( Rectanglei(pos, score_size), true);
  score_lbl = new Label("", Rectanglei(pos, score_size), font);
  score_box->AddWidget(score_lbl);
  AddWidget(score_box);
}

void ResultBox::SetResult(const std::string& name, int score)
{
  char buffer[16];
  std::string copy_name(name);

  snprintf(buffer, 16, "%i", score);

  std::string score_str(buffer);

  name_lbl->SetText(copy_name);
  score_lbl->SetText(score_str);
}

ResultsMenu::ResultsMenu(const std::vector<TeamResults*>* v,
                         const char *winner_name)
  : Menu("menu/bg_play")
  , results(v)
  , index(0)
  , max_height(DEF_SIZE+3*DEF_BORDER)
  , team_size(200, 40)
  , type_size(160, 40)
  , name_size(250, 40)
  , score_size(60, 40)
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml",false);
  Point2i pos (0, 0);
  Font* big_font = Font::GetInstance(Font::FONT_BIG);

  // Center the boxes!
  uint x = 60;
  uint y = 60;

#if 0 // Determining sizes really doesn't work so hardcoding them at init
  score_size = big_font->GetSize("999");

  // Type size
  type_size = type_size.max(big_font->GetSize(_("Most violent")));
  type_size = type_size.max(big_font->GetSize(_("Most useless")));
  type_size = type_size.max(big_font->GetSize(_("Most usefull")));
  type_size = type_size.max(big_font->GetSize(_("Most sold-out")));

  for (const_res_iterator it=v.begin(), end = v.end(); it!=end; ++it)
  {
    //Team size
    std::string name;

    if ((*it)->getTeamName() == NULL) name = std::string(_("All teams"));
    else name = (*it)->getTeamName();
    team_size = team_size.max(big_font->GetSize(name));

    //Name size
    name = (*it)->getMostViolent()->GetName();
    name_size = name_size.max(big_font->GetSize(name));
    name = (*it)->getMostUsefull()->GetName();
    name_size = name_size.max(big_font->GetSize(name));
    name = (*it)->getMostUseless()->GetName();
    name_size.max(big_font->GetSize(name));
    name = (*it)->getBiggestTraitor()->GetName();
    name_size = name_size.max(big_font->GetSize(name));
  }

  //Scaling
  score_size = score_size*Zoom;
  name_size  = name_size*Zoom;
  type_size  = type_size;
  team_size  = team_size;

  //Total width
  total_width = score_size.GetX() + type_size.GetX() + name_size.GetX();

  //Max height
  if (score_size.GetY() > max_height) max_height = score_size.GetY();
  if (team_size.GetY() > max_height) max_height = team_size.GetY();
  if (name_size.GetY() > max_height) max_height = name_size.GetY();
  if (type_size.GetY() > max_height) max_height = type_size.GetY();
  max_height += 2*DEF_BORDER;

  fprintf(stdout, "Determined:\n"
          "  * team_size = (%i,%i)\n"
          "  * type_size = (%i,%i)\n"
          "  * name_size = (%i,%i)\n"
          "  * score_size = (%i,%i)\n"
          "  * box size = (%i, %i)\n",
          team_size.GetX(), team_size.GetY(),
          type_size.GetX(), type_size.GetY(),
          name_size.GetX(), name_size.GetY(),
          score_size.GetX(), score_size.GetY(),
          total_width, max_height);
#endif

  //Team selection
  team_box = new HBox(Rectanglei(x, y, total_width, max_height), true);
  team_box->SetMargin(DEF_MARGIN);
  team_box->SetBorder(Point2i(DEF_BORDER, DEF_BORDER));
  bt_prev_team = new Button(Rectanglei(pos, Point2i(DEF_SIZE, DEF_SIZE)),
                            res, "menu/arrow-left");
  team_box->AddWidget(bt_prev_team);
  pos.SetValues(pos.GetX()+DEF_SIZE, pos.GetY());
  HBox* tmp_box = new HBox( Rectanglei(pos, team_size), true);
  team_name = new Label("", Rectanglei(pos, team_size), *big_font);
  tmp_box->AddWidget(team_name);
  team_box->AddWidget(tmp_box);
  pos.SetValues(pos.GetX()+team_size.GetX(), pos.GetY());
  bt_next_team = new Button(Rectanglei(pos, Point2i(DEF_SIZE, DEF_SIZE)),
                            res, "menu/arrow-right");
  team_box->AddWidget(bt_next_team);
  
  //Results
  most_violent = new ResultBox(Rectanglei(x, y+int(1.5*max_height), total_width, max_height),
                               true, _("Most violent"), *big_font,
                               type_size, name_size, score_size);
  most_usefull = new ResultBox(Rectanglei(x, y+3*max_height, total_width, max_height),
                               true, _("Most usefull"), *big_font,
                               type_size, name_size, score_size);
  most_useless = new ResultBox(Rectanglei(x, y+int(4.5*max_height), total_width, max_height),
                               true, _("Most useless"), *big_font,
                               type_size, name_size, score_size);
  biggest_traitor = new ResultBox(Rectanglei(x, y+6*max_height, total_width, max_height),
                                  true, _("Most sold-out"), *big_font,
                                  type_size, name_size, score_size);

  SetResult(0);
}

ResultsMenu::~ResultsMenu()
{
  delete team_box;
  delete most_violent;
  delete most_usefull;
  delete most_useless;
  delete biggest_traitor;
}

void ResultsMenu::SetResult(int i)
{
  const Character* player = NULL;
  const TeamResults* res = NULL;
  std::string name;
  
  index = i;
  if (index < 0) index = results->size()-1;
  else if (index>(int)results->size()-1) index = 0;
  res = (*results)[index];
  assert(res);

  //Team name
  if (res->getTeamName() == NULL) name = std::string(_("All teams"));
  else name = res->getTeamName();
  printf("Now result %i/%i: team '%s'\n",
         index, results->size(), name.c_str());
  team_name->SetText(name);

  //Most violent
  player = res->getMostViolent();
  if(player)
    most_violent->SetResult(player->GetName(), player->GetMostDamage());
  else
    most_violent->SetResult(_("Nobody!"), 0);

  //Most usefull
  player = res->getMostUsefull();
  if(player)
    most_usefull->SetResult(player->GetName(), player->GetOtherDamage());
  else
    most_usefull->SetResult(_("Nobody!"), 0);

  //Most usefull
  player = res->getMostUseless();
  if(player)
    most_useless->SetResult(player->GetName(), player->GetOtherDamage());
  else
    most_useless->SetResult(_("Nobody!"), 0);

  // Biggest sold-out
  player = res->getBiggestTraitor();
  if(player)
    biggest_traitor->SetResult(player->GetName(), player->GetOwnDamage());
  else
    biggest_traitor->SetResult(_("Nobody!"), 0);
}

void ResultsMenu::OnClic(const Point2i &mousePosition, int button)
{
  if (bt_prev_team->Contains(mousePosition))
    SetResult(index-1);
  else if ( bt_next_team->Contains(mousePosition))
    SetResult(index+1);
}

void ResultsMenu::Draw(const Point2i &mousePosition)
{
  team_box->Draw(mousePosition);

  most_violent->Draw(mousePosition);
  most_usefull->Draw(mousePosition);
  most_useless->Draw(mousePosition);
  biggest_traitor->Draw(mousePosition);
}

