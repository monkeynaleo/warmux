/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Camera : follow an object, center on it or follow mouse interaction.
 *****************************************************************************/

#include "camera.h"
#include "map.h"
#include "wind.h"
#include "game/game.h"
#include "graphic/video.h"
#include "include/app.h"
#include "interface/mouse.h"
#include "object/physical_obj.h"
#include "team/teams_list.h"
#include "tool/debug.h"
#include "tool/math_tools.h"

const Point2i CAMERA_SPEED(20, 20);

Camera* Camera::singleton = NULL;

Camera * Camera::GetInstance()
{
  if (singleton == NULL) {
    singleton = new Camera();
  }
  return singleton;
}

Camera::Camera():
  auto_crop(true),
  followed_object(NULL),
  throw_camera(false)
{}

void Camera::Reset()
{
  auto_crop = true;
  followed_object = NULL;
  throw_camera = false;
}

bool Camera::HasFixedX() const{
  return (int)world.GetWidth() <= GetSizeX();
}

bool Camera::HasFixedY() const{
  return (int)world.GetHeight() <= GetSizeY();
}

void Camera::SetXYabs(int x, int y){
  AppWormux * app = AppWormux::GetInstance();

  if( !HasFixedX() )
    position.x = BorneLong(x, 0, world.GetWidth() - GetSizeX());
  else
    position.x = - (app->video->window.GetWidth() - world.GetWidth())/2;

  if( !HasFixedY() )
    position.y = BorneLong(y, 0, world.GetHeight()-GetSizeY());
  else
    position.y = - (app->video->window.GetHeight() - world.GetHeight())/2;

  throw_camera = true;
}

void Camera::SetXY(Point2i pos){
  pos = pos * FreeDegrees();
  if( pos.IsNull() )
    return;

  SetXYabs(position + pos);
}

void Camera::AutoCrop(){
  /* Stuff is put static in order to be able to reach the last position
   * of the object the camera was following, in case it desapears. This
   * typically happen when something explodes or a character dies. */
  static Point2i pos(0, 0);
  static Point2i size(1, 1);

  if (followed_object && !followed_object->IsGhost() )
  {
    pos = followed_object->GetPosition();
    size = followed_object->GetSize();
  }

  if( pos.y < 0 )
    pos.y = 0;

  Point2i dstMax = GetSize()/2;

  ASSERT(!dstMax.IsNull());

  Point2i cameraBR = GetSize() + position;
  Point2i objectBRmargin = pos + size + GetSize()/2;
  Point2i dst(0, 0);

  dst += cameraBR.inf(objectBRmargin) * (objectBRmargin - cameraBR);
  dst += (pos - GetSize()/2).inf(position) * (pos - GetSize()/2 - position);

  SetXY(dst * CAMERA_SPEED / dstMax );
}

void Camera::Refresh(){
  throw_camera = false;

  // mouse mouvement
  Mouse::GetInstance()->TestCamera();
  if (throw_camera) return;

  if (auto_crop)
    AutoCrop();
}

void Camera::FollowObject(const PhysicalObj *obj, bool follow, bool center_on){
  MSG_DEBUG( "camera.tracking", "Following object %s, center_on=%d",
                                 obj->GetName().c_str(), center_on);

  if ((center_on) && (followed_object != obj || !IsVisible(*obj)))
  {
    bool visible = IsVisible(*obj);
    auto_crop = follow;
    if(!visible)
      wind.RandomizeParticlesPos();
  }
  followed_object = obj;
}

void Camera::StopFollowingObj(const PhysicalObj*){
  if(Game::GetInstance()->IsGameFinished())
    return;

  followed_object = NULL;
}

bool Camera::IsVisible(const PhysicalObj &obj) const {
   return Intersect( obj.GetRect() );
}

