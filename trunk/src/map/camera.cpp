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
 * Camera : follow an object, center on it or follow mouse interaction.
 *****************************************************************************/

#include <WORMUX_debug.h>
#include <WORMUX_random.h>

#include "character/character.h"
#include "game/config.h"
#include "game/game.h"
#include "game/time.h"
#include "graphic/video.h"
#include "include/app.h"
#include "interface/cursor.h"
#include "interface/interface.h"
#include "interface/mouse.h"
#include "map/camera.h"
#include "map/map.h"
#include "object/physical_obj.h"
#include "team/teams_list.h"
#include "tool/math_tools.h"
#include "tool/string_tools.h"

const Point2d MAX_CAMERA_SPEED(5000, 5000);
const Point2d MAX_CAMERA_ACCELERATION(1.5,1.5);
const Double ANTICIPATION = 18;
const Double REACTIVITY = 0.6;
const Double SPEED_REACTIVITY = 0.05;
const int SPEED_REACTIVITY_CEIL = 4;

#define SCROLL_KEYBOARD  20 // pixel

const Double ADVANCE_ANTICIPATION = 10;
const int REALTIME_FOLLOW_LIMIT = 25;
const Double REALTIME_FOLLOW_FACTOR = 0.15;

uint MAX_REFRESHES_PER_SECOND = 100;

Camera::Camera()
  : m_started_shaking(0)
  , m_shake_duration(0)
  , m_shake_amplitude(0, 0)
  , m_shake_centerpoint(0, 0)
  , m_shake(0, 0)
  , m_last_time_shake_calculated(0)
  , m_speed(0, 0)
  , m_stop(false)
  , m_control_mode(NO_CAMERA_CONTROL)
  , m_begin_controlled_move_time(0)
  , m_mouse_counter(0)
  , m_scroll_start_pos(0,0)
  , m_last_mouse_pos(0,0)
  , m_scroll_vector(0.0,0.0)
  , auto_crop(true)
  , followed_object(NULL)
{
  pointer_used_before_scroll = Mouse::POINTER_SELECT;
}

void Camera::Reset()
{
  m_stop = false;
  auto_crop = true;
  followed_object = NULL;
  m_begin_controlled_move_time = 0;
  m_control_mode = NO_CAMERA_CONTROL;
  SetXYabs(GetWorld().GetSize() / 2);
}

bool Camera::HasFixedX() const
{
  return (int)GetWorld().GetWidth() <= GetSizeX();
}

bool Camera::HasFixedY() const
{
  return (int)GetWorld().GetHeight() <= GetSizeY();
}

void Camera::SetXYabs(int x, int y)
{
  AppWormux * app = AppWormux::GetInstance();

  if (!HasFixedX())
    position.x = InRange_Long(x, 0, GetWorld().GetWidth() - GetSizeX());
  else
    position.x = - (app->video->window.GetWidth() - GetWorld().GetWidth())/2;

  if (!HasFixedY())
    position.y = InRange_Long(y, 0, GetWorld().GetHeight() - GetSizeY());
  else
    position.y = - (app->video->window.GetHeight() - GetWorld().GetHeight())/2;
}

void Camera::SetXY(Point2i pos)
{
  pos = pos * FreeDegrees();
  if (pos.IsNull())
    return;

  SetXYabs(position + pos);
}

