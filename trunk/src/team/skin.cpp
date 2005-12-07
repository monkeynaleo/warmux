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
 * Skin : gestion des diff�rents aspects pour les vers. Sp�cifications :
 * - image (animation),
 * - position du viseur pour les diff�rentes armes,
 * - taille,
 * - ...
 *
 * Chaque avatar diff�rent a une seule instance. Un ver poss�de seulement un
 * pointeur vers une de ces instances.
 *****************************************************************************/

#include "skin.h"
//-----------------------------------------------------------------------------
#include "../game/config.h" // DATADIR
#include "../graphic/graphism.h"
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../tool/string_tools.h"
#ifndef CL
#include "../graphic/sprite.h"
#include "../tool/resource_manager.h"
#endif
#include <iostream>
#if !defined(WIN32) || defined(__MINGW32__)
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace Wormux;
using namespace std;

const std::string NOMFICH_CONFIG = "config.xml";
//-----------------------------------------------------------------------------
typedef std::map<std::string, Skin>::value_type paire_avatar;
std::map<std::string, Skin> skins_list;
//-----------------------------------------------------------------------------
CfgSkin::CfgSkin() { Reset(); }
CfgSkin::~CfgSkin() { Reset(); }
void CfgSkin::Reset()
{
#ifdef CL
  image = CL_Sprite(); // NULL sprite
#else
  image = new Sprite();
#endif 
  test_bottom = 0;
  test_top = 0;
  test_dx = 0;
}
//-----------------------------------------------------------------------------
CfgSkin_Walking::CfgSkin_Walking() 
{ 
   Reset(); 
}

void CfgSkin_Walking::Reset()
{
  CfgSkin::Reset();
  repetition_frame = 1;
  hand_position.clear();
}
//-----------------------------------------------------------------------------
CfgSkin_Anim::CfgSkin_Anim() { Reset(); }
void CfgSkin_Anim::Reset()
{
  utilise = false;
  pos.dx = 0;
  pos.dy = 0;
}
//-----------------------------------------------------------------------------

Skin::Skin() { Reset(); }

//-----------------------------------------------------------------------------

void Skin::Reset()
{
  many_walking_skins.clear();
  many_skins.clear();
  anim.Reset();
  anim.utilise = false;
  anim.vitesse = 100;
}

//-----------------------------------------------------------------------------

