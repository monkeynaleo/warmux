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
 * Menu du jeu permettant de lancer une partie, modifier les options, d'obtenir
 * des informations, ou encore quitter le jeu.
 *****************************************************************************/

#include "options_menu.h"
//-----------------------------------------------------------------------------
#include "../gui/button.h"
#include "../gui/list_box.h"
#include "../gui/check_box.h"
#include "../gui/spin_button.h"
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
#include "../gui/question.h"
using namespace Wormux;
using namespace std;

//-----------------------------------------------------------------------------
OptionMenu options_menu;
//-----------------------------------------------------------------------------


const uint PAUSE = 10;

const uint CARTE_Y = 30;
const uint CARTE_LARG=300;
const uint CARTE_HAUT=300;

const uint MAPS_Y = 10;
const uint MAPS_LARG = 160;
const uint MAPS_HAUT = 260;

const uint TEAMS_Y = 30;
const uint TEAMS_LARG = 160;
const uint TEAMS_HAUT = CARTE_HAUT;

const uint ECUSSON_HAUT = 48;
const uint ECUSSON_LARG = 48;
const uint ECUSSON_Y = 320;

const uint VIDEO_MODE_Y = 400;
const uint VIDEO_MODE_LARG = 160;
const uint VIDEO_MODE_HAUT = 80;

const uint SOUND_FREQ_Y = 480;
const uint SOUND_FREQ_WIDTH = 120;
const uint SOUND_FREQ_HEIGHT = 60;

const uint CBOX_ENERGIE_Y = 400;
const uint FULL_SCREEN_Y = 340;

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

  valider = new Button(); 
  enregistrer = new Button();
  annuler = new Button();
   
  lboxMaps = new ListBox(0, 0, MAPS_LARG, MAPS_HAUT);
  lboxTeams = new ListBox(0, 0, TEAMS_LARG, TEAMS_HAUT);
  lboxVideoMode = new ListBox(0, 0, VIDEO_MODE_LARG, VIDEO_MODE_HAUT);
  lboxSoundFreq = new ListBox(0, 0, SOUND_FREQ_WIDTH, SOUND_FREQ_HEIGHT);

  option_affichage_energie = new CheckBox();
  option_affichage_nom = new CheckBox();
  full_screen = new CheckBox();
  opt_music = new CheckBox();
  opt_sound_effects = new CheckBox();

  option_temps_tour = new SpinButton();
  option_temps_fin_tour = new SpinButton();
  option_nb_ver = new SpinButton();
  option_energie_ini = new SpinButton();

}

//-----------------------------------------------------------------------------

#ifdef CL

void OptionMenu::SignalWM_QUIT ()
{ 
   fin_boucle = true;
}

//-----------------------------------------------------------------------------

//Traitement d'une touche clavier relachée

void OptionMenu::TraiteTouche (const CL_InputEvent &touche)
{
  if (touche.id == CL_KEY_ESCAPE)
  {
    fin_boucle = true;
    return;
  }

  if (touche.id == CL_KEY_ENTER)
  {
    EnregistreOptions();
    fin_boucle = true;
    return;
  }
}

#else
// NOTHING
#endif

//-----------------------------------------------------------------------------

