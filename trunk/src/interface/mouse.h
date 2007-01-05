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
 * Mouse management
 *****************************************************************************/

#ifndef MOUSE_H
#define MOUSE_H

#include <SDL.h>
#include "../graphic/surface.h"
#include "../include/app.h"
#include "../include/base.h"
#include "../tool/point.h"

class Mouse
{
public:
  typedef enum {
    POINTER_STANDARD,
    POINTER_SELECT,
    POINTER_MOVE,
    POINTER_ARROW_UP,
    POINTER_ARROW_UP_RIGHT,
    POINTER_ARROW_UP_LEFT,
    POINTER_ARROW_DOWN,
    POINTER_ARROW_DOWN_RIGHT,
    POINTER_ARROW_DOWN_LEFT,
    POINTER_ARROW_RIGHT,
    POINTER_ARROW_LEFT,
    POINTER_AIM
  } pointer_t;

private:
  bool scroll_actif;
  bool hide;
  pointer_t current_pointer;

  Point2i savedPos;
  Point2i lastPos;

  static Mouse * singleton;

  Surface pointer_select, 
    pointer_move, 
    pointer_arrow_up,
    pointer_arrow_up_right,
    pointer_arrow_up_left,
    pointer_arrow_down,
    pointer_arrow_down_right,
    pointer_arrow_down_left,
    pointer_arrow_right,
    pointer_arrow_left,
    pointer_aim;

  Mouse();
  pointer_t ScrollPointer() const;
  bool DrawMovePointer();
  void ScrollCamera() const;

  const Surface& GetSurfaceFromPointer(pointer_t pointer) const;
public:

  static Mouse * GetInstance();

  void TraiteClic (const SDL_Event *event);

  void Reset();

  void Refresh();
  void TestCamera();
  bool ActionClicG();
  bool ActionClicD();
  bool ActionWhellDown();
  bool ActionWhellUp();
  void ChoixVerPointe();

  Point2i GetPosition() const;
  Point2i GetWorldPosition() const;
  bool ClicG() const;
  bool ClicD() const;
  bool ClicM() const;

  // Choose the pointer
  pointer_t SetPointer(pointer_t pointer);
  void Draw();

  // Hide/show mouse pointer
  void Show();
  void Hide();
  bool IsVisible() const;

  // Center the pointer on the screen
  void CenterPointer();
};

#endif
