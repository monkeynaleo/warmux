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
 ******************************************************************************/

#ifndef BASIC_CONFIG_H
#define BASIC_CONFIG_H
#include <list>
#include <map>
#include <string>

class BasicConfig
{
  std::string config_file;

  std::map<std::string, std::string> str_value;
  std::map<std::string, int> int_value;
  std::map<std::string, bool> bool_value;
  std::list<std::string> supported_versions;

protected:
  void SetDefault(const std::string & name, const std::string & value);
  void SetDefault(const std::string & name, const int & value);
  void SetDefault(const std::string & name, const bool & value);

  void Load();
  void Display() const;
public:
  BasicConfig(const std::string & config_file);

  bool Get(const std::string & name, std::string & value) const;
  bool Get(const std::string & name, int & value) const;
  bool Get(const std::string & name, bool & value) const;

  bool IsVersionSupported(const std::string & version) const;
  const std::string SupportedVersions2Str() const;
};

#endif
