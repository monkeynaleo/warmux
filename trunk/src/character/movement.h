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
 *****************************************************************************/

#ifndef MOVEMENT_H
#define MOVEMENT_H
#include <map>
#include <vector>
#include "../graphic/sprite.h"
#include "../tool/point.h"
#include "../tool/xml_document.h"

class member_mvt
{  // Position of a member relative to its superior one
public:
  Point2f pos;
  Point2f scale;
  float alpha;
  int angle; // angle in degrees
  bool follow_crosshair;
  bool follow_half_crosshair;
  bool follow_speed;
  bool follow_direction;
  member_mvt() { pos.x = pos.y = angle = follow_crosshair = follow_half_crosshair
                       = follow_speed = follow_direction = 0; 
                 alpha = scale.x = scale.y = 1.0;};
};

class Movement
{
  typedef std::map<std::string, struct member_mvt> member_def; // Describe the position of each member for a given frame

public:
  std::vector<member_def> frames;
  bool always_moving;
  int speed;
  uint test_left, test_right, test_top, test_bottom;

  std::string type;

  ~Movement();
  Movement(xmlpp::Element *xml);
};

#endif //MEMBER_H