bool Skin::Charge (const std::string &nom, const std::string &repertoire)
{
  try
  {
    std::string nomfich;
    LitDocXml doc;

    Reset();

    // Load XML file
    nomfich = repertoire+"config.xml";

#ifdef CL
     CL_ResourceManager res(nomfich, false);
#else
     Profile *res = resource_manager.LoadXMLProfile( nomfich);
#endif
    
     if (!doc.Charge (nomfich)) {
       return false;
    }
    
    LoadManySkins(doc.racine(),res);

    // <animation>
    anim.utilise = 
      (LitDocXml::Access (doc.racine(), "sprite", "animation") != NULL);
    if (anim.utilise) {
#ifdef CL
      anim.image = CL_Sprite("animation", &res);
      anim.image.set_play_loop (false);
#else
      anim.image = resource_manager.LoadSprite( res, "animation");
      anim.image->Start();
      anim.image->SetShowOnFinish(Sprite::show_blank);
#endif
    }
  }
  catch (const xmlpp::exception &e)
  {
    std::cout << std::endl
		      << Format(_("Error during skin loading (%s) :"),nom.c_str())
              << std::endl << e.what() << std::endl;
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------

#ifdef CL
void Skin::LoadManySkins(xmlpp::Element *root, CL_ResourceManager &res) {
#else
void Skin::LoadManySkins(xmlpp::Element *root, Profile *res) {   
#endif
  many_skins.clear();

  xmlpp::Node::NodeList nodes = root -> get_children("sprite");
  xmlpp::Node::NodeList::iterator 
  it=nodes.begin(),
  end=nodes.end();

  for (; it != end; ++it) {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it);
    assert (elem != NULL);

    std::string skin_name;
    if (!LitDocXml::LitAttrString(elem, "name", skin_name)) {
       continue;
    }
    if (skin_name=="animation") {
       continue;
    }
    xmlpp::Node::NodeList nodes = elem -> get_children("hand");
    if(nodes.end()==nodes.begin())
    {
      CfgSkin config;
      xmlpp::Element *xml_config = LitDocXml::Access (root, "sprite", skin_name);
      Xml_LitRectTest(xml_config,config);
#ifdef CL
      config.image = CL_Sprite(skin_name, &res);
#else
      config.image = resource_manager.LoadSprite( res, skin_name);
      config.image->EnableFlippingCache();
#endif
      many_skins.insert(paire_skin(skin_name,config));
    }
    else
    {
      CfgSkin_Walking config;
      xmlpp::Element *xml_config = LitDocXml::Access (root, "sprite", skin_name);
      Xml_LitRectTest(xml_config,config);
#ifdef CL
      config.image = CL_Sprite(skin_name, &res);
#else
      config.image = resource_manager.LoadSprite( res, skin_name);
      config.image->EnableFlippingCache();
#endif       
      GetXmlConfig(xml_config,config);

      many_walking_skins.insert(paire_walking_skin(skin_name,config));
    }
  }
}

//-----------------------------------------------------------------------------

void Skin::Xml_LitRectTest (xmlpp::Element *elem, CfgSkin &img)
{
  img.test_dx = img.test_top = img.test_bottom = 0;
  if (elem == NULL) return;
  xmlpp::Element *collision_rect = LitDocXml::AccesBalise (elem, "collision_rect");
  if (collision_rect == NULL) return;
  LitDocXml::LitAttrUint (collision_rect, "dx", img.test_dx);
  LitDocXml::LitAttrUint (collision_rect, "top", img.test_top);
  LitDocXml::LitAttrUint (collision_rect, "bottom", img.test_bottom);
}

//-----------------------------------------------------------------------------

void Skin::Xml_ReadHandPosition(xmlpp::Element *root, CfgSkin_Walking &config) {
#ifdef CL
  int n = config.image.get_frame_count();
#else
  int n = config.image->GetFrameCount();
#endif 
  skin_translate_t pos = {0,0};
  config.hand_position.clear();
  config.hand_position.assign (n, pos);

  xmlpp::Node::NodeList nodes = root -> get_children("hand");
  xmlpp::Node::NodeList::iterator 
    it=nodes.begin(),
    end=nodes.end();

  for (; it != end; ++it) {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it);
    assert (elem != NULL);

    if (!LitDocXml::LitAttrInt(elem, "x", pos.dx)) continue;
    if (!LitDocXml::LitAttrInt(elem, "y", pos.dy)) continue;

    std::string frame_str;
    if (!LitDocXml::LitAttrString(elem, "frame", frame_str)) continue;

    if (frame_str == "*") {
      config.hand_position.assign (n, pos);	
    } else {
      int frame;
      if (!str2int (frame_str, frame)) continue;
      --frame;
      if (frame < 0) continue;
      if (n<frame) continue;
      config.hand_position.at(frame) = pos;
    }
  }
}

//-----------------------------------------------------------------------------

bool Skin::GetXmlConfig (xmlpp::Element *xml, CfgSkin_Walking &config)
{
  if (xml != NULL) {

    // <walking><wormux repetition="xxx">
    xmlpp::Element *animation = LitDocXml::AccesBalise (xml, "wormux");
    if (animation != NULL) {
      LitDocXml::LitAttrUint (animation, "repetition", 
			      config.repetition_frame);
    }

    Xml_ReadHandPosition(xml,config);
  }
  
  return true;
}

//-----------------------------------------------------------------------------

void LoadOneSkin (const std::string &dir, const std::string &file)
{
  // Skip '.', '..' and hidden files
  if (file[0] == '.') return;

  std::string fullname = dir+file;

#if !defined(WIN32) || defined(__MINGW32__)
  // Only try directories
  struct stat stat_file;
  if (stat(fullname.c_str(), &stat_file) != 0) return;
  if (!S_ISDIR(stat_file.st_mode)) return;
#endif

  // The config file doesn't exist ?
  if (!FichierExiste(fullname+PATH_SEPARATOR+NOMFICH_CONFIG)) return;

  // Try to load the skin
  Skin skin;
  if (!skin.Charge (file, fullname+PATH_SEPARATOR)) return;

  // Add the skin to the list
  if (!skins_list.empty()) std::cout << ',';
  std::cout << ' ' << file;
  std::cout.flush();
  skins_list.insert (paire_avatar(file, skin));
}

//-----------------------------------------------------------------------------

void InitSkins()
{  
  std::cout << "o " << _("Load skins:");
  std::cout.flush();
   
#if !defined(WIN32) || defined(__MINGW32__)
  struct dirent *file;

  std::string dirname = config.data_dir+"skin/";
   
  std::cout << "looking in the directory : " << dirname << std::endl;
  std::cout.flush();
  
  DIR *dir = opendir(dirname.c_str());
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL)
      LoadOneSkin(dirname, file->d_name);
    closedir (dir);
  } else {
	  Erreur (Format(_("Unable to open skins directory (%s) !"), 
				     dirname.c_str()));
  }
#else
  std::string dirname = Wormux::config.data_dir+"skin\\";
  std::string pattern = dirname + "*.*";
  WIN32_FIND_DATA file;
  HANDLE file_search;
  file_search=FindFirstFile(pattern.c_str(),&file);
  if(file_search != INVALID_HANDLE_VALUE)
  {
    while (FindNextFile(file_search,&file))
	{
	  if(file.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	    LoadOneSkin(dirname,file.cFileName);
	}
  } else {
	  Erreur (Format(_("Unable to open skins directory (%s) !"), 
				     dirname.c_str()));
  }
  FindClose(file_search);
#endif
   
#if !defined(WIN32) || defined(__MINGW32__)
  dirname = config.GetWormuxPersonalDir()+"skin/";
  dir = opendir(dirname.c_str());
  if (dir != NULL) {
    while ((file = readdir(dir)) != NULL)
      LoadOneSkin(dirname, file->d_name);
    closedir (dir);
  }
#endif
  std::cout << "done!" << std::endl;
}

//-----------------------------------------------------------------------------
