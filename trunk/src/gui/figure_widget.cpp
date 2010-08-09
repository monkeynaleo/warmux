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
 * PictureWidget having captions
 *****************************************************************************/

#include "graphic/text.h"
#include "gui/figure_widget.h"

void FigureWidget::Draw(const Point2i& mousePosition) const
{
  Point2f zoom = GetScale();

  PictureWidget::Draw(mousePosition);

  uint lsize = 130*zoom.GetX() + 0.5;

  for (uint i=0; i<captions.size(); i++) {
    Text tmp(captions[i].string,
             captions[i].color,
             captions[i].fsize*powf(zoom.GetX(), 0.75f),
             captions[i].fstyle,
             false);
    tmp.SetMaxWidth(lsize);
    Point2i pos = Point2i(zoom.x * captions[i].x, zoom.y * captions[i].y)
                + GetPicturePosition();

    tmp.DrawCenter(pos);
  }
}

