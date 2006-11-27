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
 * Wormux configuration : all variables interesting to tweak should be here.
 * A default value is affected for each variable, the value can be changed by
 * the configuration file.
 *****************************************************************************/

#include "config.h"

#define USE_AUTOPACKAGE

#include <sstream>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#ifdef WIN32
#  include <direct.h>
#endif
#include "game_mode.h"
#include "errno.h"
#include "../graphic/video.h"
#include "../include/action.h"
#include "../include/app.h"
#include "../interface/keyboard.h"
#include "../include/constant.h"
#include "../map/maps_list.h"
#include "../sound/jukebox.h"
#include "../team/teams_list.h"
#include "../tool/file_tools.h"
#include "../tool/string_tools.h"
#include "../tool/i18n.h"
#include "../weapon/weapons_list.h"
#ifdef USE_AUTOPACKAGE
#  include "../include/binreloc.h"
#endif

const std::string NOMFICH="config.xml";
Config * Config::singleton = NULL;

Config * Config::GetInstance() {
  if (singleton == NULL) {
    singleton = new Config();
  }
  return singleton;
}

Config::Config()
{
#ifdef USE_AUTOPACKAGE
  BrInitError error;
  std::string filename;

  if (br_init (&error) == 0 && error != BR_INIT_ERROR_DISABLED) {
	  std::cout << "Warning: BinReloc failed to initialize (error code "
		    << error << ")" << std::endl;
	  std::cout << "Will fallback to hardcoded default path." << std::endl;
  }
#endif

  // Default values
  exterieur_monde_vide = true;
  m_game_mode = "classic";
  display_energy_character = true;
  display_name_character = true;
  display_wind_particles = true;
  default_mouse_cursor = false;
  scroll_on_border = true;
  transparency = ALPHA;

  // video
  tmp.video.width = 800;
  tmp.video.height = 600;
  tmp.video.fullscreen = false;

  tmp.sound.music = true;
  tmp.sound.effects = true;
  tmp.sound.frequency = 44100;

  Constants::GetInstance();

  // directories
#ifdef USE_AUTOPACKAGE
  data_dir = GetEnv(Constants::ENV_DATADIR, br_find_data_dir(INSTALL_DATADIR));
  locale_dir = GetEnv(Constants::ENV_LOCALEDIR, br_find_locale_dir(INSTALL_LOCALEDIR));
  filename = data_dir + PATH_SEPARATOR + "font" + PATH_SEPARATOR + "DejaVuSans.ttf";
  ttf_filename = GetEnv(Constants::ENV_FONT_PATH, br_find_locale_dir(filename.c_str()));
#else
  data_dir = GetEnv(Constants::ENV_DATADIR, INSTALL_DATADIR);
  locale_dir = GetEnv(Constants::ENV_LOCALEDIR, INSTALL_LOCALEDIR);
  ttf_filename = GetEnv(Constants::ENV_FONT_PATH, FONT_FILE);
#endif

#ifndef WIN32
  personal_dir = GetHome()+"/.wormux/";
#else
  personal_dir = GetHome()+"\\Wormux\\";
#endif
}

bool Config::Load()
{
  bool result = ChargeVraiment();
  std::string dir;
  dir = TranslateDirectory(locale_dir);
  I18N_SetDir (dir + PATH_SEPARATOR);

  dir = TranslateDirectory(data_dir);
  resource_manager.AddDataPath(dir + PATH_SEPARATOR);
  return result;
}

bool Config::ChargeVraiment()
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
    std::cout << "o "
        << _("Error while loading configuration file: %s") << std::endl
        << e.what() << std::endl;
    return false;
  }

  return true;
}

