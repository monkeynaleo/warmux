/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
#include <WORMUX_base.h>
#include <WORMUX_singleton.h>
#include "graphic/surface.h"
#include "interface/mouse.h"
#include "map/maps_list.h"
#ifdef WIN32
   // Protects against macro definition of LoadImage when this header is included last.
#  undef LoadImage
#endif

class Sprite;
class XmlReader;
typedef struct _xmlNode xmlNode;
class MouseCursor;

class Profile
{
public:
  XmlReader *doc; //TODO move to private
  std::string filename;
  std::string relative_path;

  Profile();
  ~Profile();

  XmlReader * GetXMLDocument(void) const { return this->doc; }
};

class ResourceManager : public Singleton<ResourceManager>
{
  ResourceManager();
  ~ResourceManager();
  friend class Singleton<ResourceManager>;

public:
  void SetDataPath(const std::string& base_path);
  Surface LoadImage(const std::string& ressource_str, bool alpha = false,
                    bool set_colorkey = false, Uint32 colorkey = 0) const;

  Profile *LoadXMLProfile(const std::string& xml_filename, bool is_absolute_path) const;
  void UnLoadXMLProfile(Profile *profile) const;

  MouseCursor LoadMouseCursor(const Profile *profile, const std::string& resource_name, Mouse::pointer_t pointer_id) const;
  Color LoadColor(const Profile *profile, const std::string& resource_name) const;
  int LoadInt(const Profile *profile, const std::string& resource_name) const;
  Double LoadDouble(const Profile *profile, const std::string& resource_name) const;
  Point2i LoadPoint2i(const Profile *profile, const std::string& resource_name) const;
  Point2d LoadPoint2d(const Profile *profile, const std::string& resource_name) const;
  std::string LoadImageFilename(const Profile *profile, const std::string& resource_name) const;
  Surface LoadImage(const Profile *profile, const std::string& resource_name, bool alpha = true) const;

  Sprite *LoadSprite(const Profile *profile, const std::string& resource_name) const;

  // the following method is usefull if you have direct access to the xml file
  Sprite *LoadSprite(const xmlNode* sprite_elem, const std::string& resource_name, const std::string& main_folder) const;

  std::string GenerateMap(Profile *profile, InfoMap::Island_type generator,
                          const int width, const int height) const;
  const xmlNode*  GetElement(const Profile *profile, const std::string& ressource_type,
                             const std::string& ressource_name) const;

 private:
  std::string base_path;
};

ResourceManager& GetResourceManager();

#define LOAD_RES_IMAGE(name) GetResourceManager().LoadImage(res, name)
#define LOAD_RES_SPRITE(name) GetResourceManager().LoadSprite(res, name)
#define LOAD_RES_COLOR(name) GetResourceManager().LoadColor(res, name)

#endif /* _RESOURCE_MANAGER_H */
