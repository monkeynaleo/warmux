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
 * Liste de choix.
 *****************************************************************************/

#ifndef LIST_BOX_H
#define LIST_BOX_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "widget.h"
#include "button.h"
#include <vector>
#include <string>
#include <list>
//-----------------------------------------------------------------------------

struct SDL_Surface;

typedef struct s_list_box_item_t{
    std::string label;
    std::string value;
  } list_box_item_t;


class ListBox : public Widget
{
public:
  int selection_min; // Minimal number of selected items
  int selection_max; // Maximal number of selected items (-1 means no limit)
  typedef std::list<uint>::const_iterator selection_iterator;

private:
  // for the placement
  uint x, y;
  uint visible_height; 
  uint nb_visible_items, nb_visible_items_max;
  uint height_item;
  
  // what are the items ?
  uint first_visible_item;
  std::vector<list_box_item_t> m_items;

  //std::vector<ListBoxItem> m_items;
  std::list<uint> m_selection;

  // Buttons
  Button *m_up, *m_down;
  SDL_Surface *cursorover_box;
  SDL_Surface *selected_box;
  SDL_Surface *background;
public:
  ListBox (uint _x, uint _y, uint _width, uint _height);
  ~ListBox();

  void Draw (uint mouse_x, uint mouse_y);
  bool Clic (uint mouse_x, uint mouse_y);
  void AddItem (bool selected, 
		const std::string &label,
		const std::string &value);
  int MouseIsOnWitchItem (uint mouse_x, uint mouse_y);
  void Select (uint index);
  void Deselect (uint index);
  bool IsSelected (uint index);
  uint GetSelectedItem ();
  const std::list<uint>& GetSelection() const;
  const std::string& ReadLabel (uint index) const;
  const std::string& ReadValue (uint index) const;
};

//-----------------------------------------------------------------------------
#endif
