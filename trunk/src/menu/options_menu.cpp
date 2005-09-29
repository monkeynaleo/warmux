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



// const uint PAUSE = 10;

// const uint CARTE_Y = 30;
// const uint CARTE_LARG=300;
// const uint CARTE_HAUT=300;

// const uint MAPS_Y = 10;
// const uint MAPS_LARG = 160;
// const uint MAPS_HAUT = 260;

// const uint TEAMS_Y = 30;
// const uint TEAMS_LARG = 160;
// const uint TEAMS_HAUT = CARTE_HAUT;

// const uint ECUSSON_HAUT = 48;
// const uint ECUSSON_LARG = 48;
// const uint ECUSSON_Y = 320;

// const uint VIDEO_MODE_Y = 400;
// const uint VIDEO_MODE_LARG = 160;
// const uint VIDEO_MODE_HAUT = 80;

// const uint SOUND_FREQ_Y = 480;
// const uint SOUND_FREQ_WIDTH = 120;
// const uint SOUND_FREQ_HEIGHT = 60;

// const uint CBOX_ENERGIE_Y = 400;
// const uint FULL_SCREEN_Y = 340;

// const uint NBR_VER_MIN = 1;
// const uint NBR_VER_MAX = 10;
// const uint TPS_TOUR_MIN = 10;
// const uint TPS_TOUR_MAX = 120;
// const uint TPS_FIN_TOUR_MIN = 1;
// const uint TPS_FIN_TOUR_MAX = 10;

// //-----------------------------------------------------------------------------
// //-----------------------------------------------------------------------------

// MenuOption::MenuOption() :
//   lboxMaps (0, 0, MAPS_LARG, MAPS_HAUT),
//   lboxTeams (0, 0, TEAMS_LARG, TEAMS_HAUT),
// #ifdef BUGGY_CODE
//   lboxVideoMode (0, 0, VIDEO_MODE_LARG, VIDEO_MODE_HAUT),
// #endif
//   lboxSoundFreq(0, 0, SOUND_FREQ_WIDTH, SOUND_FREQ_HEIGHT)
// {
//   fin_boucle = false ;
//   m_init = false;
// }

// //-----------------------------------------------------------------------------

// void MenuOption::SignalWM_QUIT ()
// { fin_boucle = true; }

// //-----------------------------------------------------------------------------

// // Traitement d'une touche clavier relachée
// void MenuOption::TraiteTouche (const CL_InputEvent &touche)
// {
//   if (touche.id == CL_KEY_ESCAPE)
//   {
//     fin_boucle = true;
//     return;
//   }

//   if (touche.id == CL_KEY_ENTER)
//   {
//     EnregistreOptions();
//     fin_boucle = true;
//     return;
//   }
// }

// //-----------------------------------------------------------------------------

// void MenuOption::TraiteClic (const CL_InputEvent &event)
// {
//   if (event.id == CL_MOUSE_LEFT)
//   {
//     int x = CL_Mouse::get_x();
//     int y = CL_Mouse::get_y();
    
//     if (valider.Test (x, y)) {
//       jukebox.Play("menu/ok");
//       EnregistreOptions();
//       RetourMenuPrincipal();
//     } else if (annuler.Test (x, y)) {
//       jukebox.Play("menu/cancel");
//       RetourMenuPrincipal();
//     } else if (enregistrer.Test (x, y)) {
//       EnregistreOptions();
//     } else if (lboxMaps.Clic(x, y)) {
//       ChangeTerrain();
// #ifdef BUGGY_CODE
//     } else if (lboxVideoMode.Clic(x, y)) {
//     } else if (full_screen.Clic(x, y)) {
// #endif
//     } else if (lboxTeams.Clic(x, y)) {
//     } else if (lboxSoundFreq.Clic(x,y)) {
//     } else if (option_affichage_energie.Clic(x, y)) {
//     } else if (option_affichage_nom.Clic(x, y)) {
// #ifdef USE_SDL
//     } else if (use_sdl.Clic(x, y)) {
// #endif
//     } else if (option_temps_tour.Clic(x, y)) {
//     } else if (option_temps_fin_tour.Clic(x, y)) {
//     } else if (option_nb_ver.Clic(x, y)) {
//     } else if (option_energie_ini.Clic(x, y)) {
//     } else if (opt_sound.Clic (x,y)) {
//     } else if (opt_music.Clic (x,y)) {
//     } else if (opt_sound_effects.Clic (x,y)) {
//     }
//     return;
//   }
// }

// //-----------------------------------------------------------------------------

// void MenuOption::RetourMenuPrincipal ()
// {
//   fin_boucle = true;
// }

