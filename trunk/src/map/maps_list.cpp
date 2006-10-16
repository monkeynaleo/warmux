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
 * Maps list
 *****************************************************************************/

#include "maps_list.h"
#include "map.h"
#include "../game/config.h"
#include "../tool/debug.h"
#include "../tool/file_tools.h"
#include "../tool/i18n.h"
#include <iostream>
#if !defined(WIN32) || defined(__MINGW32__)
#include <dirent.h>
#include <sys/stat.h>
#endif

InfoMap::InfoMap ()
{
  m_donnees_chargees = false;
  nb_mine = 0;
  nb_barrel = 0;
  wind.nb_sprite = 0;
  wind.need_flip = false;
}

bool InfoMap::Init (const std::string &map_name,
			            const std::string &directory)
{
  std::string nomfich;

  m_directory = directory;

  res_profile = NULL;
  m_donnees_chargees = false;

  try
  {
    nomfich = m_directory+"config.xml";

    // Load resources
    if (!IsFileExist(nomfich))
      return false;
    res_profile = resource_manager.LoadXMLProfile( nomfich, true),
    // Load preview
    preview = resource_manager.LoadImage( res_profile, "preview");
    // Load other informations
    LitDocXml doc;
    if (!doc.Charge (nomfich)) return false;
    if (!TraiteXml (doc.racine())) return false;
  }
  catch (const xmlpp::exception &e)
  {
    std::cout << std::endl
              << Format(_("XML error during loading map '%s' :"), map_name.c_str())
              << std::endl
              << e.what() << std::endl;
    return false;
  }

  MSG_DEBUG("map.load", "Map loaded: %s", map_name.c_str());

  return true;
}

bool InfoMap::TraiteXml (xmlpp::Element *xml)
{
  // Read author informations
  xmlpp::Element *author = LitDocXml::AccesBalise (xml, "author");
  if (author != NULL) {
    std::string
      a_name,
      a_nickname,
      a_country,
      a_email;

    LitDocXml::LitString (author, "name", a_name);
    LitDocXml::LitString (author, "nickname", a_nickname);
    if (!LitDocXml::LitString (author, "country", a_country))
      a_country = "?";
    if (!LitDocXml::LitString (author, "email", a_email))
      a_email = "?";

    if (!a_nickname.empty())
      author_info = Format
	(_("%s <%s> aka %s from %s"),
	 a_name.c_str(),
	 a_email.c_str(),
	 a_nickname.c_str(),
	 a_country.c_str());
    else
      author_info = Format
	(_("%s <%s> from %s"),
	 a_name.c_str(),
	 a_email.c_str(),
	 a_country.c_str());
  }

  LitDocXml::LitString (xml, "name", name);
  LitDocXml::LitBool (xml, "water", use_water);
  LitDocXml::LitUint (xml, "nb_mine", nb_mine);
  LitDocXml::LitUint (xml, "nb_barrel", nb_barrel);
  LitDocXml::LitBool (xml, "is_open", is_opened);

  xmlpp::Element *xmlwind = LitDocXml::AccesBalise (xml, "wind");
  if (xmlwind != NULL)
  {
    LitDocXml::LitUint (xmlwind, "nbr_sprite", wind.nb_sprite);
    LitDocXml::LitBool (xmlwind, "need_flip", wind.need_flip);

    if (wind.nb_sprite > MAX_WIND_OBJECTS)
      wind.nb_sprite = MAX_WIND_OBJECTS ;
  } else {
    wind.nb_sprite = 0;
  }
  wind.default_nb_sprite = wind.nb_sprite;

  return true;
}

void InfoMap::LoadData(){
  if (m_donnees_chargees)
    return;
  m_donnees_chargees = true;

  MSG_DEBUG("map.load", "Map data loaded: %s", name.c_str());

  img_terrain = resource_manager.LoadImage(res_profile, "map");
  img_ciel = resource_manager.LoadImage(res_profile,"sky");
}

void InfoMap::FreeData(){
  img_ciel.Free();
  img_terrain.Free();
  m_donnees_chargees = false;
}

