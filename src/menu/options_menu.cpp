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
 * Options menu
 *****************************************************************************/

#include "options_menu.h"
//-----------------------------------------------------------------------------
#include "../gui/button.h"
#include "../gui/list_box.h"
#include "../gui/check_box.h"
#include "../gui/spin_button.h"
#include "../gui/box.h"
#include "../gui/question.h"

#include "../tool/resource_manager.h"
#include "../team/teams_list.h"
#include "../game/game_mode.h"
#include "../graphic/video.h"
#include "../map/maps_list.h"
#include "../include/app.h"
#include "../game/config.h"
#include "../tool/i18n.h"
#include "../sound/jukebox.h"
#include "../tool/string_tools.h"

using namespace Wormux;
using namespace std;

//-----------------------------------------------------------------------------

const uint PAUSE = 10;

const uint TEAMS_X = 30;
const uint TEAMS_Y = 30;
const uint TEAMS_W = 160;
const uint TEAMS_H = 260;
const uint TEAM_LOGO_Y = 290;
const uint TEAM_LOGO_W = 48;

const uint MAPS_X = TEAMS_X+TEAMS_W+50;
const uint MAPS_Y = TEAMS_Y;
const uint MAPS_W = 160;
const uint MAPS_H = 260;
 
const uint MAP_PREVIEW_W = 300;
const uint MAP_PREVIEW_H = 300;

const uint GAME_X = TEAMS_X;
const uint GAME_Y = TEAMS_Y+TEAMS_H+TEAM_LOGO_W+50;
const uint GAME_W = 230;
const uint GAME_H = 80;

const uint SOUND_X = GAME_X+GAME_W+50;
const uint SOUND_Y = GAME_Y;
const uint SOUND_W = 180;
const uint SOUND_H = 80;

const uint GRAPHIC_X = SOUND_X+SOUND_W+50;
const uint GRAPHIC_Y = GAME_Y;
const uint GRAPHIC_W = 230;
const uint GRAPHIC_H = 80;

const uint NBR_VER_MIN = 1;
const uint NBR_VER_MAX = 10;
const uint TPS_TOUR_MIN = 10;
const uint TPS_TOUR_MAX = 120;
const uint TPS_FIN_TOUR_MIN = 1;
const uint TPS_FIN_TOUR_MAX = 10;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

OptionMenu::OptionMenu()
{
  close_menu = false ;
  m_init = false;
   
  uint x = (video.GetWidth()/2);
  uint y = video.GetHeight()-50;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");

  /* actions buttons */
  actions_buttons = new Box(x, y, 100, 30, true);

  b_ok = new Button(0, 0, res, "menu/valider"); 
  actions_buttons->AddWidget(b_ok);

  b_record = new Button(0, 0, res, "menu/enregistrer");
  actions_buttons->AddWidget(b_record);

  b_cancel = new Button(0, 0, res, "menu/annuler");
  actions_buttons->AddWidget(b_cancel);

  /* Maps and teams listboxes */
  lboxMaps = new ListBox(MAPS_X, MAPS_Y, MAPS_W, MAPS_H);
  lboxTeams = new ListBox(TEAMS_X, TEAMS_Y, TEAMS_W, TEAMS_H);

  /* Grapic options */
  graphic_options = new Box(GRAPHIC_X, GRAPHIC_Y, GRAPHIC_W, GRAPHIC_H);

  lboxVideoMode = new ListBox(0, 0, 0, 80);
  graphic_options->AddWidget(lboxVideoMode);

  full_screen = new CheckBox(_("Fullscreen?"), 0, 0, 0); 
  graphic_options->AddWidget(full_screen);

  opt_display_wind_particles = new CheckBox(_("Display wind particles?"), 0, 0, 0); 
  graphic_options->AddWidget(opt_display_wind_particles);

  opt_display_energy = new CheckBox(_("Display player energy?"), 0, 0, 0); 
  graphic_options->AddWidget(opt_display_energy);

  opt_display_name = new CheckBox(_("Display player's name?"), 0, 0, 0); 
  graphic_options->AddWidget(opt_display_name);

  /* Sound options */
  sound_options = new Box(SOUND_X, SOUND_Y, SOUND_W, SOUND_H);

  lboxSoundFreq = new ListBox(0, 0, 0, 80);
  sound_options->AddWidget(lboxSoundFreq);

  opt_music = new CheckBox(_("Music?"), 0, 0, 0);
  sound_options->AddWidget(opt_music);

  opt_sound_effects = new CheckBox(_("Sound effects?"), 0, 0, 0);
  sound_options->AddWidget(opt_sound_effects);
  
  /* Game options */
  game_options = new Box(GAME_X, GAME_Y, GAME_W, GAME_H);
  opt_duration_turn = new SpinButton(_("Duration of a turn:"), 0, 0, 0,
				     TPS_TOUR_MIN, 5,
				     TPS_TOUR_MIN, TPS_TOUR_MAX);
  game_options->AddWidget(opt_duration_turn);

  opt_duration_end_turn = new SpinButton(_("Duration of the end of a turn:"), 0, 0, 0,
					 TPS_FIN_TOUR_MIN, 1,
					 TPS_FIN_TOUR_MIN, TPS_FIN_TOUR_MAX);
  game_options->AddWidget(opt_duration_end_turn);

  opt_nb_characters = new SpinButton(_("Number of players per team:"), 0, 0, 0,
				 4, 1,
				 NBR_VER_MIN, NBR_VER_MAX);
  game_options->AddWidget(opt_nb_characters);

  opt_energy_ini = new SpinButton(_("Initial energy:"), 0,0,0,
				      100, 5,
				      50, 200);
  
  game_options->AddWidget(opt_energy_ini);

}

