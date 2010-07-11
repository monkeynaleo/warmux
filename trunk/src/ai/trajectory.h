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
 * Represents the trajectory of a missile.
 *****************************************************************************/

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <WORMUX_point.h>

class Trajectory {
  private:
    Point2f initial_position;
    Point2f initial_speed;
    Point2f acceleration;
    Point2f half_acceleration;
  public:
    Trajectory(Point2f pos_0, Point2f v_0, Point2f a);
    const Point2i GetPositionAt(float t) const
    {
      return half_acceleration*(t*t) + initial_speed*t + initial_position;
    }
    float GetSpeedAt(float t) const
    {
      Point2f speed = acceleration*t + initial_speed;
      return speed.Norm();
    }
};
#endif
