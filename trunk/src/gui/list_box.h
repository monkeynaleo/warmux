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

#include <string>
#include <vector>
#include <SDL.h>
#include "widget.h"
#include "button.h"
#include "../include/base.h"

typedef struct s_list_box_item_t{
    std::string label;
    std::string value;
    bool enabled;
} list_box_item_t;

class ListBox : public Widget
{ 
public:
  bool always_one_selected;
  
private:
  // for the placement
  uint nb_visible_items, nb_visible_items_max;
  uint height_item;
  
  // what are the items ?
  uint first_visible_item;
  int selected_item; 
  std::vector<list_box_item_t> m_items;

  // Buttons
  Button *m_up, *m_down;

public:
  ListBox (const Rectanglei &rect);
  ~ListBox();

  void Draw(const Point2i &mousePosition);
  Widget* Clic(const Point2i &mousePosition, uint button);
  void SetSizePosition(const Rectanglei &rect);

  void AddItem(bool selected, const std::string &label,
		const std::string &value, bool enabled = true);
  void Sort();

  int MouseIsOnWhichItem(const Point2i &mousePosition);

  void Select(uint index);
  void Select(const std::string& val);
  int GetSelectedItem();
  void Deselect();
  void RemoveSelected();
  const std::string& ReadLabel() const;
  const std::string& ReadValue() const;
  const std::string& ReadValue(int index) const;

  std::vector<list_box_item_t> *GetItemsList();
};

#endif
