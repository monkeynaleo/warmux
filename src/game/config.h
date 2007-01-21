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
 * Configuration of Wormux : store game config of every tunable variable of Wormux.
 * Vars have a default value and can be change with the file configuration.
 * This object aims to become the only place where the XML is read and/or
 * written. It is handled as a singleton pattern whereas one doesn't really
 * care when the XML config file is loaded, but when it was actually read, this
 * is useless to load it again.
 * On the other hand, this objet is not designed to handle any other
 * information whereas it is nothing but a shell around the XML config file
 * created to be able to handle config data easily.
 *****************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H
//-----------------------------------------------------------------------------
#include <list>
#include <string>
#include "../include/base.h"
#include "../team/team_config.h"
#include "../tool/xml_document.h"
//-----------------------------------------------------------------------------
#if defined(WIN32) || defined(__MINGW32__)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

#ifdef __MINGW32__
#undef LoadImage
#endif
//-----------------------------------------------------------------------------

class Config
{
public:
  static const int ALPHA = 0;
  static const int COLORKEY = 1;

  bool GetDisplayEnergyCharacter() const;
  void SetDisplayEnergyCharacter(bool dec);

  bool GetDisplayNameCharacter() const;
  void SetDisplayNameCharacter(bool dnc);

  bool GetDisplayWindParticles() const;
  void SetDisplayWindParticles(bool dwp);

  bool GetDefaultMouseCursor() const;
  void SetDefaultMouseCursor(bool dmc);

  bool GetScrollOnBorder() const;
  void SetScrollOnBorder(bool sob);

  bool IsNetworkActivated() const;

  int GetTransparency() const;

  std::string GetTtfFilename() const;

  std::string GetDataDir() const;
  std::string GetLocaleDir() const;
  std::string GetPersonalDir() const;

  // Tempory values (loaded from XML, but may change during running)
  struct tmp_xml_config{
    struct tmp_xml_net{
      bool enable_network;
    } network;
    struct tmp_xml_screen{
      int width,height;
      bool fullscreen;
    } video;
    struct tmp_xml_sound{
      bool music;
      bool effects;
      uint frequency;
    } sound;
    std::list<struct ConfigTeam> teams;
    std::string map_name;
  } tmp;

  static Config * GetInstance();
// bool Load();
  bool Save();
  const inline std::string &GetGameMode() const { return m_game_mode; };

protected:
  bool LoadXml(xmlpp::Element *xml);
  bool SaveXml();
  std::string GetEnv(const std::string & name, const std::string &default_value);

  std::string m_game_mode;
  bool m_xml_loaded;
  std::string m_filename;

  std::string data_dir, locale_dir, personal_dir;

  bool display_energy_character;
  bool display_name_character;
  bool display_wind_particles;
  bool default_mouse_cursor;
  bool scroll_on_border;
  std::string ttf_filename;

  int transparency;

private:
  Config();
  static Config * singleton;
  bool DoLoading(void);
};
#endif
