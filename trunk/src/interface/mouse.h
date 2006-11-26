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
    POINTER_AIM
  } pointer_t;

private:
  bool scroll_actif;
  bool hide;
  pointer_t current_pointer;

  Point2i savedPos;
  Point2i lastPos;

  static Mouse * singleton;

  Surface pointer_select, pointer_move, pointer_aim;

  Mouse();
  bool ScrollPointer();
  bool DrawMovePointer();

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
  void ScrollCamera();

  // Choose the pointer
  void SetPointer(pointer_t pointer);
  void Draw();

  // Hide/show mouse pointer
  void Show();
  void Hide();
  bool IsVisible() const;

  // Center the pointer on the screen
  void CenterPointer();
};

#endif