// //-----------------------------------------------------------------------------

// void MenuOption::Init ()
// { 
//   if (m_init) return;
//   m_init = true;

//   app.LoadLayout("pokus.xml");
	
//   // Enter main loop 
//   app.Run();

//   espace = (video.GetWidth()-CARTE_LARG-MAPS_LARG-TEAMS_LARG)/4;

//   fond_option = CL_Surface("menu/fond_option", graphisme.LitRes());
//   fond_maps = CL_Surface("menu/fond_maps", graphisme.LitRes());
//   fond_box = CL_Surface("menu/fond_box", graphisme.LitRes());
//   fond_box2 = CL_Surface("menu/fond_box2", graphisme.LitRes());

//   // Chargement des images des boutons
//   valider.SetImage ("menu/valider", graphisme.LitRes());
//   enregistrer.SetImage ("menu/enregistrer", graphisme.LitRes());
//   annuler.SetImage("menu/annuler", graphisme.LitRes());

//   // CheckBox
//   option_affichage_energie.Init (_("Display player energy?"), espace, 0);
//   option_affichage_nom.Init (_("Display player's name?"), espace, 0);
// #ifdef BUGGY_CODE
//   full_screen.Init (_("Fullscreen?"), 0, 0);
// #endif
// #ifdef USE_SDL
//   use_sdl.Init (_("Use SDL?"), 0, 0);
// #endif

//   // SpinButton
//   option_temps_tour.Init (_("Duration of a turn:"), 0, 0,
// 			  60, 5, TPS_TOUR_MIN, TPS_TOUR_MAX);
//   option_temps_fin_tour.Init (_("Duration of the end of a turn:"), 0, 0,
// 			   2, 1, TPS_FIN_TOUR_MIN, TPS_FIN_TOUR_MAX);
//   option_nb_ver.Init(_("Number of players per team:"), espace, 0,
// 		     4, 1, NBR_VER_MIN, NBR_VER_MAX);
//   option_energie_ini.Init(_("Initial energy:"), espace, 0, 100, 5, 50, 200);

//   opt_sound.Init (_("Use sound?"), video.GetWidth()/2, 0);
//   opt_music.Init (_("Music?"), video.GetWidth()/2, 0);
//   opt_sound_effects.Init (_("Sound effects?"), video.GetWidth()/2, 0);

//   // ListBox
//   lboxMaps.Init ();
//   lboxTeams.Init ();
// #ifdef BUGG_CODE
//   lboxVideoMode.Init ();
// #endif
//   lboxSoundFreq.Init ();
  
//   // Load Maps' list
//   std::sort(lst_terrain.liste.begin(), lst_terrain.liste.end(), compareMaps);

//   ListeTerrain::iterator
//     terrain=lst_terrain.liste.begin(),
//     fin_terrain=lst_terrain.liste.end();
//   for (; terrain != fin_terrain; ++terrain)
//   {
//     bool choisi = terrain -> name == lst_terrain.TerrainActif().name;
//     lboxMaps.AddItem (choisi, terrain -> name, terrain -> name);
//   }

//   // Load Teams' list
//   teams_list.full_list.sort(compareTeams);

//   TeamsList::full_iterator
//     it=teams_list.full_list.begin(), 
//     end=teams_list.full_list.end();
//   lboxTeams.selection_max = game_mode.max_teams;
//   lboxTeams.selection_min = 2;
//   uint i=0;
//   for (; it != end; ++it)
//   {
//     bool choix = teams_list.IsSelected (i);
//     lboxTeams.AddItem (choix, (*it).GetName(), (*it).GetName());
//     ++i;
//   }

//   // Generate video mode list
// #ifdef BUGGY_CODE
//   Video::ConstIteratorMode
//     mode=video.GetModes().begin(),
//     mode_end=video.GetModes().end();
//   for (int index=0; mode != mode_end; ++mode, ++index)
//   {
//     lboxVideoMode.AddItem (index==video.GetCurrentMode(),
// 			   mode->get_string(),
// 			   mode->get_string());
//   }
// #endif
//   // Generate sound mode list
//   uint current_freq = jukebox.GetFrequency();
//   lboxSoundFreq.AddItem (current_freq == 11025, "11 kHz", "11025");
//   lboxSoundFreq.AddItem (current_freq == 22050, "22 kHz", "22050");
//   lboxSoundFreq.AddItem (current_freq == 44100, "44 kHz", "44100");
// }

// //-----------------------------------------------------------------------------

