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

#ifndef GUI_FIGURE_WIDGET_H
#define GUI_FIGURE_WIDGET_H

#include "picture_widget.h"

class FigureWidget : public PictureWidget
{
public:
  typedef struct
  {
    std::string string; // gcc does not support correctly a char* here.
    int x, y;
  } Caption;

private:
  const Caption *captions;
  uint           font_size;

public:
  // caps must be static const, as we aren't doing copies
  FigureWidget(const Point2i & size,
               const std::string & resource_id,
               const Caption *caps,
               uint  fsize = 12,
               ScalingType type = FIT_SCALING)
    : PictureWidget(size, resource_id, type)
    , captions(caps)
    , font_size(fsize)
  { }

  virtual void Draw(const Point2i& mousePosition) const;
};

#endif
