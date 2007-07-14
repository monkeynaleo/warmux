/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Maps List
 *****************************************************************************/

#ifndef LST_TERRAINS_H
#define LST_TERRAINS_H
#include <string>
#include <map>
#include "include/base.h"
#include "tool/xml_document.h"
#include "graphic/surface.h"

// Forward declarations
class Profile;

class InfoMap{

private:

  std::string name;
  std::string author_info;
  std::string music_playlist;
  Surface img_ground, img_sky;
  Surface preview;

  uint nb_mine;
  uint nb_barrel;

  bool is_opened;
  bool use_water;
  bool is_basic_info_loaded;
  bool is_data_loaded;
  bool random;

  Profile *res_profile;

  bool ProcessXmlData(xmlpp::Element *xml);
  void LoadData();

public:
  std::string m_directory;
  std::string m_map_name;
  struct s_wind
  {
    uint nb_sprite;
    uint default_nb_sprite;
    bool need_flip; //do we need to flip the sprite when it changes direction?
    float rotation_speed;
  } wind;

public:
  InfoMap ();
  bool Init(const std::string &nom, const std::string &repertoire);
  bool LoadBasicInfo();
  void FreeData();

  const std::string& GetRawName() const;
  const std::string& ReadFullMapName();
  const std::string& ReadAuthorInfo();
  const std::string& ReadMusicPlaylist();

  Surface ReadImgGround();
  Surface ReadImgSky();
  const Surface& ReadPreview();

  uint GetNbBarrel();
  uint GetNbMine();
  const Profile * const ResProfile() const;

  bool IsOpened();
  bool UseWater();

};


class MapsList
{
public:
  std::vector<InfoMap> lst;
  typedef std::vector<InfoMap>::iterator iterator;

private:
  int terrain_actif;
  bool m_init;
  static MapsList * singleton;

  void LoadOneMap (const std::string &dir, const std::string &file);
  MapsList();

public:
  static MapsList * GetInstance();

  // Return -1 if fails
  int FindMapById (const std::string &id);
  void SelectMapByName (const std::string &nom);
  void SelectMapByIndex (uint index);
  int GetActiveMapIndex ();
  InfoMap& ActiveMap();
};

InfoMap& ActiveMap();

bool compareMaps(const InfoMap& a, const InfoMap& b) ;

#endif