void Camera::AutoCrop()
{
  /* Stuff is put static in order to be able to reach the last position
   * of the object the camera was following, in case it desapears. This
   * typically happen when something explodes or a character dies. */
  static Point2i obj_pos(0, 0);

  Point2i target(0,0);
  bool stop = false;

  if (followed_object && !(followed_object->IsGhost())) {

    /* compute the ideal position!
     * it takes the physical object direction into account
     */
    obj_pos = followed_object->GetCenter();

    if (obj_pos > GetPosition() + GetSize() / 7 &&
        obj_pos < GetPosition() + 6 * GetSize() / 7) {
      if (m_stop)
        stop = true;

    } else {
      m_stop = false;
    }

    target = obj_pos;

    if (followed_object->IsMoving()) {
      Point2d anticipation = ADVANCE_ANTICIPATION * followed_object->GetSpeed();

      //limit anticipation to screen size/3
      Point2d anticipation_limit = GetSize()/3;
      target += anticipation.clamp(-anticipation_limit, anticipation_limit);
    }

    target -= GetSize()/2;

  } else {
    target = GetPosition();
    m_stop = true;
  }

  //Compute new speed to reach target
  Point2d acceleration = (target - ANTICIPATION*m_speed - position)*REACTIVITY;
  // Limit acceleration
  acceleration = acceleration.clamp(-MAX_CAMERA_ACCELERATION, MAX_CAMERA_ACCELERATION);

  // std::cout<<"acceleration before : "<<acceleration.x<<" "<<acceleration.y<<std::endl;
  if (abs((int)m_speed.x) > SPEED_REACTIVITY_CEIL) {
    acceleration.x *= (1 + SPEED_REACTIVITY * (abs((int)m_speed.x) - SPEED_REACTIVITY_CEIL));
  }

  if (abs((int)m_speed.y) > SPEED_REACTIVITY_CEIL) {
    acceleration.y *= (1 + SPEED_REACTIVITY * (abs((int)m_speed.y) - SPEED_REACTIVITY_CEIL));
  }

  if (stop) {
    m_speed = m_speed/2;

  } else {

    //Apply acceleration
    m_speed = m_speed + acceleration;

    //Realtime follow is enable if object is too fast to be correctly followed

    if (abs((int)followed_object->GetSpeed().x) > REALTIME_FOLLOW_LIMIT) {
      m_speed.x = (target.x - position.x) * REALTIME_FOLLOW_FACTOR;
    }

    if (abs((int)followed_object->GetSpeed().y) > REALTIME_FOLLOW_LIMIT) {
      m_speed.y = (target.y - position.y) * REALTIME_FOLLOW_FACTOR;
    }

    //Limit
    m_speed = m_speed.clamp(-MAX_CAMERA_SPEED, MAX_CAMERA_SPEED);
  }

  //Update position
  Point2i next_position = m_speed;
  SetXY(next_position);

  if (!m_stop && next_position.IsNull() && followed_object->GetSpeed().IsNull()) {
    m_stop = true;
  }
}

void Camera::SaveMouseCursor()
{
  Mouse::pointer_t current_pointer = Mouse::GetInstance()->GetPointer();
  if (current_pointer != Mouse::POINTER_MOVE &&
      current_pointer != Mouse::POINTER_ARROW_UP &&
      current_pointer != Mouse::POINTER_ARROW_DOWN &&
      current_pointer != Mouse::POINTER_ARROW_LEFT &&
      current_pointer != Mouse::POINTER_ARROW_RIGHT &&
      current_pointer != Mouse::POINTER_ARROW_DOWN_RIGHT &&
      current_pointer != Mouse::POINTER_ARROW_UP_RIGHT &&
      current_pointer != Mouse::POINTER_ARROW_UP_LEFT &&
      current_pointer != Mouse::POINTER_ARROW_DOWN_LEFT) {
    pointer_used_before_scroll = current_pointer;
  }
}

void Camera::RestoreMouseCursor()
{
  Mouse::GetInstance()->SetPointer(pointer_used_before_scroll);
}