//-----------------------------------------------------------------------------

OptionMenu::~OptionMenu()
{
  close_menu = false ;
  m_init = false;

  delete actions_buttons;
   
  delete lboxMaps;
  delete lboxTeams;

  delete graphic_options;
  delete sound_options;
  delete game_options;
}

//-----------------------------------------------------------------------------

void OptionMenu::onClick ( int x, int y)
{     
  if (b_ok->MouseIsOver (x, y)) {
    jukebox.Play("share", "menu/ok");
    SaveOptions();
    close_menu = true;
  } else if (b_cancel->MouseIsOver (x, y)) {
    jukebox.Play("share", "menu/cancel");
    close_menu = true;
  } else if (b_record->MouseIsOver (x, y)) {
    SaveOptions();
  } else if (lboxMaps->Clic(x, y)) {
    ChangeMap();
  } else if (lboxTeams->Clic(x, y)) {
  } else if (graphic_options->Clic (x,y)) {
  } else if (sound_options->Clic (x,y)) {
  } else if (game_options->Clic (x,y)) {
  }

}

//-----------------------------------------------------------------------------

void OptionMenu::Init ()
{ 
  if (m_init) return;
  m_init = true;

#ifdef CL
  //Enter main loop 
  app.Run();
#else
   // WHY app.run() ????
#endif

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");

  bg_option = new Sprite( resource_manager.LoadImage( res, "menu/bg_option"));
  bg_long_box = resource_manager.LoadImage( res, "menu/bg_long_box");
  bg_small_box = resource_manager.LoadImage( res, "menu/bg_small_box");

  // Load Maps' list
  std::sort(lst_terrain.liste.begin(), lst_terrain.liste.end(), compareMaps);

  ListeTerrain::iterator
    terrain=lst_terrain.liste.begin(),
    fin_terrain=lst_terrain.liste.end();
  for (; terrain != fin_terrain; ++terrain)
  {
    bool choisi = terrain -> name == lst_terrain.TerrainActif().name;
    lboxMaps->AddItem (choisi, terrain -> name, terrain -> name);
  }

  // Load Teams' list
  teams_list.full_list.sort(compareTeams);

  TeamsList::full_iterator
    it=teams_list.full_list.begin(), 
    end=teams_list.full_list.end();
  lboxTeams->selection_max = game_mode.max_teams;
  lboxTeams->selection_min = 2;
  uint i=0;
  for (; it != end; ++it)
  {
    bool choix = teams_list.IsSelected (i);
    lboxTeams->AddItem (choix, (*it).GetName(), (*it).GetName());
    ++i;
  }

  //Generate video mode list
  SDL_Rect **modes;

  /* Get available fullscreen/hardware modes */
  modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

  /* Check is there are any modes available */
  if(modes == (SDL_Rect **)0){
    std::ostringstream ss;
    ss << app.sdlwindow->w << "x" << app.sdlwindow->h ;
    lboxVideoMode->AddItem(false,"No modes available!", ss.str());
  } else {
    for(i=0;modes[i];++i) {
      if (modes[i]->w < 800 || modes[i]->h < 600) break; 
      std::ostringstream ss;
      ss << modes[i]->w << "x" << modes[i]->h ;
      if (modes[i]->w == app.sdlwindow->w && modes[i]->h == app.sdlwindow->h)
	lboxVideoMode->AddItem(true, ss.str(), ss.str());
      else
	lboxVideoMode->AddItem(false, ss.str(), ss.str());
    }
  }

  // Generate sound mode list
  uint current_freq = jukebox.GetFrequency();
  lboxSoundFreq->AddItem (current_freq == 11025, "11 kHz", "11025");
  lboxSoundFreq->AddItem (current_freq == 22050, "22 kHz", "22050");
  lboxSoundFreq->AddItem (current_freq == 44100, "44 kHz", "44100");

  resource_manager.UnLoadXMLProfile( res);
}