// void MenuOption::Reset()
// {
//   option_affichage_energie.SetValue (config.affiche_energie_ver);
//   option_affichage_nom.SetValue (config.affiche_nom_ver);
// #ifdef BUGGY_CODE
//   full_screen.SetValue (video.IsFullScreen());
// #endif
// #ifdef USE_SDL
//   use_sdl.SetValue (config.use_sdl);
//   old_use_sdl = config.use_sdl;
// #endif
//   option_temps_tour.SetValue(game_mode.duration_turn);
//   option_temps_fin_tour.SetValue(game_mode.duration_turn_end);
//   option_nb_ver.SetValue(game_mode.max_characters);
//   option_energie_ini.SetValue(game_mode.character.init_energy);

//   opt_sound.SetValue( jukebox.UseSound() );
//   opt_music.SetValue( jukebox.GetMusicConfig() );
//   opt_sound_effects.SetValue( jukebox.GetEffectsConfig() );
// }

// //-----------------------------------------------------------------------------

// void MenuOption::EnregistreOptions()
// {
//   // Save values
//   std::string map_id = lboxMaps.ReadLabel(lboxMaps.GetSelectedItem());
//   lst_terrain.ChangeTerrainNom (map_id);
//   teams_list.ChangeSelection (lboxTeams.GetSelection());
//   config.affiche_energie_ver = option_affichage_energie.GetValue();
//   config.affiche_nom_ver = option_affichage_nom.GetValue();

//   game_mode.duration_turn = option_temps_tour.GetValue() ;
//   game_mode.duration_turn_end = option_temps_fin_tour.GetValue() ;
//   game_mode.max_characters = option_nb_ver.GetValue() ;

//   game_mode.character.init_energy = option_energie_ini.GetValue() ;
// #ifdef BUGGY_CODE
//   video.ChangeMode(lboxVideoMode.GetSelectedItem(),full_screen.GetValue());
// #endif
// #ifdef USE_SDL
//   config.use_sdl = use_sdl.GetValue();
// #endif
  
//   // Sound
//   jukebox.ActiveSound( opt_sound.GetValue() );
//   jukebox.ActiveMusic( opt_music.GetValue() );
//   jukebox.ActiveEffects( opt_sound_effects.GetValue() );

//   std::string sfreq = lboxSoundFreq.ReadValue(lboxSoundFreq.GetSelectedItem());
//   long freq;
//   if (str2long(sfreq,freq)) jukebox.SetFrequency (freq);

//   // Enregistre le tout dans le XML
//   config.Sauve();
// }

// //-----------------------------------------------------------------------------

// void MenuOption::ChangeTerrain()
// {
//   std::string map_id = lboxMaps.ReadLabel(lboxMaps.GetSelectedItem());
//   uint map = lst_terrain.FindMapById(map_id);
//   map_preview = lst_terrain.liste[map].preview;
// #ifndef WIN32
//   float scale = std::min( float(CARTE_HAUT)/map_preview.get_height(), 
//                           float(CARTE_LARG)/map_preview.get_width() ) ;
// #else
//   float scale = _cpp_min( float(CARTE_HAUT)/map_preview.get_height(), 
//                           float(CARTE_LARG)/map_preview.get_width() ) ;
// #endif
//   carte_haut = (uint)(map_preview.get_height() * scale);
//   carte_larg = (uint)(map_preview.get_width() * scale);

//   map_preview.set_scale (scale, scale);
  
//   carte_x = (espace*2+MAPS_LARG)+(CARTE_LARG-carte_larg)/2;
//   carte_y = CARTE_Y+(CARTE_HAUT-carte_haut)/2;
// }

// //-----------------------------------------------------------------------------

// // Traitement d'une touche clavier relachée
// void MenuOption::Lance ()
// { 
//   bool terrain_init = false;
//   int x=0, y=0;
//   Team* derniere_equipe = teams_list.FindByIndex(0);;

//   Init();
//   Reset();

//   // Installe le pilote clavier
//   keyboard_slot = CL_Keyboard::sig_key_up().connect(this, &MenuOption::TraiteTouche);
//   mouse_slot = CL_Mouse::sig_key_up().connect(this, &MenuOption::TraiteClic);
//   slot_quit = app.clwindow -> sig_window_close().connect(this, &MenuOption::SignalWM_QUIT);

//   x = (video.GetWidth()/2);
//   y = video.GetHeight()-50;
//   valider.SetPos (x-50, y);
//   enregistrer.SetPos (x, y);
//   annuler.SetPos (x+50, y);

//   espace = (video.GetWidth()-CARTE_LARG-MAPS_LARG-TEAMS_LARG)/4;
//   //espace_h = (video.GetHeight()-CARTE_HAUT-30-100)/4;
//   uint centre_x = video.GetWidth()/2;