void Camera::ScrollCamera()
{
  if (!Mouse::GetInstance()->HasFocus()) // The application has not the focus, don't move the camera!!
    return;

  Point2i mousePos = Mouse::GetInstance()->GetPosition();

  if (!Config::GetInstance()->GetScrollOnBorder()) {
    /* Kinetic scrolling */
    if (!SDL_GetMouseState(NULL, NULL)) {
      m_scroll_start_pos = Point2i();
      m_last_mouse_pos   = Point2i();
      m_mouse_counter    = 0;

      if (!m_scroll_vector.IsNull()) {
        Point2d brk = m_scroll_vector.GetNormal() * Double(0.2);

        MSG_DEBUG("camera",
                  "scroll_vector=(%s,%s)  scroll_vector_break=(%s,%s)\n",
                  Double2str(m_scroll_vector.GetX(), 3).c_str(),
                  Double2str(m_scroll_vector.GetY(), 3).c_str(),
                  Double2str(brk.GetX(), 3).c_str(),
                  Double2str(brk.GetY(), 3).c_str());

        m_scroll_vector -= brk;
        SetXY(-m_scroll_vector);
        SetAutoCrop(false);
        if (m_scroll_vector.Norm() < 1)
          m_scroll_vector = Point2d();
      }
      return;
    }
    m_mouse_counter++;

    if (m_scroll_start_pos.IsNull())
      m_scroll_start_pos = mousePos;
    if (m_last_mouse_pos.IsNull())
      m_last_mouse_pos = mousePos;

      m_scroll_vector = mousePos - m_scroll_start_pos;
      m_scroll_vector = m_scroll_vector / m_mouse_counter;
      MSG_DEBUG("camera",
                "scroll_vector=(%f,%f)\n"
                "mousePos=(%i,%i)"
                "lastMousePos=(%i,%i)"
                "scrollStartPos=(%i,%i)",
                Double2str(m_scroll_vector.GetX(), 3).c_str(),
                Double2str(m_scroll_vector.GetY(), 3).c_str(),
                Double2str(m_last_mouse_pos.GetX(), 3).c_str(),
                Double2str(m_last_mouse_pos.GetY(), 3).c_str(),
                Double2str(m_scroll_start_pos.GetX(), 3).c_str(),
                Double2str(m_scroll_start_pos.GetY(), 3).c_str());

      SetXY(-(mousePos-m_last_mouse_pos));
      m_last_mouse_pos = mousePos;
      SetAutoCrop(false);
  } else {

    uint zone_size = Config::GetInstance()->GetScrollBorderSize();
    Point2i sensitZone(zone_size, zone_size);

    /* tstVector represents the vector of how deep the cursor is in a sensit
     * zone; negative value means that the camera has to reduce its coordinates,
     * a positive value means that it should increase. Actually reduce means
     * LEFT/UP (for x/y) and increase RIGHT/DOWN directions.
     * The bigger tstVector is, the faster the camera will scroll. */
    Point2i tstVector;
    tstVector = GetSize().inf(mousePos + sensitZone) * (mousePos + sensitZone - GetSize()) ;
    tstVector -= mousePos.inf(sensitZone) * (sensitZone - mousePos);

    if (!tstVector.IsNull()) {
      SetXY(tstVector);
      SetAutoCrop(false);
    }

    /* mouse pointer ***********************************************************/
    SaveMouseCursor();

    if (tstVector.IsNull())
      RestoreMouseCursor();
    else if (tstVector.IsXNull() && tstVector.y < 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_UP);
    else if (tstVector.IsXNull() && tstVector.y > 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_DOWN);
    else if (tstVector.IsYNull() && tstVector.x < 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_LEFT);
    else if (tstVector.IsYNull() && tstVector.x > 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_RIGHT);
    else if (tstVector.y > 0 && tstVector.x > 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_DOWN_RIGHT);
    else if (tstVector.y < 0 && tstVector.x > 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_UP_RIGHT);
    else if (tstVector.y < 0 && tstVector.x < 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_UP_LEFT);
    else if (tstVector.y > 0 && tstVector.x < 0)
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_ARROW_DOWN_LEFT);
    /***************************************************************************/
  }
}

void Camera::HandleMouseMovement()
{
  static Point2i first_mouse_pos(-1, -1);
  static Point2i last_mouse_pos(0, 0);
  Point2i curr_pos = Mouse::GetInstance()->GetPosition();

  int x,y;
  //Move camera with mouse holding Ctrl key down or with middle button of mouse
  if (SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_MIDDLE)
      || SDL_GetModState() & KMOD_CTRL) {

    // Begin to move the camera...
    if (Mouse::GetInstance()->GetPointer() != Mouse::POINTER_MOVE) {
      first_mouse_pos = Point2i(x, y);
      SaveMouseCursor();
      Mouse::GetInstance()->SetPointer(Mouse::POINTER_MOVE);
    }

    SetAutoCrop(false);
    SetXY(last_mouse_pos - curr_pos);
    last_mouse_pos = curr_pos;

    if (m_begin_controlled_move_time == 0) {
      m_begin_controlled_move_time = Time::GetInstance()->Read();
    }

    if (SDL_GetModState() & KMOD_CTRL) {
      m_control_mode = KEYBOARD_CAMERA_CONTROL;
    }else{
      m_control_mode = MOUSE_CAMERA_CONTROL;
    }
    return;

  } else if (m_control_mode == MOUSE_CAMERA_CONTROL) {

    // if the mouse has not moved at all since the user pressed the middle button, we center the camera!
    if (abs((int)first_mouse_pos.x - curr_pos.x) < 5 &&
        abs((int)first_mouse_pos.y - curr_pos.y) < 5 &&
        Time::GetInstance()->Read() - m_begin_controlled_move_time < 500) {
      CenterOnActiveCharacter();
    }

    first_mouse_pos = Point2i(-1, -1);
    RestoreMouseCursor();
    m_control_mode = NO_CAMERA_CONTROL;
    m_begin_controlled_move_time = 0;

  } else if (m_control_mode == KEYBOARD_CAMERA_CONTROL) {
    first_mouse_pos = Point2i(-1, -1);
    RestoreMouseCursor();
    m_control_mode = NO_CAMERA_CONTROL;
    m_begin_controlled_move_time = 0;
  }

  ScrollCamera();
  last_mouse_pos = curr_pos;
}

