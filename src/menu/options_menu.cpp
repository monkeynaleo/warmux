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
#include "../graphic/graphism.h"
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


const uint MAPS_X = 30;
const uint MAPS_Y = 30;
const uint MAPS_W = 160;
const uint MAPS_H = 260;
 
const uint MAP_PREVIEW_W = 300;
const uint MAP_PREVIEW_H = 300;

const uint TEAMS_X = 550;
const uint TEAMS_Y = MAPS_Y;
const uint TEAMS_W = MAPS_W;
const uint TEAMS_H = MAPS_H;

const uint TEAM_LOGO_Y = 320;
const uint TEAM_LOGO_W = 48;

const uint CBOX_ENERGIE_Y = 400;
const uint FULL_SCREEN_Y = 340;

const uint SOUND_X = 50;
const uint SOUND_Y = 400;
const uint SOUND_W = 230;
const uint SOUND_H = 80;

const uint GRAPHIC_X = 350;
const uint GRAPHIC_Y = 400;
const uint GRAPHIC_W = 230;
const uint GRAPHIC_H = 80;

const uint GAME_X = 650;
const uint GAME_Y = 400;
const uint GAME_W = 230;
const uint GAME_H = 80;


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
  fin_boucle = false ;
  m_init = false;
   
  uint x = (video.GetWidth()/2);
  uint y = video.GetHeight()-50;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");

  /* actions buttons */
  actions_buttons = new Box(x, y, 100, 30, true);

  valider = new Button(0, 0, res, "menu/valider"); 
  actions_buttons->AddWidget(valider);

  enregistrer = new Button(0, 0, res, "menu/enregistrer");
  actions_buttons->AddWidget(enregistrer);

  annuler = new Button(0, 0, res, "menu/annuler");
  actions_buttons->AddWidget(annuler);

  /* Maps and teams listboxes */
  lboxMaps = new ListBox(MAPS_X, MAPS_Y, MAPS_W, MAPS_H);
  lboxTeams = new ListBox(TEAMS_X, TEAMS_Y, TEAMS_W, TEAMS_H);

  /* Grapic options */
  graphic_options = new Box(GRAPHIC_X, GRAPHIC_Y, GRAPHIC_W, GRAPHIC_H);

  lboxVideoMode = new ListBox(0, 0, 0, 80);
  graphic_options->AddWidget(lboxVideoMode);

  full_screen = new CheckBox(_("Fullscreen?"), 0, 0, 0); 
  graphic_options->AddWidget(full_screen);

  option_display_wind_particles = new CheckBox(_("Display wind particles?"), 0, 0, 0); 
  graphic_options->AddWidget(option_display_wind_particles);

  option_affichage_energie = new CheckBox(_("Display player energy?"), 0, 0, 0); 
  graphic_options->AddWidget(option_affichage_energie);

  option_affichage_nom = new CheckBox(_("Display player's name?"), 0, 0, 0); 
  graphic_options->AddWidget(option_affichage_nom);

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
  option_temps_tour = new SpinButton(_("Duration of a turn:"), 0, 0, 0,
				     TPS_TOUR_MIN, 5,
				     TPS_TOUR_MIN, TPS_TOUR_MAX);
  game_options->AddWidget(option_temps_tour);

  option_temps_fin_tour = new SpinButton(_("Duration of the end of a turn:"), 0, 0, 0,
					 TPS_FIN_TOUR_MIN, 1,
					 TPS_FIN_TOUR_MIN, TPS_FIN_TOUR_MAX);
  game_options->AddWidget(option_temps_fin_tour);

  option_nb_ver = new SpinButton(_("Number of players per team:"), 0, 0, 0,
				 4, 1,
				 NBR_VER_MIN, NBR_VER_MAX);
  game_options->AddWidget(option_nb_ver);

  option_energie_ini = new SpinButton(_("Initial energy:"), 0,0,0,
				      100, 5,
				      50, 200);
  
  game_options->AddWidget(option_energie_ini);

}

//-----------------------------------------------------------------------------