//-----------------------------------------------------------------------------

void OptionMenu::Reset()
{
  opt_display_wind_particles->SetValue (config.display_wind_particles);
  opt_display_energy->SetValue (config.affiche_energie_ver);
  opt_display_name->SetValue (config.affiche_nom_ver);
  full_screen->SetValue (video.IsFullScreen());
  opt_duration_turn->SetValue(game_mode.duration_turn);
  opt_duration_end_turn->SetValue(game_mode.duration_turn_end);
  opt_nb_characters->SetValue(game_mode.max_characters);
  opt_energy_ini->SetValue(game_mode.character.init_energy);


  opt_music->SetValue( jukebox.UseMusic() );
  opt_sound_effects->SetValue( jukebox.UseEffects() );
}

//-----------------------------------------------------------------------------

void OptionMenu::SaveOptions()
{
  // Save values
  std::string map_id = lboxMaps->ReadLabel(lboxMaps->GetSelectedItem());
  lst_terrain.ChangeTerrainNom (map_id);
  teams_list.ChangeSelection (lboxTeams->GetSelection());
  config.display_wind_particles = opt_display_wind_particles->GetValue();
  config.affiche_energie_ver = opt_display_energy->GetValue();
  config.affiche_nom_ver = opt_display_name->GetValue();

  game_mode.duration_turn = opt_duration_turn->GetValue() ;
  game_mode.duration_turn_end = opt_duration_end_turn->GetValue() ;
  game_mode.max_characters = opt_nb_characters->GetValue() ;

  game_mode.character.init_energy = opt_energy_ini->GetValue() ;

  // Video mode
  uint mode = lboxVideoMode->GetSelectedItem();
  std::string s_mode = lboxVideoMode->ReadValue(mode);
  int w, h;
  sscanf(s_mode.c_str(),"%dx%d", &w, &h);
  video.SetConfig(w, h, full_screen->GetValue());

   
  // Sound
  jukebox.ActiveMusic( opt_music->GetValue() );
  jukebox.ActiveEffects( opt_sound_effects->GetValue() );
  std::string sfreq = lboxSoundFreq->ReadValue(lboxSoundFreq->GetSelectedItem());
  long freq;
  if (str2long(sfreq,freq)) jukebox.SetFrequency (freq);
  
  jukebox.Init(); // commit modification on sound options
   
  //Save options in XML
  config.Sauve();
}

//-----------------------------------------------------------------------------

