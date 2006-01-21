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

#include "camera.h"
//-----------------------------------------------------------------------------
#include "maps_list.h"
#include "map.h"
#include "../graphic/video.h"
#include "../game/time.h"
#include "../game/game_loop.h"
#include "../team/teams_list.h"
#include "../team/macro.h"
#include "../object/objects_list.h"
#include "../interface/mouse.h"
#include "../tool/random.h"
#include <iostream>

using namespace Wormux;
//-----------------------------------------------------------------------------

#ifdef DEBUG
//#define CAM_DBG
#endif

#define COUT_DBG std::cout << "[Camera] "

// After the user moved the camera, wait USER_MOVE_TIMEOUT ms, before
// refocusing "where the action takes place"
const unsigned int USER_MOVE_TIMEOUT = 3000;
// Time between 2 moves.
const unsigned int MOVE_TIME = 1000;
// Time the camera shake during an explosion
const unsigned int EXPLOSION_SHAKE_TIME = 500;
// Dx or Dy max accepted without doing a camera movement
const unsigned int DELTA_MVT_MAX = 10;
// Time between 2 shake
const unsigned int SHAKE_TIME_DT = 150;
// Border size between object and the screen edge while following an object
const unsigned int BORDER_SIZE = 100;
//-----------------------------------------------------------------------------
Camera camera;
//-----------------------------------------------------------------------------
Camera::Camera()
{
}

//-----------------------------------------------------------------------------
void Camera::Reset()
{
  pos.x=0;
  pos.y=0;
  followed_obj = NULL;
  last_refresh = 0;
  mvt_begin = 0; //Time when the last camera movement began (automatic movement)
                 //Used when changing followed object
  user_mvt_to = 0;//User move timeout = 0 -> move the camera in Refresh()
                  //                  = USER_MOVE_TO ->the camera just moved with the mouse
  shake_end_time = 0;
  shake_last_time = 0;
  shake_offset_max = 0;
  InternSetXY(ActiveCharacter().GetX(),
              ActiveCharacter().GetY());
}

//-----------------------------------------------------------------------------
int Camera::GetX() const { return pos.x; }
int Camera::GetY() const { return pos.y; }

//-----------------------------------------------------------------------------
bool Camera::HasFixedX() const { return (world.GetWidth() <= GetWidth()); }
bool Camera::HasFixedY() const { return (world.GetHeight() <= GetHeight()); }

//-----------------------------------------------------------------------------
void Camera::InternSetXY (int x, int y)
{ 
  if(!TerrainActif().infinite_bg)
  {
    if (!HasFixedX()) {
      pos.x = BorneLong(x, 0, world.GetWidth()-GetWidth());
    } else {
      //pos.x = BorneLong(x, 0, GetWidth()-world.GetWidth());
        pos.x = BorneLong(x, world.GetWidth()-GetWidth(), 0);
    }
  }
  else
  {
    pos.x = x;
  }

  if(!TerrainActif().infinite_bg)
  {
    if (!HasFixedY()) {
      pos.y = BorneLong(y, 0, world.GetHeight()-GetHeight());
    } else {
      //pos.y = BorneLong(y, 0, GetHeight()-world.GetHeight());
          pos.y = BorneLong(y, world.GetHeight()-GetHeight(), 0);
    }
  }
  else
  {
    if( y > (int)world.GetHeight()-(int)GetHeight() )
      pos.y = (int)world.GetHeight()-(int)GetHeight();
    else
      pos.y = y;
  }    
}

//----------------------------------------------------------------------------
void Camera::InternSetdXY(int dx, int dy)
{ 
  if (camera.HasFixedX()) dx = 0;
  if (camera.HasFixedY()) dy = 0;
  if ((dx == 0) && (dy == 0)) return;
  InternSetXY (pos.x+dx,pos.y+dy);
}

//-----------------------------------------------------------------------------
void Camera::SetdXY(int dx, int dy)
{
  user_mvt_to = USER_MOVE_TIMEOUT;
  InternSetdXY(dx,dy);
}

void Camera::SetXY (int x, int y)
{
  user_mvt_to = USER_MOVE_TIMEOUT;
  InternSetXY(x,y);
}
//-----------------------------------------------------------------------------

