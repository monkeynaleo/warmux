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

#include <vector>
#include <string>
#include <map>
#include <SDL.h>
#include "xml_document.h"

struct Profile;
class Sprite;
struct Element;

class ResourceManager
{
 public:
   ResourceManager();
   ~ResourceManager();
  
   void AddDataPath( std::string base_path);
   SDL_Surface *LoadImage( std::string ressource_str, bool alpha = false, bool set_colorkey = false, Uint32 colorkey = 0);
  
   Profile *LoadXMLProfile( std::string xml_filename);
   Profile *UnLoadXMLProfile( Profile *profile);
   
   SDL_Surface *LoadImage( Profile *profile, std::string resource_name); 
   Sprite *LoadSprite( Profile *profile, std::string resource_name); 
   
 private:
   
   std::string base_path;
// std::vector< std::string> data_pathes;
// map < std::string, Resource *> resources;

   xmlpp::Element * GetElement( Profile *profile, std::string ressource_type, std::string ressource_name);

};

extern ResourceManager resource_manager;

#endif /* _RESOURCE_MANAGER_H */
