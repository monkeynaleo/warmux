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
 * Null widget (blank) in GUI.
 *****************************************************************************/

#include "null_widget.h"

NullWidget::NullWidget (const Rectanglei &rect) : Widget(rect){
}

NullWidget::~NullWidget(){
}

void NullWidget::Draw(const Point2i &mousePosition, Surface& surf){
}

void NullWidget::SetSizePosition(const Rectanglei &rect){
  StdSetSizePosition(rect);
}