OptionMenu::~OptionMenu()
{
  fin_boucle = false ;
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
  if (valider->MouseIsOver (x, y)) {
    jukebox.Play("share", "menu/ok");
    EnregistreOptions();
    fin_boucle = true;
  } else if (annuler->MouseIsOver (x, y)) {
    jukebox.Play("share", "menu/cancel");
    fin_boucle = true;
  } else if (enregistrer->MouseIsOver (x, y)) {
    EnregistreOptions();
  } else if (lboxMaps->Clic(x, y)) {
    ChangeTerrain();
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

  espace = 30 ; //(video.GetWidth()-CARTE_LARG-MAPS_LARG-TEAMS_LARG)/4;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");

  fond_option = new Sprite( resource_manager.LoadImage( res, "menu/fond_option"));
  fond_maps = resource_manager.LoadImage( res, "menu/fond_maps");
  fond_box = resource_manager.LoadImage( res, "menu/fond_box");
  fond_box2 = resource_manager.LoadImage( res, "menu/fond_box2");

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
  option_display_wind_particles->SetValue (config.display_wind_particles);
  option_affichage_energie->SetValue (config.affiche_energie_ver);
  option_affichage_nom->SetValue (config.affiche_nom_ver);
  full_screen->SetValue (video.IsFullScreen());
  option_temps_tour->SetValue(game_mode.duration_turn);
  option_temps_fin_tour->SetValue(game_mode.duration_turn_end);
  option_nb_ver->SetValue(game_mode.max_characters);
  option_energie_ini->SetValue(game_mode.character.init_energy);


  opt_music->SetValue( jukebox.UseMusic() );
  opt_sound_effects->SetValue( jukebox.UseEffects() );
}

//-----------------------------------------------------------------------------

void OptionMenu::EnregistreOptions()
{
  // Save values
  std::string map_id = lboxMaps->ReadLabel(lboxMaps->GetSelectedItem());
  lst_terrain.ChangeTerrainNom (map_id);
  teams_list.ChangeSelection (lboxTeams->GetSelection());
  config.display_wind_particles = option_display_wind_particles->GetValue();
  config.affiche_energie_ver = option_affichage_energie->GetValue();
  config.affiche_nom_ver = option_affichage_nom->GetValue();

  game_mode.duration_turn = option_temps_tour->GetValue() ;
  game_mode.duration_turn_end = option_temps_fin_tour->GetValue() ;
  game_mode.max_characters = option_nb_ver->GetValue() ;

  game_mode.character.init_energy = option_energie_ini->GetValue() ;

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
   
  //Enregistre le tout dans le XML
  config.Sauve();
}

//-----------------------------------------------------------------------------

void OptionMenu::ChangeTerrain()
{
  std::string map_id = lboxMaps->ReadLabel(lboxMaps->GetSelectedItem());
  uint map = lst_terrain.FindMapById(map_id);
  map_preview = new Sprite(lst_terrain.liste[map].preview);
  float scale = std::min( float(MAP_PREVIEW_H)/map_preview->GetHeight(), 
                          float(MAP_PREVIEW_W)/map_preview->GetWidth() ) ;
  carte_haut = (uint)(map_preview->GetHeight() * scale);
  carte_larg = (uint)(map_preview->GetWidth() * scale);

  map_preview->Scale (scale, scale);
  
  carte_x = (espace*2+MAPS_W)+(MAP_PREVIEW_W-carte_larg)/2;
  carte_y = MAPS_Y+(MAP_PREVIEW_H-carte_haut)/2;
}

//-----------------------------------------------------------------------------

// Traitement d'une touche clavier relachée
void OptionMenu::Lance ()
{ 
  bool terrain_init = false;
  int x=0, y=0;
  Team* derniere_equipe = teams_list.FindByIndex(0);;

  Init();
  Reset();

  espace = (video.GetWidth()-MAP_PREVIEW_W-MAPS_W-TEAMS_W)/4;
  //espace_h = (video.GetHeight()-CARTE_HAUT-30-100)/4;

  fin_boucle = false;
  do
  {
   // Poll and treat events
	
   SDL_Event event;
     
   while( SDL_PollEvent( &event) ) 
     {      
	if ( event.type == SDL_QUIT) 
	  {  
	     fin_boucle = true;
	  }
	else if ( event.type == SDL_KEYDOWN )
	  {	       
	     switch ( event.key.keysym.sym)
	       { 
		case SDLK_ESCAPE: 
		  fin_boucle = true;
		  break;
		  
		case SDLK_RETURN: 
		  EnregistreOptions();
		  fin_boucle = true;
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
    fond_option->ScaleSize(app.sdlwindow->w, app.sdlwindow->h);
    fond_option->Blit( app.sdlwindow, 0, 0);
    SDL_Rect r1 = {espace-30,5,fond_maps->w,fond_maps->h};	  
    SDL_BlitSurface( fond_maps, NULL, app.sdlwindow, &r1);
    SDL_Rect r2 = {(espace*3)+MAP_PREVIEW_W+MAPS_W-30, 5,fond_maps->w,fond_maps->h};
    SDL_BlitSurface( fond_maps, NULL, app.sdlwindow, &r2);
    SDL_Rect r3 = {espace-30, CBOX_ENERGIE_Y-20,fond_box->w,fond_box->h};
    SDL_BlitSurface( fond_box, NULL, app.sdlwindow, &r3);
    SDL_Rect r4 = {(espace*3)+MAP_PREVIEW_W+MAPS_W-100, GRAPHIC_Y-25,fond_box->w,fond_box->h};
    SDL_BlitSurface( fond_box2, NULL, app.sdlwindow, &r4);
     
    valider->Draw(x,y) ;
    annuler->Draw(x,y) ;
    enregistrer->Draw(x,y) ;
    lboxMaps->Draw(x,y);
    lboxTeams->Draw(x,y);

    graphic_options->Draw(x, y);
    sound_options->Draw(x,y);
    game_options->Draw(x,y);
    

   SDL_Rect team_icon_rect = { (espace*3)+MAP_PREVIEW_W+MAPS_W+(TEAMS_W/2)-TEAM_LOGO_W/2,
	                       TEAM_LOGO_W,
                               TEAM_LOGO_W,
                               TEAM_LOGO_W};
   SDL_BlitSurface (derniere_equipe->ecusson, NULL, app.sdlwindow, &team_icon_rect); 
     
    if (!terrain_init)
    {
      terrain_init = true;
      ChangeTerrain();
    }

    map_preview->Blit ( app.sdlwindow, carte_x, carte_y);     
     
    SDL_Flip( app.sdlwindow);
     
  } while (!fin_boucle);

}

//-----------------------------------------------------------------------------