#ifdef CL
void OptionMenu::TraiteClic (const CL_InputEvent &event)
{
  if (event.id == CL_MOUSE_LEFT)
  {
    int x = CL_Mouse::get_x();
    int y = CL_Mouse::get_y();
#else
void OptionMenu::onClick ( int x, int y)
{     
#endif
     
    if (valider->Test (x, y)) {
      jukebox.Play("share", "menu/ok");
      EnregistreOptions();
      fin_boucle = true;
    } else if (annuler->Test (x, y)) {
      jukebox.Play("share", "menu/cancel");
      fin_boucle = true;
    } else if (enregistrer->Test (x, y)) {
      EnregistreOptions();
    } else if (lboxMaps->Clic(x, y)) {
       ChangeTerrain();
    } else if (lboxVideoMode->Clic(x, y)) {
    } else if (full_screen->Clic(x, y)) {
    } else if (lboxTeams->Clic(x, y)) {
    } else if (lboxSoundFreq->Clic(x,y)) {
    } else if (option_affichage_energie->Clic(x, y)) {
    } else if (option_affichage_nom->Clic(x, y)) {
    } else if (option_temps_tour->Clic(x, y)) {
    } else if (option_temps_fin_tour->Clic(x, y)) {
    } else if (option_nb_ver->Clic(x, y)) {
    } else if (option_energie_ini->Clic(x, y)) {
    } else if (opt_music->Clic (x,y)) {
    } else if (opt_sound_effects->Clic (x,y)) {
    }
    return;
}

//-----------------------------------------------------------------------------

#ifdef CL
void OptionMenu::RetourMenuPrincipal ()
{
  fin_boucle = true;
}
#endif

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
  espace = (video.GetWidth()-CARTE_LARG-MAPS_LARG-TEAMS_LARG)/4;

#ifdef CL
  fond_option = new CL_Surface("menu/fond_option", graphisme.LitRes());
  fond_maps = new CL_Surface("menu/fond_maps", graphisme.LitRes());
  fond_box = new CL_Surface("menu/fond_box", graphisme.LitRes());
  fond_box2 = new CL_Surface("menu/fond_box2", graphisme.LitRes());

  // Chargement des images des boutons
  valider->SetImage ("menu/valider", graphisme.LitRes());
  enregistrer->SetImage ("menu/enregistrer", graphisme.LitRes());
  annuler->SetImage("menu/annuler", graphisme.LitRes());

#else
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");

  fond_option = new Sprite( resource_manager.LoadImage( res, "menu/fond_option"));
  fond_maps = resource_manager.LoadImage( res, "menu/fond_maps");
  fond_box = resource_manager.LoadImage( res, "menu/fond_box");
  fond_box2 = resource_manager.LoadImage( res, "menu/fond_box2");

  // Chargement des images des boutons
  valider->SetImage (res, "menu/valider");
  enregistrer->SetImage ( res, "menu/enregistrer");
  annuler->SetImage( res, "menu/annuler");

#endif
   
  // CheckBox
  option_affichage_energie->Init (_("Display player energy?"), espace, 0);
  option_affichage_nom->Init (_("Display player's name?"), espace, 0);
  full_screen->Init (_("Fullscreen?"), 0, 0);

  // SpinButton
  option_temps_tour->Init (_("Duration of a turn:"), 0, 0,
			  60, 5, TPS_TOUR_MIN, TPS_TOUR_MAX);
  option_temps_fin_tour->Init (_("Duration of the end of a turn:"), 0, 0,
			   2, 1, TPS_FIN_TOUR_MIN, TPS_FIN_TOUR_MAX);
  option_nb_ver->Init(_("Number of players per team:"), espace, 0,
		     4, 1, NBR_VER_MIN, NBR_VER_MAX);
  option_energie_ini->Init(_("Initial energy:"), espace, 0, 100, 5, 50, 200);

  opt_music->Init (_("Music?"), video.GetWidth()/2, 0);
  opt_sound_effects->Init (_("Sound effects?"), video.GetWidth()/2, 0);

  // ListBox
  lboxMaps->Init ();
  lboxTeams->Init ();
  lboxVideoMode->Init ();
  lboxSoundFreq->Init ();
  
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
#ifdef BUGGY_CODE
  Video::ConstIteratorMode
    mode=video.GetModes().begin(),
    mode_end=video.GetModes().end();
  for (int index=0; mode != mode_end; ++mode, ++index)
  {
    lboxVideoMode->AddItem (index==video.GetCurrentMode(),
			   mode->get_string(),
			   mode->get_string());
  }
#endif
  // Generate sound mode list
  uint current_freq = jukebox.GetFrequency();
  lboxSoundFreq->AddItem (current_freq == 11025, "11 kHz", "11025");
  lboxSoundFreq->AddItem (current_freq == 22050, "22 kHz", "22050");
  lboxSoundFreq->AddItem (current_freq == 44100, "44 kHz", "44100");
}

//-----------------------------------------------------------------------------

void OptionMenu::Reset()
{
  option_affichage_energie->SetValue (config.affiche_energie_ver);
  option_affichage_nom->SetValue (config.affiche_nom_ver);
  full_screen->SetValue (video.IsFullScreen());
  option_temps_tour->SetValue(game_mode.duration_turn);
  option_temps_fin_tour->SetValue(game_mode.duration_turn_end);
  option_nb_ver->SetValue(game_mode.max_characters);
  option_energie_ini->SetValue(game_mode.character.init_energy);

#ifdef CL
  opt_music->SetValue( jukebox.GetMusicConfig() );
  opt_sound_effects->SetValue( jukebox.GetEffectsConfig() );
#else
  opt_music->SetValue( jukebox.UseMusic() );
  opt_sound_effects->SetValue( jukebox.UseEffects() );
#endif
}

//-----------------------------------------------------------------------------

void OptionMenu::EnregistreOptions()
{
  // Save values
  std::string map_id = lboxMaps->ReadLabel(lboxMaps->GetSelectedItem());
  lst_terrain.ChangeTerrainNom (map_id);
  teams_list.ChangeSelection (lboxTeams->GetSelection());
  config.affiche_energie_ver = option_affichage_energie->GetValue();
  config.affiche_nom_ver = option_affichage_nom->GetValue();

  game_mode.duration_turn = option_temps_tour->GetValue() ;
  game_mode.duration_turn_end = option_temps_fin_tour->GetValue() ;
  game_mode.max_characters = option_nb_ver->GetValue() ;

  game_mode.character.init_energy = option_energie_ini->GetValue() ;
#ifdef CL
  video.ChangeMode(lboxVideoMode->GetSelectedItem(),full_screen->GetValue());
#else
   // TODO
#endif
   
  // Sound
#ifdef CL
  jukebox.ActiveSound( opt_sound->GetValue() );
  jukebox.ActiveMusic( opt_music->GetValue() );
  jukebox.ActiveEffects( opt_sound_effects->GetValue() );
  std::string sfreq = lboxSoundFreq.ReadValue(lboxSoundFreq.GetSelectedItem());
  long freq;
  if (str2long(sfreq,freq)) jukebox.SetFrequency (freq);
#else
  jukebox.ActiveMusic( opt_music->GetValue() );
  jukebox.ActiveEffects( opt_sound_effects->GetValue() );
  std::string sfreq = lboxSoundFreq->ReadValue(lboxSoundFreq->GetSelectedItem());
  long freq;
  if (str2long(sfreq,freq)) jukebox.SetFrequency (freq);
  
  jukebox.Init(); // commit modification on sound options
#endif
   
  //Enregistre le tout dans le XML
  config.Sauve();
}

//-----------------------------------------------------------------------------

#ifdef CL
void OptionMenu::ChangeTerrain()
{
  std::string map_id = lboxMaps->ReadLabel(lboxMaps->GetSelectedItem());
  uint map = lst_terrain.FindMapById(map_id);
  map_preview = lst_terrain.liste[map].preview;
  float scale = std::min( float(CARTE_HAUT)/map_preview.get_height(), 
                          float(CARTE_LARG)/map_preview.get_width() ) ;
  carte_haut = (uint)(map_preview.get_height() * scale);
  carte_larg = (uint)(map_preview.get_width() * scale);

  map_preview.set_scale (scale, scale);
  
  carte_x = (espace*2+MAPS_LARG)+(CARTE_LARG-carte_larg)/2;
  carte_y = CARTE_Y+(CARTE_HAUT-carte_haut)/2;
}
#else
void OptionMenu::ChangeTerrain()
{
  std::string map_id = lboxMaps->ReadLabel(lboxMaps->GetSelectedItem());
  uint map = lst_terrain.FindMapById(map_id);
  map_preview = new Sprite(lst_terrain.liste[map].preview);
  float scale = std::min( float(CARTE_HAUT)/map_preview->GetHeight(), 
                          float(CARTE_LARG)/map_preview->GetWidth() ) ;
  carte_haut = (uint)(map_preview->GetHeight() * scale);
  carte_larg = (uint)(map_preview->GetWidth() * scale);

  map_preview->Scale (scale, scale);
  
  carte_x = (espace*2+MAPS_LARG)+(CARTE_LARG-carte_larg)/2;
  carte_y = CARTE_Y+(CARTE_HAUT-carte_haut)/2;
}
#endif

//-----------------------------------------------------------------------------

// Traitement d'une touche clavier relachée
void OptionMenu::Lance ()
{ 
  bool terrain_init = false;
  int x=0, y=0;
  Team* derniere_equipe = teams_list.FindByIndex(0);;

  Init();
  Reset();

#ifdef CL
  //Installe le pilote clavier
  keyboard_slot = CL_Keyboard::sig_key_up().connect(this, &OptionMenu::TraiteTouche);
  mouse_slot = CL_Mouse::sig_key_up().connect(this, &OptionMenu::TraiteClic);
  slot_quit = app.clwindow -> sig_window_close().connect(this, &OptionMenu::SignalWM_QUIT);
#endif
   
  x = (video.GetWidth()/2);
  y = video.GetHeight()-50;
  valider->SetPos (x-50, y);
  enregistrer->SetPos (x, y);
  annuler->SetPos (x+50, y);

  espace = (video.GetWidth()-CARTE_LARG-MAPS_LARG-TEAMS_LARG)/4;
  //espace_h = (video.GetHeight()-CARTE_HAUT-30-100)/4;
  uint centre_x = video.GetWidth()/2;

  lboxMaps->SetXY (espace, 30);
  lboxTeams->SetXY ((espace*3)+CARTE_LARG+MAPS_LARG, TEAMS_Y);
  lboxVideoMode->SetXY ((espace*3)+CARTE_LARG+MAPS_LARG-30, VIDEO_MODE_Y);

  x = espace;
  option_affichage_energie->SetXY (x, CBOX_ENERGIE_Y);
  option_affichage_nom->SetXY (x, CBOX_ENERGIE_Y+20);
  option_temps_tour->SetXY (x, CBOX_ENERGIE_Y+(2*20));
  option_temps_fin_tour->SetXY (x, CBOX_ENERGIE_Y+(3*20));
  option_nb_ver->SetXY (x, CBOX_ENERGIE_Y+(4*20));
  option_energie_ini->SetXY (x, CBOX_ENERGIE_Y+(5*20));

  x = centre_x-50;
  y = FULL_SCREEN_Y;
  full_screen->SetXY (x, y);
  y += 20; opt_music->SetXY (x, y);
  y += 20; opt_sound_effects->SetXY (x, y);

  lboxSoundFreq->SetXY (x, SOUND_FREQ_Y);

  fin_boucle = false;
  do
  {
#ifdef CL
    //Lit la position de la souris
    x = CL_Mouse::get_x();
    y = CL_Mouse::get_y();

    CL_Display::clear();
#else
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
     
#endif
     
    int nv_equipe = lboxTeams->MouseIsOnWitchItem (x,y);
    if (nv_equipe != -1)
    {
      derniere_equipe = teams_list.FindByIndex(nv_equipe);
    }
      
    // affichage des boutons et de la carte
#ifdef CL
    fond_option->draw(CL_Rect(0, 0, video.GetWidth(), video.GetHeight()));
    fond_maps->draw(espace-30, 5);
    fond_maps->draw((espace*3)+CARTE_LARG+MAPS_LARG-30, 5);
    fond_box->draw(espace-30, CBOX_ENERGIE_Y-20);
    fond_box2->draw((espace*3)+CARTE_LARG+MAPS_LARG-100, VIDEO_MODE_Y-25);
#else
    fond_option->Blit( app.sdlwindow, 0, 0);
    SDL_Rect r1 = {espace-30,5,fond_maps->w,fond_maps->h};	  
    SDL_BlitSurface( fond_maps, NULL, app.sdlwindow, &r1);
    SDL_Rect r2 = {(espace*3)+CARTE_LARG+MAPS_LARG-30, 5,fond_maps->w,fond_maps->h};
    SDL_BlitSurface( fond_maps, NULL, app.sdlwindow, &r2);
    SDL_Rect r3 = {espace-30, CBOX_ENERGIE_Y-20,fond_box->w,fond_box->h};
    SDL_BlitSurface( fond_box, NULL, app.sdlwindow, &r3);
    SDL_Rect r4 = {(espace*3)+CARTE_LARG+MAPS_LARG-100, VIDEO_MODE_Y-25,fond_box->w,fond_box->h};
    SDL_BlitSurface( fond_box2, NULL, app.sdlwindow, &r4);
#endif
     
    valider->Draw(x,y) ;
    annuler->Draw(x,y) ;
    enregistrer->Draw(x,y) ;
    lboxMaps->Display(x,y);
    lboxTeams->Display(x,y);
    lboxVideoMode->Display(x,y);
    full_screen->Display(x,y);
    lboxSoundFreq->Display(x,y);
    option_affichage_energie->Display(x,y);
    option_affichage_nom->Display(x,y);
    option_temps_tour->Display(x,y);
    option_temps_fin_tour->Display(x,y);
    option_nb_ver->Display(x,y);
    option_energie_ini->Display(x,y);
    opt_music->Display(x,y);
    opt_sound_effects->Display(x,y);

#ifdef CL
    derniere_equipe -> ecusson.draw ((espace*3)+CARTE_LARG+MAPS_LARG+
				     (TEAMS_LARG/2)-ECUSSON_LARG/2 ,
				     ECUSSON_Y);
#else
   SDL_Rect team_icon_rect = { (espace*3)+CARTE_LARG+MAPS_LARG+(TEAMS_LARG/2)-ECUSSON_LARG/2,
	                       ECUSSON_Y,
                               ECUSSON_LARG,
                               ECUSSON_HAUT};
   SDL_BlitSurface (derniere_equipe->ecusson, NULL, app.sdlwindow, &team_icon_rect); 
     
#endif
     
    if (!terrain_init)
    {
      terrain_init = true;
      ChangeTerrain();
    }

#ifdef CL
    map_preview.draw (carte_x, carte_y);
      
    CL_Display::flip();

    CL_System::keep_alive (PAUSE) ;
#else
    map_preview->Blit ( app.sdlwindow, carte_x, carte_y);     
     
    SDL_Flip( app.sdlwindow);
#endif
     
  } while (!fin_boucle);

#ifdef CL
//  Désinstalle le pilote
  CL_Keyboard::sig_key_up().disconnect(keyboard_slot);
  CL_Mouse::sig_key_up().disconnect(mouse_slot);
#ifdef PORT_CL07
  CL_DisplayWindow::sig_window_close().disconnect(slot_quit);
#endif
#endif
}

//-----------------------------------------------------------------------------
