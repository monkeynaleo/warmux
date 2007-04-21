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
 * Resource Manager: Load resources (images/sprites) suitable for SDL
 *                   Load directly or from refernece in xml resource profile
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de)
 *             Initial version
 *
 * TODO:       Keep reference to resources, better exceptions
 *****************************************************************************/

#ifndef _RESOURCE_MANAGER_H
#define _RESOURCE_MANAGER_H

#include <string>
#include "xml_document.h"
#include "../graphic/sprite.h"
#include "../map/random_map.h"

class Profile
{
  /* If you need this, implement it (correctly)*/
  Profile(const Profile&);
  const Profile& operator=(const Profile&);
  /*********************************************/

 public:
  XmlReader *doc;
  std::string filename;
  std::string relative_path;

  Profile();
  ~Profile();
};

class ResourceManager
{
 public:
   ResourceManager();
   ~ResourceManager();

   void AddDataPath(std::string base_path);
   Surface LoadImage(const std::string ressource_str, bool alpha = false, bool set_colorkey = false, Uint32 colorkey = 0);

   Profile *LoadXMLProfile(const std::string xml_filename, bool relative_path);
   void UnLoadXMLProfile(Profile *profile);

   Color LoadColor(const Profile *profile, const std::string resource_name);
   Point2i LoadPoint2i(const Profile *profile, const std::string resource_name);
   Point2d LoadPoint2d(const Profile *profile, const std::string resource_name);
   Surface LoadImage(const Profile *profile, const std::string resource_name);
   Sprite *LoadSprite(const Profile *profile, const std::string resource_name);
   Surface GenerateMap(Profile *profile, const int width, const int height);
   xmlpp::Element * GetElement(const Profile *profile, const std::string ressource_type, const std::string ressource_name);

 private:
   std::string base_path;
};

extern ResourceManager resource_manager;

#endif /* _RESOURCE_MANAGER_H */
