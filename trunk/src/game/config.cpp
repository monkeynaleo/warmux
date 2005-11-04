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
 * Configuration de Wormux : toutes les variables qui sont intéressantes à
 * modifier se retrouvent ici. Les variables ont une valeur par défaut qui
 * peut être modifiée avec le fichier de configuration.
 *****************************************************************************/

#include "config.h"
//-----------------------------------------------------------------------------
#include "../team/teams_list.h"
#include "../graphic/video.h"
#include "../sound/jukebox.h"
#include "../team/skin.h"
#include "../map/maps_list.h"
#include "../weapon/weapons_list.h"
#include "../include/action.h"
#include "../interface/keyboard.h"
#ifdef CL
#include "../graphic/graphism.h"
#include <ClanLib/display.h>
#else
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include "../include/constant.h"
#include "game_mode.h"
#include "../tool/file_tools.h"
#include "../tool/string_tools.h"
#include "../tool/i18n.h"
#endif
//-----------------------------------------------------------------------------
namespace Wormux 
{
Config config;

#ifndef WIN32
const std::string REP_CONFIG="~/.wormux/";
#else
const std::string REP_CONFIG="";
#endif

const std::string NOMFICH="config.xml";
//-----------------------------------------------------------------------------

Config::Config()
{
  // Default values
  exterieur_monde_vide = true;
  m_game_mode = "classic";
  affiche_energie_ver = true;
  affiche_nom_ver = true;
  transparency = ALPHA;
   
  // directories
#ifndef WIN32
  data_dir = DEFAULT_DATADIR;
  locale_dir = DEFAULT_LOCALEDIR;
#else
  data_dir = "data\\";
  locale_dir = "locale\\";
#endif


  // video
#ifdef USE_SDL
  use_sdl = false;
#endif
  tmp.video.width = 800;
  tmp.video.height = 600;
  tmp.video.fullscreen = false;

  tmp.sound.music = true;
  tmp.sound.effects = true;
  tmp.sound.frequency = 44100;

#ifndef WIN32
  personal_dir = RepertoireHome()+"/.wormux/";
#else
  personal_dir ="";
#endif
}

//-----------------------------------------------------------------------------

bool Config::Charge()
{
  m_xml_charge=false;
  try
  {
    // Charge la configuration XML
    LitDocXml doc;
    m_nomfich = personal_dir+NOMFICH;
    if (!doc.Charge (m_nomfich)) return false;
    if (!ChargeXml (doc.racine())) return false;
    m_xml_charge = true;
  }
  catch (const xmlpp::exception &e)
  {
    std::cout << "o Erreur lors du chargement :" << std::endl
	      << e.what() << std::endl;
    return false;
  } 

  return true;
}

//-----------------------------------------------------------------------------

// Lit les données sur une équipe
bool Config::ChargeXml(xmlpp::Element *xml)
{
  xmlpp::Element *elem;

  //=== Directories ===
  if (LitDocXml::LitString  (xml, "data_dir", data_dir)) {
    data_dir = TraduitRepertoire(data_dir);
#ifndef CL
    resource_manager.AddDataPath(data_dir);
#endif
  }
  if (LitDocXml::LitString  (xml, "locale_dir", locale_dir)) {
    locale_dir = TraduitRepertoire(locale_dir);
  }
 
  //=== Map ===
  LitDocXml::LitString  (xml, "map", tmp.map_name);

  //=== Equipes ===
  elem = LitDocXml::AccesBalise (xml, "teams");
  if (elem != NULL)
  {
    LitDocXml::LitListeString (elem, "team", tmp.teams);
  }

  //=== Vidéo ===
  elem = LitDocXml::AccesBalise (xml, "video");
  if (elem != NULL)
  {
#ifdef BUGGY_CODE
    uint max_fps;
    if (LitDocXml::LitUint (elem, "max_fps", max_fps)) 
      video.SetMaxFps(max_fps);
#endif
#ifdef USE_SDL
    LitDocXml::LitBool (elem, "use_sdl", use_sdl);
#endif
    LitDocXml::LitInt (elem, "width", tmp.video.width);
    LitDocXml::LitInt (elem, "height", tmp.video.height);
    LitDocXml::LitBool (elem, "full_screen", tmp.video.fullscreen);

    std::string transparency_str;
    if ( LitDocXml::LitString (elem, "transparency", transparency_str))
       {
	  if ( transparency_str == "alpha" )
	    transparency = ALPHA;
	  else if ( transparency_str == "colorkey" )
	    transparency = COLORKEY;
	  else
	    {
	       std::cerr << "o Unknow transparency \"" << transparency_str  
		 << "\" in config.xml [IGNORED]." << std::endl;
	    }	  
       }
     
	 
  }

  //=== Son ===
  elem = LitDocXml::AccesBalise (xml, "sound");
  if (elem != NULL)
  {
    LitDocXml::LitBool (elem, "music", tmp.sound.music);
    LitDocXml::LitBool (elem, "effects", tmp.sound.effects);
    LitDocXml::LitUint (elem, "frequency", tmp.sound.frequency);
  }

  //=== Mode de jeu ===
  LitDocXml::LitString (xml, "game_mode", m_game_mode);

#ifdef CL
	clavier.SetKeyAction(CL_KEY_LEFT,		ACTION_MOVE_LEFT);		
	clavier.SetKeyAction(CL_KEY_RIGHT,	ACTION_MOVE_RIGHT);
	clavier.SetKeyAction(CL_KEY_UP,			ACTION_UP);
	clavier.SetKeyAction(CL_KEY_DOWN,	ACTION_DOWN);
	clavier.SetKeyAction(CL_KEY_ENTER,	ACTION_JUMP);
	clavier.SetKeyAction(CL_KEY_BACKSPACE, ACTION_SUPER_JUMP);
	clavier.SetKeyAction(CL_KEY_SPACE, ACTION_SHOOT);
	clavier.SetKeyAction(CL_KEY_TAB, ACTION_CHANGE_CHARACTER);
	clavier.SetKeyAction(CL_KEY_ESCAPE, ACTION_QUIT);
	clavier.SetKeyAction(CL_KEY_P, ACTION_PAUSE);
	clavier.SetKeyAction(CL_KEY_F10, ACTION_FULLSCREEN);
	clavier.SetKeyAction(CL_KEY_F9, ACTION_TOGGLE_INTERFACE);
	clavier.SetKeyAction(CL_KEY_F1, ACTION_WEAPONS1);
	clavier.SetKeyAction(CL_KEY_F2, ACTION_WEAPONS2);
	clavier.SetKeyAction(CL_KEY_F3, ACTION_WEAPONS3);
	clavier.SetKeyAction(CL_KEY_F4, ACTION_WEAPONS4);
	clavier.SetKeyAction(CL_KEY_F5, ACTION_WEAPONS5);
	clavier.SetKeyAction(CL_KEY_F6, ACTION_WEAPONS6);
	clavier.SetKeyAction(CL_KEY_F7, ACTION_WEAPONS7);
	clavier.SetKeyAction(CL_KEY_F8, ACTION_WEAPONS8);
	clavier.SetKeyAction(CL_KEY_C, ACTION_CENTER);
#else
	clavier.SetKeyAction(SDLK_LEFT,		ACTION_MOVE_LEFT);		
	clavier.SetKeyAction(SDLK_RIGHT,	ACTION_MOVE_RIGHT);
	clavier.SetKeyAction(SDLK_UP,			ACTION_UP);
	clavier.SetKeyAction(SDLK_DOWN,	ACTION_DOWN);
	clavier.SetKeyAction(SDLK_RETURN,	ACTION_JUMP);
	clavier.SetKeyAction(SDLK_BACKSPACE, ACTION_SUPER_JUMP);
	clavier.SetKeyAction(SDLK_SPACE, ACTION_SHOOT);
	clavier.SetKeyAction(SDLK_TAB, ACTION_CHANGE_CHARACTER);
	clavier.SetKeyAction(SDLK_ESCAPE, ACTION_QUIT);
	clavier.SetKeyAction(SDLK_p, ACTION_PAUSE);
	clavier.SetKeyAction(SDLK_F10, ACTION_FULLSCREEN);
	clavier.SetKeyAction(SDLK_F9, ACTION_TOGGLE_INTERFACE);
	clavier.SetKeyAction(SDLK_F1, ACTION_WEAPONS1);
	clavier.SetKeyAction(SDLK_F2, ACTION_WEAPONS2);
	clavier.SetKeyAction(SDLK_F3, ACTION_WEAPONS3);
	clavier.SetKeyAction(SDLK_F4, ACTION_WEAPONS4);
	clavier.SetKeyAction(SDLK_F5, ACTION_WEAPONS5);
	clavier.SetKeyAction(SDLK_F6, ACTION_WEAPONS6);
	clavier.SetKeyAction(SDLK_F7, ACTION_WEAPONS7);
	clavier.SetKeyAction(SDLK_F8, ACTION_WEAPONS8);
	clavier.SetKeyAction(SDLK_c, ACTION_CENTER);

#endif
 	return true;
}

//-----------------------------------------------------------------------------

void Config::Applique()
{
  I18N_SetDir (locale_dir);

  // Charge le mode jeu
  weapons_list.Init();
  
  game_mode.Load(m_game_mode);

  // Son
  jukebox.ActiveMusic (tmp.sound.music);
  jukebox.ActiveEffects (tmp.sound.effects);
  jukebox.SetFrequency (tmp.sound.frequency);

  // Charge les équipes 
  InitSkins();
  teams_list.LoadList();
  if (m_xml_charge) teams_list.InitList (tmp.teams);

   
  // Charge les terrains
  lst_terrain.Init();
  if (m_xml_charge && !tmp.map_name.empty()) 
    lst_terrain.ChangeTerrainNom (tmp.map_name);
  else
    lst_terrain.ChangeTerrain (0);
}

//-----------------------------------------------------------------------------

bool Config::Sauve()
{
  // Le répertoire de config n'existe pas ? le créer
  std::string rep = TraduitRepertoire(REP_CONFIG);
#ifndef WIN32
  if (!FichierExiste (rep))
  {
    if (mkdir (rep.c_str(), 0750) != 0)
    {
      std::cout << "o Erreur lors de la création répertoire " << rep 
		<< ", impossible de créer le fichier de configuration." 
		<< std::endl;
      return false;
    }
  }
#endif

  if (!SauveXml())
  {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------

bool Config::SauveXml()
{
  EcritDocXml doc;

  doc.Cree (m_nomfich, "config", "1.0", "iso-8859-1");
  xmlpp::Element *racine = doc.racine();

  //=== Directories ===
  doc.EcritBalise (racine, "data_dir", data_dir);
  doc.EcritBalise (racine, "locale_dir", locale_dir);

  //=== Terrain ===
  doc.EcritBalise (racine, "map", lst_terrain.TerrainActif().name);

  //=== Equipes ===
  xmlpp::Element *balise_equipes = racine -> add_child("teams");
  TeamsList::iterator
    it=teams_list.list.begin(),
    fin=teams_list.list.end();
  for (; it != fin; ++it)
  {
    doc.EcritBalise (balise_equipes, "team", (**it).GetId());
  }

  //=== Video ===
  xmlpp::Element *noeud_video = racine -> add_child("video");
  doc.EcritBalise (noeud_video, "width", ulong2str(video.GetWidth()));
  doc.EcritBalise (noeud_video, "height", ulong2str(video.GetHeight()));
  doc.EcritBalise (noeud_video, "full_screen", 
		   ulong2str(static_cast<uint>(video.IsFullScreen())) );
  //=== Son ===
  xmlpp::Element *noeud_son = racine -> add_child("sound");
#ifdef CL
  doc.EcritBalise (noeud_son, "music", ulong2str(jukebox.GetMusicConfig()));
  doc.EcritBalise (noeud_son, "effects", 
		   ulong2str(jukebox.GetEffectsConfig()));
  doc.EcritBalise (noeud_son, "frequency",
		   ulong2str(jukebox.GetFrequency()));
#else
  doc.EcritBalise (noeud_son, "music", ulong2str(jukebox.UseMusic()));
  doc.EcritBalise (noeud_son, "effects", 
		   ulong2str(jukebox.UseEffects()));
  doc.EcritBalise (noeud_son, "frequency",
		   ulong2str(jukebox.GetFrequency()));
#endif
   
  //=== Mode de jeu ===
  doc.EcritBalise (racine, "game_mode", m_game_mode);
  return doc.Sauve();
}

//-----------------------------------------------------------------------------

std::string Config::GetWormuxPersonalDir() const { return personal_dir; }

//-----------------------------------------------------------------------------
}