Surface InfoMap::ReadImgGround(){
  LoadData();
  return img_terrain;
}

Surface InfoMap::ReadImgSky(){
  LoadData();
  return img_ciel;
}

bool InfoMap::DonneesChargees() const{
   return m_donnees_chargees;
}


MapsList* MapsList::singleton = NULL;

MapsList* MapsList::GetInstance()
{
  if (singleton == NULL) {
    singleton = new MapsList();
  }

  return singleton;
}

MapsList::MapsList()
{
  lst.clear() ;

  std::cout << "o " << _("Load maps:");
  terrain_actif = -1;

  Config * config = Config::GetInstance();
  std::string dirname = config->GetDataDir() + PATH_SEPARATOR + "map" + PATH_SEPARATOR;
#if !defined(WIN32) || defined(__MINGW32__)
  DIR *dir = opendir(dirname.c_str());
  struct dirent *file;
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL)
	  LoadOneMap (dirname, file->d_name);
    closedir (dir);
  } else {
    Error (Format(_("Unable to open maps directory (%s)!"),
		   dirname.c_str()));
  }
#else
  std::string pattern = dirname + "*.*";
  WIN32_FIND_DATA file;
  HANDLE file_search;
  file_search=FindFirstFile(pattern.c_str(),&file);
  if(file_search != INVALID_HANDLE_VALUE)
  {
    while (FindNextFile(file_search,&file))
	{
	  if(file.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	    LoadOneMap(dirname,file.cFileName);
	}
  } else {
    Error (Format(_("Unable to open maps directory (%s)!"),
		   dirname.c_str()));
  }
  FindClose(file_search);
#endif

#if !defined(WIN32) || defined(__MINGW32__)
  // Load personal maps
  dirname = config->GetPersonalDir() + PATH_SEPARATOR + "map";
  dir = opendir(dirname.c_str());
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL)
      LoadOneMap (dirname, file->d_name);
    closedir (dir);
  }
#endif
  std::cout << std::endl << std::endl;

  // On a au moins une carte ?
  if (lst.size() < 1)
    Error(_("You need at least one valid map !"));

  std::sort(lst.begin(), lst.end(), compareMaps);
}

void MapsList::LoadOneMap (const std::string &dir, const std::string &file)
{
  std::string fullname = dir+file;

#if !defined(WIN32) || defined(__MINGW32__)
  struct stat stat_file;
  if (file[0] == '.') return;
  if (stat(fullname.c_str(), &stat_file) != 0) return;
  if (!S_ISDIR(stat_file.st_mode)) return;
#endif

  InfoMap nv_terrain;
  bool ok = nv_terrain.Init (file, fullname + PATH_SEPARATOR);
  if (!ok) return;

  std::cout << (lst.empty()?" ":", ") << file;
  std::cout.flush();
  lst.push_back(nv_terrain);
}

int MapsList::FindMapById (const std::string &id)
{
  iterator
    terrain=lst.begin(),
    fin_terrain=lst.end();
  uint i=0;
  for (; i < lst.size(); ++i)
    if (lst[i].name == id)
      return i;
  return -1;
}

void MapsList::SelectMapByName (const std::string &nom)
{
  int index = FindMapById (nom);

  if (index == -1){
    index = 0;
    std::cout << Format(_("! Map %s not found :-("), nom.c_str()) << std::endl;
  }
  SelectMapByIndex (index);
}

void MapsList::SelectMapByIndex (uint index)
{
  assert (index < lst.size());
  if (terrain_actif == (int)index)
    return;

  terrain_actif = index;
}

int MapsList::GetActiveMapIndex ()
{
  return terrain_actif;
}

InfoMap& MapsList::ActiveMap()
{
  assert (0 <= terrain_actif);
  return lst.at(terrain_actif);
}

InfoMap& ActiveMap()
{
  return MapsList::GetInstance()->ActiveMap();
}

bool compareMaps(const InfoMap& a, const InfoMap& b)
{
  return a.name < b.name;
}