void OptionMenu::ChangeMap()
{
  std::string map_id = lboxMaps->ReadLabel(lboxMaps->GetSelectedItem());
  uint map = lst_terrain.FindMapById(map_id);
  map_preview = new Sprite(lst_terrain.liste[map].preview);
  float scale = std::min( float(MAP_PREVIEW_H)/map_preview->GetHeight(), 
                          float(MAP_PREVIEW_W)/map_preview->GetWidth() ) ;

  map_preview->Scale (scale, scale);
}

//-----------------------------------------------------------------------------

// Traitement d'une touche clavier relachée
void OptionMenu::Run ()
{ 
  bool terrain_init = false;
  int x=0, y=0;
  Team* derniere_equipe = teams_list.FindByIndex(0);;

  Init();
  Reset();

  close_menu = false;
  do
  {
   // Poll and treat events
	
   SDL_Event event;
     
   while( SDL_PollEvent( &event) ) 
     {      
	if ( event.type == SDL_QUIT) 
	  {  
	     close_menu = true;
	  }
	else if ( event.type == SDL_KEYDOWN )
	  {	       
	     switch ( event.key.keysym.sym)
	       { 
		case SDLK_ESCAPE: 
		  close_menu = true;
		  break;
		  
		case SDLK_RETURN: 
		  SaveOptions();
		  close_menu = true;
		  break;
		  
		default:
		  break;
	       }  
	  }
	else if ( event.type == SDL_MOUSEBUTTONDOWN )
	  {
	     onClick( event.button.x, event.button.y);
	  }
     }

   SDL_GetMouseState( &x, &y);
     
    int nv_equipe = lboxTeams->MouseIsOnWitchItem (x,y);
    if (nv_equipe != -1)
    {
      derniere_equipe = teams_list.FindByIndex(nv_equipe);
    }
      
    // affichage des boutons et de la carte
    bg_option->ScaleSize(app.sdlwindow->w, app.sdlwindow->h);
    bg_option->Blit( app.sdlwindow, 0, 0);

    SDL_Rect r1 = {TEAMS_X-30,TEAMS_Y-30,bg_long_box->w,bg_long_box->h};	  
    SDL_BlitSurface( bg_long_box, NULL, app.sdlwindow, &r1);

    SDL_Rect r2 = {MAPS_X-30, MAPS_Y-30,bg_long_box->w,bg_long_box->h};
    SDL_BlitSurface( bg_long_box, NULL, app.sdlwindow, &r2);

    SDL_Rect r3 = {GAME_X-30, GAME_Y-30,bg_small_box->w,bg_small_box->h};
    SDL_BlitSurface( bg_small_box, NULL, app.sdlwindow, &r3);

    SDL_Rect r4 = {SOUND_X-30, SOUND_Y-30,bg_small_box->w,bg_small_box->h};
    SDL_BlitSurface( bg_small_box, NULL, app.sdlwindow, &r4);

    SDL_Rect r5 = {GRAPHIC_X-30, GRAPHIC_Y-30,bg_small_box->w,bg_small_box->h};
    SDL_BlitSurface( bg_small_box, NULL, app.sdlwindow, &r5);
     
    actions_buttons->Draw(x,y);
    lboxMaps->Draw(x,y);
    lboxTeams->Draw(x,y);

    graphic_options->Draw(x, y);
    sound_options->Draw(x,y);
    game_options->Draw(x,y);
    

    SDL_Rect team_icon_rect = { TEAMS_X+(TEAMS_W/2)-(TEAM_LOGO_W/2),
				TEAMS_Y+TEAMS_H,
				TEAM_LOGO_W,
				TEAM_LOGO_W};
    SDL_BlitSurface (derniere_equipe->ecusson, NULL, app.sdlwindow, &team_icon_rect); 
     
    if (!terrain_init)
    {
      terrain_init = true;
      ChangeMap();
    }

    map_preview->Blit ( app.sdlwindow, MAPS_X+MAPS_W+50, MAPS_Y);
     
    SDL_Flip( app.sdlwindow);
     
  } while (!close_menu);

}

//-----------------------------------------------------------------------------
