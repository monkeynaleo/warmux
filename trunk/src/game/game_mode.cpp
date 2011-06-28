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
 * Warmux configuration : all main configuration variables have there default
 * value here. They should all be modifiable using the xml config file
 *****************************************************************************/

#include <iostream>
#include <cstdio>
#include <WARMUX_file_tools.h>
#include "game/config.h"
#include "game/game.h"
#include "game/game_mode.h"
#include "object/medkit.h"
#include "object/bonus_box.h"
#include "weapon/weapons_list.h"

static std::string txt;

GameMode::GameMode():
  doc_objects(NULL),
  weapons_xml(NULL)
{
  m_current = "classic";

  main_settings.push_back(new StringConfigElement("rules", &rules, "none"));
  main_settings.push_back(new BoolConfigElement("auto_change_character", &auto_change_character, true));
  main_settings.push_back(new StringConfigElement("allow_character_selection", &txt, "always"));
  main_settings.push_back(new UintConfigElement("duration_turn", &duration_turn, 60));
  main_settings.push_back(new UintConfigElement("duration_move_player", &duration_move_player, 3));
  main_settings.push_back(new UintConfigElement("duration_exchange_player", &duration_exchange_player, 2));
  main_settings.push_back(new UintConfigElement("duration_before_death_mode", &duration_before_death_mode, 20 * 60));
  main_settings.push_back(new UintConfigElement("damage_per_turn_during_death_mode", &damage_per_turn_during_death_mode, 5));
  main_settings.push_back(new UintConfigElement("max_teams", &max_teams, 8));
  main_settings.push_back(new UintConfigElement("nb_characters", &nb_characters, 6));
  main_settings.push_back(new DoubleConfigElement("gravity", &gravity, 9.81));
  main_settings.push_back(new IntConfigElement("safe_fall", &safe_fall, 10));
  main_settings.push_back(new UintConfigElement("damage_per_fall_unit", &damage_per_fall_unit, 7));

  char_settings.push_back(new UintConfigElement("mass", &character.mass, 100));
  char_settings.push_back(new DoubleConfigElement("air_resist_factor", &character.air_resist_factor, 1.0));
  char_settings.push_back(new UintConfigElement("walking_pause", &character.walking_pause, 50));
  main_settings.LinkList(&char_settings, "character");

  energy.push_back(new UintConfigElement("initial", &character.init_energy, 100, 1, 200, true));
  energy.push_back(new UintConfigElement("maximum", &character.max_energy, 100, 1, 200, true));
  char_settings.LinkList(&energy, "energy");

  jump.push_back(new IntConfigElement("strength", &character.jump_strength, 8, true));
  jump.push_back(new AngleConfigElement("angle", &character.jump_angle, -60, true));
  char_settings.LinkList(&jump, "jump");

  super_jump.push_back(new IntConfigElement("strength", &character.super_jump_strength, 11, true));
  super_jump.push_back(new AngleConfigElement("angle", &character.super_jump_angle, -80, true));
  char_settings.LinkList(&super_jump, "super_jump");

  back_jump.push_back(new IntConfigElement("strength", &character.back_jump_strength, 9, true));
  back_jump.push_back(new AngleConfigElement("angle", &character.back_jump_angle, -100, true));
  char_settings.LinkList(&back_jump, "back_jump");

  barrel.LinkList(&barrel_explosion_cfg, "explosion");
  main_settings.LinkList(&barrel, "barrel");

  bonus_box.LinkList(&bonus_box_explosion_cfg, "explosion");
  main_settings.LinkList(&bonus_box, "bonus_box");

  main_settings.LinkList(Medkit::GetConfigList(), "medkit");
}

void GameMode::LoadDefaultValues()
{
  rules = "none";
  nb_characters = 6;
  max_teams = 8;
  duration_turn = 60;
  duration_move_player = 3;
  duration_exchange_player = 2;
  duration_before_death_mode = 20 * 60;
  damage_per_turn_during_death_mode = 5;
  gravity = 9.81;
  safe_fall = 10;
  damage_per_fall_unit = 7;

  character.init_energy = 100;
  character.max_energy = 100;
  character.mass = 100;
  character.air_resist_factor = 1.0;
  character.jump_strength = 8;
  character.jump_angle = -60;
  character.super_jump_strength = 11;
  character.super_jump_angle = -80;
  character.back_jump_strength = 9;
  character.back_jump_angle = -100;
  character.walking_pause = 50;

  auto_change_character = true;

  allow_character_selection = BEFORE_FIRST_ACTION;

  if (doc_objects)
    delete doc_objects;

  doc_objects = new XmlReader();
}

GameMode::~GameMode()
{
  delete doc_objects;
}

// Load data options from the selected game_mode
bool GameMode::LoadXml()
{
  const xmlNode *root = doc.GetRoot();
  // Load all elements
  main_settings.LoadXml(root);

  if (txt == "always")
    allow_character_selection = ALWAYS;
  else if (txt == "never")
    allow_character_selection = NEVER;
  else if (txt == "before_first_action")
    allow_character_selection = BEFORE_FIRST_ACTION;
  else
    fprintf(stderr, "%s is not a valid option for \"allow_character_selection\"\n", txt.c_str());

  //=== Weapons ===
  weapons_xml = XmlReader::GetMarker(root, "weapons");

  return true;
}

