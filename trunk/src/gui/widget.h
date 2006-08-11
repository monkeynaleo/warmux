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
 * Widget
 *****************************************************************************/

#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include <SDL_keyboard.h>
#include "../include/base.h"
#include "../tool/rectangle.h"
#include "../tool/point.h"

#include "container.h"

class Widget : public Rectanglei
{
 protected:
  Container * ct;
  bool need_redrawing;
  void StdSetSizePosition(const Rectanglei &rect);

 public:
  bool have_focus;

  Widget();
  Widget(const Rectanglei &rect);
  virtual ~Widget();

  virtual void Update(const Point2i &mousePosition,
		      const Point2i &lastMousePosition); // virtual only for Box !!
  virtual void Draw(const Point2i &mousePosition) = 0;
  void ForceRedraw(); // set need_redrawing to true;

  virtual void SendKey(SDL_keysym key);
  virtual Widget* Clic(const Point2i &mousePosition, uint button);

  void SetContainer(Container * _ct);

  virtual void SetSizePosition(const Rectanglei &rect) = 0;
  void SetXY(int _x, int _y){ 
	  SetSizePosition( Rectanglei(Point2i(_x, _y), size) ); 
  };  
};

#endif

