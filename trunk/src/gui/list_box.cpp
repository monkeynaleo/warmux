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

#include "list_box.h"
//-----------------------------------------------------------------------------
#include "../tool/math_tools.h"
#include "../tool/resource_manager.h"
#include "../include/app.h"
#include "../graphic/font.h"
#include <algorithm>
#include <SDL_gfxPrimitives.h>
//-----------------------------------------------------------------------------

ListBox::ListBox (uint _x, uint _y, uint _w, uint _h)
  : Widget(_x,_y,_w,_h) 
{  
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml"); 
  m_up = new Button(x+w-10, y, 10, 5, res, "menu/up");
  m_down = new Button(x+w-10, y+h-5, 10, 5, res, "menu/down");

  height_item = small_font.GetHeight();
  first_visible_item = 0;
  nb_visible_items_max = h/height_item;
  nb_visible_items = 0;
  selection_min = 1;
  selection_max = 1;
}

//-----------------------------------------------------------------------------

ListBox::~ListBox()
{
   delete m_up;
   delete m_down;

   m_selection.clear();
   m_items.clear();
}

//-----------------------------------------------------------------------------

int ListBox::MouseIsOnWitchItem (uint mouse_x, uint mouse_y)
{
  if ((mouse_x < x+1) 
      || (mouse_y < y+1)
      || ((y+1 + h) < mouse_y)
      || ((x + w) < mouse_x)) 
    return -1;

  int index = (mouse_y - y)/height_item;
  return BorneLong(index+first_visible_item, 0, m_items.size()-1);
}

//-----------------------------------------------------------------------------

bool ListBox::Clic (uint mouse_x, uint mouse_y)
{

  // buttons for listbox with more items than visible
  if (m_items.size() > nb_visible_items_max)
  {
    if ( m_down->MouseIsOver(mouse_x, mouse_y) )
    {
      // bottom button
      if ( m_items.size()-1 - first_visible_item > nb_visible_items_max ) first_visible_item++ ;
      return true;
    }

    
    if ( m_up->MouseIsOver(mouse_x,mouse_y) )
    {
      // top button
      if (first_visible_item > 0) first_visible_item-- ;
      return true;
    }
  }

  int item = MouseIsOnWitchItem(mouse_x,mouse_y);
  if (item == -1) return false;
  if (IsSelected(item))
    Deselect (item);
  else
    Select (item);
  return true;
}

//-----------------------------------------------------------------------------

void ListBox::Draw (uint mouse_x, uint mouse_y)
{
  int item = MouseIsOnWitchItem(mouse_x, mouse_y);
  
  boxRGBA(app.sdlwindow, x, y, x+w, y+h,
	  255, 255, 255, 255*3/10);

  rectangleRGBA(app.sdlwindow, x, y, x+w, y+h,
		255, 255, 255, 255);

  for (uint i=0; i < nb_visible_items; i++) 
  {
     if ( i+first_visible_item == uint(item) ) {
       boxRGBA(app.sdlwindow, 
	       x+1, y+i*height_item+1, 
	       x+1+w-2, y+i*height_item+1+height_item-2,
	       0,0,255*6/10,255*4/10);
       
     } else if ( IsSelected(i+first_visible_item) ) {
       boxRGBA(app.sdlwindow, 
	       x+1, y+i*height_item+1, 
	       x+1+w-2, y+i*height_item+1+height_item-2,
	       0,0,255*6/10,255*8/10);
     }
     
     small_font.WriteLeft(x+5,
			  y+i*height_item,
			  m_items[i+first_visible_item].label,
			  white_color);//IsSelected(i+first_visible_item) ? black_color : white_color) ;
     
  }  


  // buttons for listbox with more items than visible
  if (m_items.size() > nb_visible_items_max)
  {
    m_up->Draw (mouse_x, mouse_y);
    m_down->Draw (mouse_x, mouse_y);
  }
  
  
}

//-----------------------------------------------------------------------------

void ListBox::SetSizePosition(uint _x, uint _y, uint _w, uint _h)
{
  StdSetSizePosition(_x, _y, _w, _h);
  m_up->SetSizePosition(x+w-10, y, 10, 5);
  m_down->SetSizePosition(x+w-10, y+h-5, 10, 5);  

  nb_visible_items_max = h/height_item;
}

//-----------------------------------------------------------------------------

void ListBox::AddItem (bool selected, 
		       const std::string &label,
		       const std::string &value) 
{ 
  uint pos = m_items.size();

  // Push item
  list_box_item_t item;
  item.label = label;
  item.value = value;
  m_items.push_back (item);

  // Select it if selected
  if (selected) Select (pos);

  nb_visible_items = m_items.size();
  if (nb_visible_items_max < nb_visible_items) 
    nb_visible_items = nb_visible_items_max;
}

//-----------------------------------------------------------------------------

void ListBox::Select (uint index)
{
  // If they are to much selection, kick the oldest one
  if (selection_max != -1)
  {
    if ((int)m_selection.size() == selection_max) 
      m_selection.erase(m_selection.begin());
    assert ((int)m_selection.size() < selection_max);
  }

  // Add new selection
  m_selection.push_back (index);
}

//-----------------------------------------------------------------------------

void ListBox::Deselect (uint index)
{
  if ((int)m_selection.size()-1 < selection_min) return;
  m_selection.remove (index);
}

//-----------------------------------------------------------------------------

bool ListBox::IsSelected (uint index)
{
  return std::find (m_selection.begin(), m_selection.end(), index) 
    != m_selection.end();
}

//-----------------------------------------------------------------------------

uint ListBox::GetSelectedItem ()
{
  assert (m_selection.size() == 1);
  return m_selection.front();
}

//-----------------------------------------------------------------------------

const std::list<uint>& ListBox::GetSelection() const { return m_selection; }
const std::string& ListBox::ReadLabel (uint index) const
{
  assert (index < m_items.size());
  return m_items[index].label;
}
const std::string& ListBox::ReadValue (uint index) const
{
  assert (index < m_items.size());
  return m_items[index].value;
}

//-----------------------------------------------------------------------------