bool GameMode::Load(void)
{
  Config * config = Config::GetInstance();
  m_current = config->GetGameMode();

  LoadDefaultValues();

  // Game mode objects configuration file
  if (!doc_objects->Load(GetObjectsFilename()))
    return false;

  if (!doc.Load(GetFilename()))
    return false;
  if (!LoadXml())
    return false;

  return true;
}

// Load the game mode from strings (probably from network)
bool GameMode::LoadFromString(const std::string& game_mode_name,
                              const std::string& game_mode_contents,
                              const std::string& game_mode_objects_contents)
{
  m_current = game_mode_name;
  MSG_DEBUG("game_mode", "Loading %s from network: ", m_current.c_str());

  if (!doc_objects->LoadFromString(game_mode_objects_contents))
    return false;

  if (!doc.LoadFromString(game_mode_contents))
    return false;
  if (!LoadXml())
    return false;

  MSG_DEBUG("game_mode", "OK\n");
  return true;
}

bool GameMode::ExportFileToString(const std::string& filename, std::string& contents) const
{
  contents = "";

  XmlReader doc;
  if (!doc.Load(filename))
    return false;

  contents = doc.ExportToString();

  return true;
}

bool GameMode::ExportToString(std::string& mode,
                              std::string& mode_objects) const
{
#if 0
  bool r;

  r = ExportFileToString(GetFilename(), mode);
  if (r) {
    r = ExportFileToString(GetObjectsFilename(), mode_objects);
  }

  return r;
#else
  mode_objects = doc_objects->ExportToString();
  mode = doc.ExportToString();
  return !mode_objects.empty() && mode.empty();
#endif
}

bool GameMode::AllowCharacterSelection() const
{
  switch (allow_character_selection) {
  case GameMode::ALWAYS: break;

  case GameMode::BEFORE_FIRST_ACTION:
    return (Game::GetInstance()->ReadState() == Game::PLAYING) &&
            !Game::GetInstance()->IsCharacterAlreadyChosen();

  case GameMode::NEVER: return false;
  }

  return true;
}

std::string GameMode::GetFilename() const
{
  Config * config = Config::GetInstance();
  std::string filename = std::string("game_mode" PATH_SEPARATOR)
                       + m_current + std::string(".xml");

  std::string fullname = config->GetPersonalDataDir() + filename;

  if (!DoesFileExist(fullname))
    fullname = config->GetDataDir() + filename;

  if (!DoesFileExist(fullname)) {
    Error(Format("Can not find file %s", fullname.c_str()));
  }

  return fullname;
}

bool GameMode::ExportToFile(const std::string& game_mode_name, WeaponsList& wlist)
{
  Config * config = Config::GetInstance();
  std::string filename = std::string("game_mode" PATH_SEPARATOR)
                       + game_mode_name + std::string(".xml");

  std::string fullname = config->GetPersonalDataDir() + filename;
  XmlWriter out;
  if (!out.Create(fullname, "game_mode", "1.0", "utf-8"))
    return false;

  xmlNode *node = out.GetRoot();
  main_settings.SaveXml(out, node);

  node = XmlWriter::AddNode(node, "weapons");
  wlist.Save(out, node);

  return true;
}

std::string GameMode::GetDefaultObjectsFilename() const
{
  std::string filename("game_mode" PATH_SEPARATOR "default_objects.xml");

  return filename;
}

std::string GameMode::GetObjectsFilename() const
{
  Config * config = Config::GetInstance();
  std::string filename = std::string("game_mode" PATH_SEPARATOR)
                       + m_current + std::string("_objects.xml");

  std::string fullname = config->GetPersonalDataDir() + filename;

  if (!DoesFileExist(fullname))
    fullname = config->GetDataDir() + filename;

  if (!DoesFileExist(fullname)) {
    std::cerr << "Game mode: File " << fullname
      << " does not exist, use the default one instead." << std::endl;
  }

  fullname = config->GetDataDir() + GetDefaultObjectsFilename();
  if (!DoesFileExist(fullname)) {
    Error(Format("Can not find file %s", fullname.c_str()));
  }

  return fullname;
}

// Static method
std::vector<std::pair<std::string, std::string> > GameMode::ListGameModes()
{
  std::vector<std::pair<std::string, std::string> > game_modes;
  game_modes.push_back(std::pair<std::string, std::string>("classic", _("Classic")));
  game_modes.push_back(std::pair<std::string, std::string>("unlimited", _("Unlimited")));
  game_modes.push_back(std::pair<std::string, std::string>("blitz", _("Blitz")));
#ifdef DEBUG
  game_modes.push_back(std::pair<std::string, std::string>("skin_viewer", "Skin Viewer"));
#endif

  std::string personal_dir = Config::GetInstance()->GetPersonalDataDir() +
    std::string("game_mode" PATH_SEPARATOR);

  FolderSearch *f = OpenFolder(personal_dir);
  if (f) {
    bool is_file = true;
    const char *name;
    while ((name = FolderSearchNext(f, is_file)) != NULL) {

      // Only check files
      if (is_file) {
        std::string filename(name);

        if (filename.size() >= 5
            && filename.compare(filename.size()-4, 4, ".xml") == 0
            && (filename.size() < 12
                || filename.compare(filename.size()-12, 12, "_objects.xml") != 0)) {

          std::string game_mode_name = filename.substr(0, filename.size()-4);
          game_modes.push_back(std::pair<std::string, std::string>(game_mode_name, game_mode_name));
        }
      }

      // Prepare again for searching files
      is_file = true;
    }
    CloseFolder(f);
  }

  return game_modes;
}
