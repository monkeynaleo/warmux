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
 * Game menu
 *****************************************************************************/

#include "game_menu.h"
//-----------------------------------------------------------------------------

#include "../include/app.h"
#include "../graphic/video.h"
#include "../team/teams_list.h"
#include "../game/game.h"
#include "../game/game_mode.h"
#include "../map/maps_list.h"
#include "../game/config.h"
#include "../tool/i18n.h"
#include "../tool/string_tools.h"

using namespace Wormux;
using namespace std;

//-----------------------------------------------------------------------------

const uint TEAMS_X = 30;
const uint TEAMS_Y = 30;
const uint TEAMS_W = 160;
const uint TEAMS_H = 260;
const uint TEAM_LOGO_Y = 290;
const uint TEAM_LOGO_H = 48;

const uint MAPS_X = TEAMS_X+TEAMS_W+50;
const uint MAPS_Y = TEAMS_Y;
const uint MAPS_W = 160;
const uint MAPS_H = 260;
 
const uint MAP_PREVIEW_W = 300;
const uint MAP_PREVIEW_H = 300+5;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

GameMenu::GameMenu() : Menu("menu/bg_option")
{
  //-----------------------------------------------------------------------------
  // Widget creation
  //-----------------------------------------------------------------------------

  /* Choose the teams !! */
  team_box = new VBox(TEAMS_X, TEAMS_Y, TEAMS_W+10);
  team_box->AddWidget(new Label(_("Select the teams:"), 0, 0, 0, normal_font));

  lbox_teams = new ListBox(0, 0, TEAMS_W, TEAMS_H - TEAM_LOGO_H - 5);
  team_box->AddWidget(lbox_teams);

  team_box->AddWidget(new NullWidget(0, 0, TEAM_LOGO_H, TEAM_LOGO_H));

  /* Choose the map !! */
  Box * tmp_box = new HBox(0, 0, MAPS_H, false);

  lbox_maps = new ListBox(0, 0, MAPS_W, MAPS_H);
  tmp_box->AddWidget(lbox_maps);
  tmp_box->AddWidget(new NullWidget(0, 0, MAP_PREVIEW_W, MAP_PREVIEW_H));
  
  map_box = new VBox(MAPS_X, MAPS_Y, tmp_box->GetW());
  map_box->AddWidget(new Label(_("Select the world:"), 0, 0, 0, normal_font));
  map_box->AddWidget(tmp_box);


  //-----------------------------------------------------------------------------
  // Values initialization
  //-----------------------------------------------------------------------------

  // Load Maps' list
  std::sort(lst_terrain.liste.begin(), lst_terrain.liste.end(), compareMaps);

  ListeTerrain::iterator
    terrain=lst_terrain.liste.begin(),
    fin_terrain=lst_terrain.liste.end();
  for (; terrain != fin_terrain; ++terrain)
  {
    bool choisi = terrain -> name == lst_terrain.TerrainActif().name;
    lbox_maps->AddItem (choisi, terrain -> name, terrain -> name);
  }

  // Load Teams' list
  teams_list.full_list.sort(compareTeams);

  TeamsList::full_iterator
    it=teams_list.full_list.begin(), 
    end=teams_list.full_list.end();
  lbox_teams->selection_max = game_mode.max_teams;
  lbox_teams->selection_min = 2;
  uint i=0;
  for (; it != end; ++it)
  {
    bool choix = teams_list.IsSelected (i);
    lbox_teams->AddItem (choix, (*it).GetName(), (*it).GetName());
    ++i;
  }

}

//-----------------------------------------------------------------------------

GameMenu::~GameMenu()
{
  delete map_box;
  delete team_box;
}

//-----------------------------------------------------------------------------

void GameMenu::OnClic ( int x, int y)
{     
  if (lbox_maps->Clic(x, y)) {
    ChangeMap();
  } else if (lbox_teams->Clic(x, y)) {
  }
}

//-----------------------------------------------------------------------------

void GameMenu::SaveOptions()
{
  // Save values
  std::string map_id = lbox_maps->ReadLabel(lbox_maps->GetSelectedItem());
  lst_terrain.ChangeTerrainNom (map_id);
  teams_list.ChangeSelection (lbox_teams->GetSelection());
   
  //Save options in XML
  config.Sauve();
}

//-----------------------------------------------------------------------------

void GameMenu::__sig_ok()
{
  SaveOptions();
  jeu.LanceJeu();
}

//-----------------------------------------------------------------------------

void GameMenu::__sig_cancel()
{
  // Nothing to do
}

//-----------------------------------------------------------------------------

void GameMenu::__sig_record()
{
  SaveOptions();
}

//-----------------------------------------------------------------------------

void GameMenu::ChangeMap()
{
  std::string map_id = lbox_maps->ReadLabel(lbox_maps->GetSelectedItem());
  uint map = lst_terrain.FindMapById(map_id);
  map_preview = new Sprite(lst_terrain.liste[map].preview);
  float scale = std::min( float(MAP_PREVIEW_H)/map_preview->GetHeight(), 
                          float(MAP_PREVIEW_W)/map_preview->GetWidth() ) ;

  map_preview->Scale (scale, scale);
}

//-----------------------------------------------------------------------------

void GameMenu::Draw(int mouse_x, int mouse_y)
{   
  bool terrain_init = false;  
  Team* derniere_equipe = teams_list.FindByIndex(0);
   
  map_box->Draw(mouse_x,mouse_y);
  team_box->Draw(mouse_x,mouse_y);
     
  int nv_equipe = lbox_teams->MouseIsOnWitchItem (mouse_x,mouse_y);
  if (nv_equipe != -1) {
    derniere_equipe = teams_list.FindByIndex(nv_equipe);
  }
   
  SDL_Rect team_icon_rect = { TEAMS_X+(TEAMS_W/2)-(TEAM_LOGO_H/2),
			      TEAMS_Y + TEAMS_H - TEAM_LOGO_H +5,
			      TEAM_LOGO_H,
			      TEAM_LOGO_H};
  SDL_BlitSurface (derniere_equipe->ecusson, NULL, app.sdlwindow, &team_icon_rect); 
  
  if (!terrain_init)
    {
      terrain_init = true;
      ChangeMap();
    }
  
  map_preview->Blit ( app.sdlwindow, MAPS_X+MAPS_W+10, MAPS_Y+5);  
}

//-----------------------------------------------------------------------------