void Camera::Refresh()
{
  //Check user movement of the camera
  mouse.TestCamera();
  if(user_mvt_to != 0)
  {
    //The user is currently moving the camera
    user_mvt_to -= global_time.Read() - last_refresh;
    last_refresh = global_time.Read();
    if(user_mvt_to <= 0)
    {
      user_mvt_to = 0;
      mvt_begin = global_time.Read();
#ifdef CAM_DBG
      COUT_DBG << "User defined camera movement ending!" << std::endl;
#endif
    }
    else
      return;
  }

  //Choose which object we are going to follow
  PhysicalObj* last_followed_obj = followed_obj;
  int focus_x = 0;
  int focus_y = 0;

  //Focus on the object with the highest speed
  PhysicalObj* fastest_obj = GetFastestObj();
  if(fastest_obj!=NULL && !ActiveCharacter().IsWalking())
  {
    followed_obj = fastest_obj;
    ComputeFocus_Object(focus_x, focus_y);
  }
  else
  {
    //If nothing move, then focus on current character
    if(game_loop.ReadState() != gamePLAYING) return;
    followed_obj = &ActiveCharacter();
    ComputeFocus_ActiveChar(focus_x, focus_y);
  }

  // Begin a movement if we changed the focused object
  if(followed_obj != last_followed_obj)
  {
    mvt_begin = global_time.Read();
#ifdef CAM_DBG
    COUT_DBG << "Following \"" << followed_obj->m_name << "\" object" << std::endl;
#endif
  }

  unsigned int dt = global_time.Read() - mvt_begin;
  if(global_time.Read() - mvt_begin < MOVE_TIME)
  {
    int dx = ((focus_x - GetX()) * (int)dt) / (int)MOVE_TIME;
    int dy = ((focus_y - GetY()) * (int)dt) / (int)MOVE_TIME;

    if(dx > 0) dx++;
    if(dx < 0) dx--;
    if(dy > 0) dy++;
    if(dx < 0) dy--;

    InternSetdXY(dx,dy);
  }
  else
  {
    InternSetXY(focus_x,focus_y);
  }

  //Shake the camera (during explosion...)
  if(global_time.Read() < shake_end_time)
  {
    if(global_time.Read() > shake_last_time)
    {
      shake_offset.x = RandomLong(shake_offset_max/2,shake_offset_max);
      shake_offset.x *= RandomBool() ? 1 : -1;
      shake_offset.y = RandomLong(shake_offset_max/2,shake_offset_max);
      shake_offset.y *= RandomBool() ? 1 : -1;
      shake_offset_max = (int) ((float)shake_offset_max / 1.1);
    }
    InternSetdXY(shake_offset.x,shake_offset.y);
  }

  last_refresh = global_time.Read();
}

//-----------------------------------------------------------------------------
PhysicalObj* Camera::GetFastestObj()
{
  //We don't exactly send back the quickest object:
  //In order of priority:
  //Return the playing character, if he is moving:
  double max_speed=0.0;
  PhysicalObj* fastest_obj=NULL;

  //Return the skin with the highst speed:
  POUR_TOUS_VERS(team, character)
  if(character->IsMoving() && &(*character) != &ActiveCharacter())
  {
    double speed,angle;
    character->GetSpeed(speed,angle);
    if(speed > max_speed)
    {
      max_speed = speed;
      fastest_obj = (PhysicalObj*) &(*character);
    }
  }
  if(max_speed != 0.0)
    return fastest_obj;

  POUR_TOUS_OBJETS(obj)
  {
    double speed=0.0;
    double angle=0.0;
    obj->ptr->GetSpeed(speed,angle);
    if(speed > max_speed)
    {
      max_speed = speed;
      fastest_obj = obj->ptr;
    }
  }

  if(max_speed != 0.0)
    return fastest_obj;

  return NULL;
}

//-----------------------------------------------------------------------------
void Camera::ComputeFocus_Object(int & x, int & y)
{
  //Center object in the window
  uint window_x = GetWidth() /2;  //Final position of the followed,
  uint window_y = GetHeight()/2; // in the window
  x = followed_obj->GetX() - (int)window_x;
  y = followed_obj->GetY() - (int)window_y;
}

//-----------------------------------------------------------------------------
void Camera::ComputeFocus_ActiveChar(int & x, int & y)
{
  if(ActiveTeam().GetWeapon().min_angle != ActiveTeam().GetWeapon().max_angle)
  {
    //The current weapon uses crosshair
    double angle = ActiveTeam().crosshair.GetAngle() * M_PI / 180.0; //interval -PI -> PI
    angle += M_PI;
    if(angle > M_PI)
      angle -= 2 * M_PI;
    uint window_x = GetWidth()/2+(int)((double)(GetHeight()/2 - BORDER_SIZE) * cos(angle));  //Final position in the window
    uint window_y = GetHeight()/2+(int)((double)(GetHeight()/2 - BORDER_SIZE) * sin(angle));  //Final position in the window
    window_x -= followed_obj->GetWidth()/2;
    window_y -= followed_obj->GetHeight()/2;

    x = followed_obj->GetX() - (int)window_x;
    y = followed_obj->GetY() - (int)window_y;

    //If the focus point changed too much, begin mvt
    if(abs(x - last_char_target.x) > (int)DELTA_MVT_MAX
    || abs(y - last_char_target.y) > (int)DELTA_MVT_MAX)
      mvt_begin = global_time.Read();

    last_char_target.x = x;
    last_char_target.y = y;
  }
  else
  {
    //The current weapon  doesn't use crosshair -> center on skin
    uint window_x = GetWidth() /2;  //Final position of the followed,
    uint window_y = GetHeight()/2; // in the window
    x = followed_obj->GetX() - (int)window_x;
    y = followed_obj->GetY() - (int)window_y;
  }  
}

//-----------------------------------------------------------------------------
void Camera::InitShake(unsigned int duration, int max_offset)
{
  shake_end_time = global_time.Read() + duration;
  if(max_offset > shake_offset_max)
    shake_offset_max = max_offset;
  shake_offset.x = RandomBool() ? max_offset : -max_offset;
  shake_offset.y = RandomBool() ? max_offset : -max_offset;
  shake_last_time = global_time.Read();
}

//-----------------------------------------------------------------------------
void Camera::SignalExplosion(double force)
{
  InitShake(EXPLOSION_SHAKE_TIME, (int)(30.0 * force));
}

//-----------------------------------------------------------------------------
uint Camera::GetWidth() const { return video.GetWidth(); }
uint Camera::GetHeight() const { return video.GetHeight(); }

//-----------------------------------------------------------------------------