//   lboxMaps.SetXY (espace, 30);
//   lboxTeams.SetXY ((espace*3)+CARTE_LARG+MAPS_LARG, TEAMS_Y);
// #ifdef BUGGY_CODE
//   lboxVideoMode.SetXY ((espace*3)+CARTE_LARG+MAPS_LARG-30, VIDEO_MODE_Y);
// #endif

//   x = espace;
//   option_affichage_energie.SetXY (x, CBOX_ENERGIE_Y);
//   option_affichage_nom.SetXY (x, CBOX_ENERGIE_Y+20);
//   option_temps_tour.SetXY (x, CBOX_ENERGIE_Y+(2*20));
//   option_temps_fin_tour.SetXY (x, CBOX_ENERGIE_Y+(3*20));
//   option_nb_ver.SetXY (x, CBOX_ENERGIE_Y+(4*20));
//   option_energie_ini.SetXY (x, CBOX_ENERGIE_Y+(5*20));

//   x = centre_x-50;
//   y = FULL_SCREEN_Y;
// #ifdef USE_SDL
//   use_sdl.SetXY (x, y);
//   y += 20; 
// #endif
// #ifdef BUGGY_CODE
//   full_screen.SetXY (x, y);
// #endif
//   y += 20; opt_sound.SetXY (x, y);
//   y += 20; opt_music.SetXY (x, y);
//   y += 20; opt_sound_effects.SetXY (x, y);

//   lboxSoundFreq.SetXY (x, SOUND_FREQ_Y);

//   fin_boucle = false;
//   do
//   {
//     // Lit la position de la souris
//     x = CL_Mouse::get_x();
//     y = CL_Mouse::get_y();

//     CL_Display::clear();

//     int nv_equipe = lboxTeams.MouseIsOnWitchItem (x,y);
//     if (nv_equipe != -1)
//     {
//       derniere_equipe = teams_list.FindByIndex(nv_equipe);
//     }
      
//     // affichage des boutons et de la carte
//     fond_option.draw(CL_Rect(0, 0, video.GetWidth(), video.GetHeight()));
//     fond_maps.draw(espace-30, 5);
//     fond_maps.draw((espace*3)+CARTE_LARG+MAPS_LARG-30, 5);
//     fond_box.draw(espace-30, CBOX_ENERGIE_Y-20);
//     fond_box2.draw((espace*3)+CARTE_LARG+MAPS_LARG-100, VIDEO_MODE_Y-25);

//     valider.Draw(x,y) ;
//     annuler.Draw(x,y) ;
//     enregistrer.Draw(x,y) ;
//     lboxMaps.Display(x,y);
//     lboxTeams.Display(x,y);
// #ifdef BUGGY_CODE
//     lboxVideoMode.Display(x,y);
//     full_screen.Display(x,y);
// #endif
//     lboxSoundFreq.Display(x,y);
//     option_affichage_energie.Display(x,y);
//     option_affichage_nom.Display(x,y);
// #ifdef USE_SDL
//     use_sdl.Display(x,y);
// #endif
//     option_temps_tour.Display(x,y);
//     option_temps_fin_tour.Display(x,y);
//     option_nb_ver.Display(x,y);
//     option_energie_ini.Display(x,y);
//     opt_sound.Display(x,y);
//     opt_music.Display(x,y);
//     opt_sound_effects.Display(x,y);

//     derniere_equipe -> ecusson.draw ((espace*3)+CARTE_LARG+MAPS_LARG+
// 				     (TEAMS_LARG/2)-ECUSSON_LARG/2 ,
// 				     ECUSSON_Y);

//     if (!terrain_init)
//     {
//       terrain_init = true;
//       ChangeTerrain();
//     }

//     map_preview.draw (carte_x, carte_y);
      
//     CL_Display::flip();

//     CL_System::keep_alive (PAUSE) ;
//   } while (!fin_boucle);

//   // Désinstalle le pilote
//   CL_Keyboard::sig_key_up().disconnect(keyboard_slot);
//   CL_Mouse::sig_key_up().disconnect(mouse_slot);
// #ifdef PORT_CL07
//   CL_DisplayWindow::sig_window_close().disconnect(slot_quit);
// #endif

// #ifdef USE_SDL
//   if (old_use_sdl != config.use_sdl) {
//     Question question;
//     question.Init 
//       (_("SDL option changed:\n"
// 	 "You have to restart Wormux to apply the new settings."), 
//        true, 0);
//     question.PoseQuestion();
//   }
// #endif
// }

// //-----------------------------------------------------------------------------