// Read personal config file
bool Config::ChargeXml(xmlpp::Element *xml)
{
  std::cout << "o " << _("Reading personal config file") << std::endl;

  xmlpp::Element *elem;

  //=== Map ===
  LitDocXml::LitString  (xml, "map", tmp.map_name);

  //=== Teams ===
  elem = LitDocXml::AccesBalise (xml, "teams");
  int i = 0;

  xmlpp::Element *team = LitDocXml::AccesBalise (elem, "team_" + ulong2str(i));

  while (team != NULL)
  {
    ConfigTeam one_team;
    LitDocXml::LitString  (team, "id", one_team.id);
    LitDocXml::LitString  (team, "player_name", one_team.player_name);

    int tmp_nb_characters;
    LitDocXml::LitInt (team, "nb_characters", tmp_nb_characters);
    one_team.nb_characters = (uint)tmp_nb_characters;

    tmp.teams.push_back(one_team);

    // get next team
    i++;
    team = LitDocXml::AccesBalise (elem, "team_"+ulong2str(i));
  }

  //=== Video ===
  elem = LitDocXml::AccesBalise (xml, "video");
  if (elem != NULL)
  {
    uint max_fps;
    if (LitDocXml::LitUint (elem, "max_fps", max_fps))
      AppWormux::GetInstance()->video.SetMaxFps(max_fps);

    LitDocXml::LitBool (elem, "display_wind_particles", display_wind_particles);
    LitDocXml::LitBool (elem, "display_energy_character", display_energy_character);
    LitDocXml::LitBool (elem, "display_name_character", display_name_character);
    LitDocXml::LitBool (elem, "default_mouse_cursor", default_mouse_cursor);
    LitDocXml::LitBool (elem, "scroll_on_border", scroll_on_border);
    LitDocXml::LitInt (elem, "width", tmp.video.width);
    LitDocXml::LitInt (elem, "height", tmp.video.height);
    LitDocXml::LitBool (elem, "full_screen", tmp.video.fullscreen);
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
  return true;
}

void Config::SetKeyboardConfig()
{
  Clavier * clavier = Clavier::GetInstance();

  clavier->SetKeyAction(SDLK_LEFT,      Action::ACTION_MOVE_LEFT);
  clavier->SetKeyAction(SDLK_RIGHT,     Action::ACTION_MOVE_RIGHT);
  clavier->SetKeyAction(SDLK_UP,        Action::ACTION_UP);
  clavier->SetKeyAction(SDLK_DOWN,      Action::ACTION_DOWN);
  clavier->SetKeyAction(SDLK_RETURN,    Action::ACTION_JUMP);
  clavier->SetKeyAction(SDLK_BACKSPACE, Action::ACTION_HIGH_JUMP);
  clavier->SetKeyAction(SDLK_b,         Action::ACTION_BACK_JUMP);
  clavier->SetKeyAction(SDLK_SPACE,     Action::ACTION_SHOOT);
  clavier->SetKeyAction(SDLK_TAB,       Action::ACTION_NEXT_CHARACTER);
  clavier->SetKeyAction(SDLK_ESCAPE,    Action::ACTION_QUIT);
  clavier->SetKeyAction(SDLK_p,         Action::ACTION_PAUSE);
  clavier->SetKeyAction(SDLK_F10,       Action::ACTION_FULLSCREEN);
  clavier->SetKeyAction(SDLK_F9,        Action::ACTION_TOGGLE_INTERFACE);
  clavier->SetKeyAction(SDLK_F1,        Action::ACTION_WEAPONS1);
  clavier->SetKeyAction(SDLK_F2,        Action::ACTION_WEAPONS2);
  clavier->SetKeyAction(SDLK_F3,        Action::ACTION_WEAPONS3);
  clavier->SetKeyAction(SDLK_F4,        Action::ACTION_WEAPONS4);
  clavier->SetKeyAction(SDLK_F5,        Action::ACTION_WEAPONS5);
  clavier->SetKeyAction(SDLK_F6,        Action::ACTION_WEAPONS6);
  clavier->SetKeyAction(SDLK_F7,        Action::ACTION_WEAPONS7);
  clavier->SetKeyAction(SDLK_F8,        Action::ACTION_WEAPONS8);
  clavier->SetKeyAction(SDLK_c,         Action::ACTION_CENTER);
  clavier->SetKeyAction(SDLK_1,         Action::ACTION_WEAPON_1);
  clavier->SetKeyAction(SDLK_2,         Action::ACTION_WEAPON_2);
  clavier->SetKeyAction(SDLK_3,         Action::ACTION_WEAPON_3);
  clavier->SetKeyAction(SDLK_4,         Action::ACTION_WEAPON_4);
  clavier->SetKeyAction(SDLK_5,         Action::ACTION_WEAPON_5);
  clavier->SetKeyAction(SDLK_6,         Action::ACTION_WEAPON_6);
  clavier->SetKeyAction(SDLK_7,         Action::ACTION_WEAPON_7);
  clavier->SetKeyAction(SDLK_8,         Action::ACTION_WEAPON_8);
  clavier->SetKeyAction(SDLK_9,         Action::ACTION_WEAPON_9);
  clavier->SetKeyAction(SDLK_PAGEUP,    Action::ACTION_WEAPON_MORE);
  clavier->SetKeyAction(SDLK_PAGEDOWN,  Action::ACTION_WEAPON_LESS);
  clavier->SetKeyAction(SDLK_s,         Action::ACTION_CHAT);

}

void Config::Apply()
{
  SetKeyboardConfig();

  // Charge le mode jeu
  weapons_list.Init();

  GameMode::GetInstance()->Load(m_game_mode);

  // Son
  jukebox.ActiveMusic (tmp.sound.music);
  jukebox.ActiveEffects (tmp.sound.effects);
  jukebox.SetFrequency (tmp.sound.frequency);

  // load the teams
  teams_list.LoadList();
  if (m_xml_charge)
    teams_list.InitList (tmp.teams);

  // Load maps
  if (m_xml_charge && !tmp.map_name.empty())
    MapsList::GetInstance()->SelectMapByName (tmp.map_name);
  else
    MapsList::GetInstance()->SelectMapByIndex (0);
}

bool Config::Save()
{
  std::string rep = personal_dir;

  // Create the directory if it doesn't exist
#ifndef WIN32
   if (mkdir (personal_dir.c_str(), 0750) != 0 && errno != EEXIST)
#else
  if (_mkdir (personal_dir.c_str()) != 0 && errno != EEXIST)
#endif
  {
    std::cerr << "o "
      << Format(_("Error while creating directory \"%s\": unable to store configuration file."),
          rep.c_str())
      << std::endl;
    return false;
  }

  if (!SauveXml())
  {
    return false;
  }
  return true;
}

bool Config::SauveXml()
{
  EcritDocXml doc;

  doc.Cree (m_nomfich, "config", "1.0", "utf-8");
  xmlpp::Element *racine = doc.racine();
  doc.EcritBalise (racine, "version", Constants::VERSION);

  //=== Map ===
  doc.EcritBalise (racine, "map", MapsList::GetInstance()->ActiveMap().ReadName());

  //=== Teams ===
  xmlpp::Element *balise_equipes = racine -> add_child("teams");

  TeamsList::iterator
    it=teams_list.playing_list.begin(),
    fin=teams_list.playing_list.end();
  for (int i=0; it != fin; ++it, i++)
  {
    xmlpp::Element *une_equipe = balise_equipes -> add_child("team_"+ulong2str(i));
    doc.EcritBalise (une_equipe, "id", (**it).GetId());
    doc.EcritBalise (une_equipe, "player_name", (**it).GetPlayerName());
    doc.EcritBalise (une_equipe, "nb_characters", ulong2str((**it).GetNbCharacters()));
  }

  //=== Video ===
  AppWormux * app = AppWormux::GetInstance();

  xmlpp::Element *noeud_video = racine -> add_child("video");
  doc.EcritBalise (noeud_video, "display_wind_particles", ulong2str(display_wind_particles));
  doc.EcritBalise (noeud_video, "display_energy_character", ulong2str(display_energy_character));
  doc.EcritBalise (noeud_video, "display_name_character", ulong2str(display_name_character));
  doc.EcritBalise (noeud_video, "default_mouse_cursor", ulong2str(default_mouse_cursor));
  doc.EcritBalise (noeud_video, "scroll_on_border", ulong2str(scroll_on_border));
  doc.EcritBalise (noeud_video, "width", ulong2str(app->video.window.GetWidth()));
  doc.EcritBalise (noeud_video, "height", ulong2str(app->video.window.GetHeight()));
  doc.EcritBalise (noeud_video, "full_screen",
		   ulong2str(static_cast<uint>(app->video.IsFullScreen())) );
  doc.EcritBalise (noeud_video, "max_fps",
          long2str(static_cast<int>(app->video.GetMaxFps())));

  if ( transparency == ALPHA )
    doc.EcritBalise (noeud_video, "transparency", "alpha");
  else if ( transparency == COLORKEY )
    doc.EcritBalise (noeud_video, "transparency", "colorkey");

  //=== Son ===
  xmlpp::Element *noeud_son = racine -> add_child("sound");
  doc.EcritBalise (noeud_son, "music", ulong2str(jukebox.UseMusic()));
  doc.EcritBalise (noeud_son, "effects",
		   ulong2str(jukebox.UseEffects()));
  doc.EcritBalise (noeud_son, "frequency",
		   ulong2str(jukebox.GetFrequency()));

  //=== Mode de jeu ===
  doc.EcritBalise (racine, "game_mode", m_game_mode);
  return doc.Sauve();
}

/*
 * Return the value of the environment variable 'name' or
 * 'default' if not set
 */
std::string Config::GetEnv(const std::string & name, const std::string &default_value)
{
  const char *env = std::getenv(name.c_str());
  if (env != NULL) {
    return env;
  } else {
    return default_value;
  }
}

std::string Config::GetDataDir() const
{
  return data_dir;
}

std::string Config::GetLocaleDir() const
{
  return locale_dir;
}

std::string Config::GetPersonalDir() const
{
  return personal_dir;
}

bool Config::GetExterieurMondeVide() const
{
  return exterieur_monde_vide;
}

bool Config::GetDisplayEnergyCharacter() const
{
  return display_energy_character;
}

bool Config::GetDisplayNameCharacter() const
{
  return display_name_character;
}

bool Config::GetDisplayWindParticles() const
{
  return display_wind_particles;
}

bool Config::GetDefaultMouseCursor() const
{
  return default_mouse_cursor;
}

bool Config::GetScrollOnBorder() const
{
  return scroll_on_border;
}

std::string Config::GetTtfFilename() const
{
  return ttf_filename;
}

void Config::SetDisplayEnergyCharacter(bool dec)
{
  display_energy_character = dec;
}

void Config::SetDisplayNameCharacter(bool dnc)
{
  display_name_character = dnc;
}

void Config::SetDisplayWindParticles(bool dwp)
{
  display_wind_particles = dwp;
}

void Config::SetDefaultMouseCursor(bool dmc)
{
  default_mouse_cursor = dmc;
}

void Config::SetScrollOnBorder(bool sob)
{
  scroll_on_border = sob;
}

void Config::SetExterieurMondeVide(bool emv)
{
  exterieur_monde_vide = emv;
}

int Config::GetTransparency() const
{
  return transparency;
}
