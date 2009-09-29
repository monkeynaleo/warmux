/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * Class containing object physic constants
 *****************************************************************************/

//-----------------------------------------------------------------------------
#include <cstring>
#include <iostream>
#include <WORMUX_debug.h>
#include "game/config.h"
#include "game/game_mode.h"
#include "include/base.h"
#include "physic/object_cfg.h"
#include "tool/xml_document.h"
//-----------------------------------------------------------------------------

ObjectConfig::ObjectConfig()
{
  m_rebound_factor = 0.01;
  m_air_resist_factor = 0.0;
  m_wind_factor = 1.0;
  m_gravity_factor = 1.0;
  m_auto_align_force = 0.0;
  m_rotating = false;
  m_center_position = Point2d(0,0);
  m_type = PhysicalEngine::RIGID_BODY;
}

void ObjectConfig::LoadXml(const std::string& obj_name, const std::string &config_file)
{
  const xmlNode* elem = NULL;
  XmlReader doc;

  if (config_file == "") {

    MSG_DEBUG("game_mode", "Load %s configuration from %s\n",
              obj_name.c_str(),
              GameMode::GetInstance()->GetName().c_str());

    const XmlReader* ddoc = GameMode::GetInstance()->GetXmlObjects();
    elem = XmlReader::GetMarker(ddoc->GetRoot(), obj_name);

  } else {

    MSG_DEBUG("game_mode", "** Load %s configuration from file %s\n",
              obj_name.c_str(), config_file.c_str());

    // Load Xml configuration
    ASSERT(doc.Load(config_file));
    elem = XmlReader::GetMarker(doc.GetRoot(), obj_name);
  }

  ASSERT(elem != NULL);
  XmlReader::ReadDouble(elem, "wind_factor", m_wind_factor);
  XmlReader::ReadDouble(elem, "air_resist_factor", m_air_resist_factor);
  XmlReader::ReadDouble(elem, "gravity_factor", m_gravity_factor);
  XmlReader::ReadDouble(elem, "rebound_factor", m_rebound_factor);
  XmlReader::ReadBool(elem, "rotating", m_rotating);
  XmlReader::ReadDouble(elem, "auto_align_force", m_auto_align_force);

  elem = XmlReader::GetMarker(elem, "center_position");
  if(elem){
    uint pos_x = 0;
    uint pos_y = 0;
    bool r;
    r = XmlReader::ReadUintAttr(elem, "x", pos_x);
    if (r)
     r = XmlReader::ReadUintAttr(elem, "y", pos_y);

    if(r)
    {
      m_center_position = Point2d(pos_x,pos_y);
    }
  }

}
