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

#include "../include/base.h"
//-----------------------------------------------------------------------------

class Widget
{
 protected:
  int x, y;
  uint w, h;
  void StdSetSizePosition(int _x, int _y, uint _w, uint _h);

 public:
  Widget(int _x, int _y, uint _w, uint _h);
  virtual ~Widget();

  virtual void Draw (uint mouse_x, uint mouse_y) = 0;

  bool MouseIsOver (uint mouse_x, uint mouse_y) ;
  virtual bool Clic (uint mouse_x, uint mouse_y, uint button);

  virtual void SetSizePosition(int _x, int _y, uint _w, uint _h) = 0;
  void SetSize (uint _w, uint _h) { SetSizePosition(x, y, _w, _h); };
  void SetXY   (int  _x, int  _y) { SetSizePosition(_x, _y, w, h); };

  int GetX() const {return x;}
  int GetY() const {return y;}
  uint GetW() const {return w;}
  uint GetH() const {return h;}
};

//-----------------------------------------------------------------------------
#endif

