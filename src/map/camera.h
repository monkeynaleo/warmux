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
 * Camera : Show where the action takes place (follow moving objects...)
 *          Follow mouse, when it's on the border of the window
 *****************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H
#include "../object/physical_obj.h"
#include "../tool/Point.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class Camera
{
  Point2i pos;
  Point2i last_char_target;
  PhysicalObj* followed_obj;
  unsigned int last_refresh;
  unsigned int mvt_begin; //Time when the last camera movement began (automatic movement)
                          //Used when changing followed object
  int user_mvt_to;//User move timeout = 0 -> move the camera in Refresh()
                  //                  = USER_MOVE_TO ->the camera just moved with the mouse

  Point2i shake_offset;
  int shake_offset_max;
  unsigned int shake_end_time;
  unsigned int shake_last_time;
public:
  Camera();
  void Reset();
  void Refresh();

  int GetX() const;
  int GetY() const;
  unsigned int GetWidth() const;
  unsigned int GetHeight() const;
  bool HasFixedX() const;
  bool HasFixedY() const;
  void SetXY(int x, int y);
  void SetdXY(int dx, int dy);

  void SignalExplosion(double force);
  void InitShake(unsigned int duration, int max_offset);
private:
  void InternSetXY(int x, int y);
  void InternSetdXY(int dx, int dy);

  PhysicalObj* GetFastestObj();
  void ComputeFocus_Object(int & x, int & y);
  void ComputeFocus_ActiveChar(int & x, int & y);
};

extern Camera camera;

//-----------------------------------------------------------------------------
#endif //CAMERA_H
