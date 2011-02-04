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
 * Replay menu
 *****************************************************************************/

#include <fstream>
#include <time.h>

#include <WARMUX_i18n.h>

#include "game/game.h"
#include "game/config.h"
#include "graphic/font.h"
#include "graphic/video.h"
#include "gui/file_list_box.h"
#include "gui/question.h"
#include "gui/select_box.h"
#include "gui/vertical_box.h"
#include "menu/replay_menu.h"
#include "replay/replay.h"
#include "replay/replay_info.h"

ReplayMenu::ReplayMenu()
  : Menu("menu/bg_play")
  , selected(NULL)
{
  // Center the boxes!
  Point2i size = GetMainWindow().GetSize()/10;

  //Replay files
  uint  w = 4*size.GetX();
  file_box = new VBox(w);
  file_box->SetPosition(size);
  file_box->AddWidget(new Label(_("Select a replay:"), w, Font::FONT_BIG));

  Box * tmp_box = new HBox(7*size.GetY(), false);
  tmp_box->SetMargin(5);
  tmp_box->SetBorder(0, 0);
  replay_lbox = new FileListBox(Point2i(w-10, 6*size.GetY()));
  replay_lbox->AddExtensionFilter("dat");
  replay_lbox->StartListing();
  tmp_box->AddWidget(replay_lbox);
  file_box->AddWidget(tmp_box);
  widgets.AddWidget(file_box);

  //Info
  w = 3*size.GetX();
  info_box = new VBox(w);
  info_box->SetPosition(Point2i(6*size.GetX(), size.GetY()));
  info_box->AddWidget(new Label(_("Replay info"), w, Font::FONT_BIG));
  info_box->SetBorder(6, 6);
  Box *panel = new VBox(w-12);
  panel->SetMargin(4);
  
  // Version
  panel->AddWidget(new Label(_("Version"), w-20, Font::FONT_BIG));
  version_lbl = new Label(" ", w-20, Font::FONT_MEDIUM);
  panel->AddWidget(version_lbl);

  // Date
  panel->AddWidget(new Label(_("Date"), w-20, Font::FONT_BIG));
  date_lbl = new Label(" ", w-20, Font::FONT_MEDIUM);
  panel->AddWidget(date_lbl);
  
  // Duration
  panel->AddWidget(new Label(_("Duration"), w-20, Font::FONT_BIG));
  duration_lbl = new Label(" ", w-20, Font::FONT_MEDIUM);
  panel->AddWidget(duration_lbl);

  // Comment
  panel->AddWidget(new Label(_("Comment"), w-20, Font::FONT_BIG));
  comment_lbl = new Label(" ", w-20, Font::FONT_MEDIUM);
  panel->AddWidget(comment_lbl);

  // Teams
  panel->AddWidget(new Label(_("Teams"), w-20, Font::FONT_BIG));
  teams_lbox = new ScrollBox(Point2i(w-20, 4*size.GetY()));
  panel->AddWidget(teams_lbox);
  
  // Finish info box
  info_box->AddWidget(panel);
 
  widgets.AddWidget(info_box);
  widgets.Pack();
}

ReplayMenu::~ReplayMenu()
{
}

void ReplayMenu::ClearReplayInfo()
{
  version_lbl->SetText("");
  date_lbl->SetText("");
  duration_lbl->SetText("");
  comment_lbl->SetText("");
  teams_lbox->Clear();
}

void ReplayMenu::ChangeReplay()
{
  const std::string *name = replay_lbox->GetSelectedFile();
  selected = NULL;
  if (!name) {
    ClearReplayInfo();
    return;
  }

  // Get info from Replay:: and fill in here
  std::ifstream in(name->c_str(), std::fstream::binary);
  if (!in) {
    Question question;
    std::string err = "Error: file ";
    err += *name;
    err += " not found";
    question.Set(err, true, 0);
    question.Ask();
    return; /* File deleted meanwhile ? */
  }

  ReplayInfo *info = ReplayInfo::ReplayInfoFromFile(in);
  in.close();

  if (!info)
    return; /* Bad problem */
  
  // Below gets risky to analyze so error out
  if (!info->IsValid()) {
    const std::string& err = info->GetLastError();

    // Clean current state
    ClearReplayInfo();
    
    Question question;
    std::cerr << "Error: " << err << "\n";
    question.Set(err, true, 0);
    question.Ask();

    delete info;
    return;
  }

  // Version
  std::string text = info->GetVersion();
  version_lbl->SetText(text);

  // Date
  time_t      t    = info->GetDate();
  text = ctime(&t);
  if (text.size() > 1)
    text[text.size()-1] = 0;
  date_lbl->SetText(text);

  // Duration
  Uint32 time = (info->GetMillisecondsDuration()+500)/1000;
  char   temp[32];
  snprintf(temp, 32, "%um%us", time/60, time%60);
  text = temp;
  duration_lbl->SetText(text);

  // Comment
  text = info->GetComment();
  comment_lbl->SetText(text);

  // Teams
  teams_lbox->Clear();
  for (uint i=0; i<info->GetTeams().size(); i++) {
    printf("Adding %s\n", info->GetTeams()[i].id.c_str());
    teams_lbox->AddWidget(new Label(info->GetTeams()[i].id, 0, Font::FONT_MEDIUM));
  }
  teams_lbox->Pack();
  teams_lbox->NeedRedrawing();

  delete info;
  selected = name;
}

void ReplayMenu::OnClickUp(const Point2i &mousePosition, int button)
{
  if (widgets.ClickUp(mousePosition, button) == replay_lbox)
    ChangeReplay();
}

bool ReplayMenu::signal_ok()
{
  // Restore path, because paths are expected to be relative afterwards
  if (selected) {
    StartPlaying(*selected);
    return true;
  }
  return false;
}