void Camera::HandleMoveIntentions()
{
  const UDMoveIntention * ud_move_intention = GetLastUDMoveIntention();
  if (ud_move_intention) {
    if (ud_move_intention->GetDirection() == DIRECTION_UP)
      SetXY(Point2i(0, -SCROLL_KEYBOARD));
    else
      SetXY(Point2i(0, SCROLL_KEYBOARD));
  }
  const LRMoveIntention * lr_move_intention = GetLastLRMoveIntention();
  if (lr_move_intention) {
    if (lr_move_intention->GetDirection() == DIRECTION_RIGHT)
      SetXY(Point2i(SCROLL_KEYBOARD, 0));
    else
      SetXY(Point2i(-SCROLL_KEYBOARD, 0));
  }
  if (lr_move_intention || ud_move_intention)
    SetAutoCrop(false);
}

void Camera::Refresh(){
  // Refresh gets called very often when the game is paused.
  // This "if" ensures that the camera doesn't move to fast.
  if (refresh_stopwatch.GetValue() >= 1000 / MAX_REFRESHES_PER_SECOND) {
    // Check if player wants the camera to move
    HandleMouseMovement();
    HandleMoveIntentions();

    if (auto_crop && followed_object != NULL)
      AutoCrop();
    refresh_stopwatch.Reset(1.0);
  }
}

void Camera::FollowObject(const PhysicalObj *obj, bool follow_closely)
{
  MSG_DEBUG("camera.tracking", "Following object %s (%d)", obj->GetName().c_str(), follow_closely);

  Mouse::GetInstance()->Hide();

  auto_crop = true;

  m_stop = !follow_closely;
  followed_object = obj;
}

void Camera::StopFollowingObj(const PhysicalObj* obj)
{
  if (followed_object == obj)
  {
    followed_object = NULL;
    m_stop = true;
    m_speed = Point2d(0,0);
  }
}

bool Camera::IsVisible(const PhysicalObj &obj) const
{
   return Intersect(obj.GetRect());
}

void Camera::CenterOnActiveCharacter()
{
  CharacterCursor::GetInstance()->FollowActiveCharacter();
  FollowObject(&ActiveCharacter(),true);
}

Point2i Camera::ComputeShake() const
{
  uint time = Time::GetInstance()->Read();
  ASSERT(time >= m_started_shaking);

  if (time > m_started_shaking + m_shake_duration || m_shake_duration == 0) {
    return Point2i(0, 0); // not shaking now
  }

  if (time == m_last_time_shake_calculated)
    return m_shake;

  // FIXME: we can underflow to 0 if time and m_started_shaking are large enough
  Double t = (Double)(time - m_started_shaking) / (Double)m_shake_duration;

  Double func_val = 1.0f;
  if (t >= EPSILON) {
    const Double k_scale_angle = 10 * PI;
    Double arg = k_scale_angle * t;
    // denormalized sinc
    func_val = (1 - t) * sin(arg) / arg;
  }

  Double x_ampl = RandomLocal().GetDouble(-m_shake_amplitude.x, m_shake_amplitude.x);
  Double y_ampl = RandomLocal().GetDouble(-m_shake_amplitude.y, m_shake_amplitude.y);

  m_shake.x = (int)(x_ampl * func_val//(Double)m_shake_amplitude.x * func_val
                    + (Double)m_shake_centerpoint.x);
  m_shake.y = (int)(y_ampl * func_val//(Double)m_shake_amplitude.y * func_val
                    + (Double)m_shake_centerpoint.y);

  static uint t_last_time_logged = 0;
  if (time - t_last_time_logged > 10) {
    MSG_DEBUG("camera.shake", "Shaking: time = %d, t = %s, func_val = %s, shake: %d, %d",
              time, Double2str(t).c_str(), Double2str(func_val).c_str(), m_shake.x, m_shake.y);
    t_last_time_logged = time;
  }

  m_last_time_shake_calculated = time;
  return m_shake;
}

void Camera::Shake(uint how_long_msec, const Point2i & amplitude, const Point2i & centerpoint)
{
  MSG_DEBUG("camera.shake", "Shake added!");

  uint time = Time::GetInstance()->Read();

  ASSERT(time >= m_started_shaking);

  if (m_started_shaking + m_shake_duration > time) {
    // still shaking, so add amplitude/centerpoint to allow shakes to combine
    m_shake_amplitude = max(m_shake_amplitude, amplitude);
    m_shake_centerpoint = centerpoint;

    // increase shake duration so it lasts how_long_msec from this time
    m_shake_duration = how_long_msec + (time - m_started_shaking);
  } else {
    // reinit the shake
    m_started_shaking = time;
    m_shake_duration = how_long_msec;
    m_shake_amplitude = amplitude;
    m_shake_centerpoint = centerpoint;
  }
}

void Camera::ResetShake()
{
  m_started_shaking = 0;
  m_shake_duration = 0;
  m_last_time_shake_calculated = 0;
  m_shake = Point2i(0, 0);
}