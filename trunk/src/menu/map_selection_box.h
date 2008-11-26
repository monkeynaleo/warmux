/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Map selection box
 *****************************************************************************/

#ifndef MAP_SELECTION_BOX_H
#define MAP_SELECTION_BOX_H

#include "graphic/surface.h"
#include "gui/box.h"
#include "include/base.h"
#include <WORMUX_point.h>
#include <WORMUX_rectangle.h>

// Forward declarations
class Button;
class Label;
class PictureWidget;

class MapSelectionBox : public VBox
{
 private:
  /* If you need this, implement it (correctly) */
  MapSelectionBox(const MapSelectionBox&);
  MapSelectionBox operator=(const MapSelectionBox&);
  /**********************************************/

  uint selected_map_index;
  bool display_only;

  PictureWidget *map_preview_selected;
  PictureWidget *map_preview_before, *map_preview_before2;
  PictureWidget *map_preview_after, *map_preview_after2;

  Label *map_name_label;
  Label *map_author_label;
  Button *bt_map_plus, *bt_map_minus;
  Surface random_map_preview;

  void ChangeMap(uint index);
  void UpdateMapInfo(PictureWidget * widget, uint index, bool selected);
  void UpdateRandomMapInfo(PictureWidget * widget, bool selected);

 public:
  void ChangeMapDelta(int delta_index);
  MapSelectionBox(const Point2i &size, bool _display_only = false);

  void ValidMapSelection();
  void ChangeMapCallback();

  virtual Widget* Click(const Point2i &mousePosition, uint button);
  virtual Widget* ClickUp(const Point2i &mousePosition, uint button);
  virtual void Pack();
};


#endif
