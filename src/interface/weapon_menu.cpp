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
 * Interface affichant différentes informations sur la jeu.
 *****************************************************************************/

#include "weapon_menu.h"
//-----------------------------------------------------------------------------
#include <sstream>
#include <math.h>
#include "interface.h"
#include "../map/camera.h"
#include "../team/teams_list.h"
#include "../tool/string_tools.h"
#include "../weapon/weapons_list.h"
#include "../weapon/weapon.h"
#include "../graphic/video.h"
#include "../game/time.h"
#include "../team/team.h"
#include "../include/action_handler.h"
#include "../interface/mouse.h"
#include "../tool/Point.h"
#include "../tool/Rectangle.h"
#include "../tool/Distance.h"
#include "../include/app.h"
#include "../tool/resource_manager.h"
#include "../graphic/sprite.h"

using namespace Wormux;
//-----------------------------------------------------------------------------

// Weapon menu
const uint BUTTON_ICO_WIDTH = 58;  // Width of the button icon
const uint BUTTON_ICO_HEIGHT = 58; // Height of the button icon

const uint WEAPON_ICO_WIDTH = 48;   // Width of the weapon icon
const uint WEAPON_ICO_HEIGHT = 48;  // Height of the button icon

const uint BUTTON_ICO_GAP = 8 ; // Gap between buttons when a button is zoomed
                                   

const uint ICONS_DRAW_TIME = 600; // Time to display all icons (in ms)
const uint ICON_ZOOM_TIME = 150; // Time to zomm one icon.

const double DEFAULT_ICON_SCALE = 0.7 ;
const double MAX_ICON_SCALE = 1.1 ;

const uint BUTTON_WIDTH = (int)(BUTTON_ICO_GAP + BUTTON_ICO_WIDTH  *
				(DEFAULT_ICON_SCALE+MAX_ICON_SCALE)/2);

const uint BUTTON_HEIGHT = (int)(BUTTON_ICO_GAP + BUTTON_ICO_HEIGHT  *
				 (DEFAULT_ICON_SCALE+MAX_ICON_SCALE)/2);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
WeaponMenuItem::WeaponMenuItem(uint num_sort)
{
  weapon_type = num_sort;
  Reset();
}

//-----------------------------------------------------------------------------
void WeaponMenuItem::Reset()
{
  scale = DEFAULT_ICON_SCALE;
  zoom = false;
  dezoom = false;
}

//-----------------------------------------------------------------------------
void WeaponMenuItem::ChangeZoom()
{
  zoom_start_time = temps.Lit();

  if(!zoom && scale < 1)
    {
      zoom = true;
      dezoom = false;
    }
  else
    {
      zoom = false;
      dezoom = true;
    }
}

//-----------------------------------------------------------------------------
void WeaponMenuItem::ComputeScale()
{
  double scale_range, time_range ;

  time_range = ((double)temps.Lit() - zoom_start_time) / ICON_ZOOM_TIME ;
  if (time_range > 1)
    time_range = 1 ;

  scale_range = sin (time_range * M_PI / 2) * (MAX_ICON_SCALE - DEFAULT_ICON_SCALE) ;

  if(zoom)
  {
    scale = DEFAULT_ICON_SCALE + scale_range ;

    if(time_range == 1)
      zoom = false;
  }
  else
    if(dezoom)
      {
	scale = MAX_ICON_SCALE - scale_range ;

	if(time_range == 1)
	  dezoom = false;
      }
}

//-----------------------------------------------------------------------------
bool WeaponMenuItem::MouseOn(int s_x, int s_y)
{
  ComputeScale();
  int scaled_width = (int)(BUTTON_ICO_WIDTH * scale) ;
  int scaled_height = (int)(BUTTON_ICO_HEIGHT * scale) ;

#ifdef CL
  CL_Rect rect;
  rect.left = interface.weapons_menu.GetX() + x - (int)(0.5 * scaled_width) ;
  rect.top = interface.weapons_menu.GetY() + y - (int)(0.5 * scaled_height) ;
  rect.right = rect.left + scaled_width ;
  rect.bottom = rect.top + scaled_height ;
#else
  Rectanglei rect;
  rect.x = interface.weapons_menu.GetX() + x - (int)(0.5 * scaled_width);
  rect.w = scaled_width;
  rect.y = interface.weapons_menu.GetY() + y - (int)(0.5 * scaled_height) ;
  rect.h = scaled_height;;
#endif
   
#ifdef CL
  CL_Point point;
#else
  Point2i point;
#endif
   
  point.x = s_x;
  point.y = s_y;

#ifdef CL
   if (rect.is_inside(point))
#else
   if ( IsInside( rect, point))  
#endif
     return true;
  else
    {
      if(scale > DEFAULT_ICON_SCALE && !dezoom)
	dezoom = true;
      return false;
    }
}

//-----------------------------------------------------------------------------
// Draw a button
void WeaponMenuItem::Draw()
{
  ComputeScale();
  int c_x = interface.weapons_menu.GetX() + x; // (c_x, cy) : center of button
  int c_y = interface.weapons_menu.GetY() + y;
  std::ostringstream txt;
  int nb_bullets ;

#ifdef CL
  CL_Surface button ;
#else
  Sprite *button;
#endif
   
  switch (weapon_type) {
    case 1:
      button = interface.weapons_menu.my_button1 ;
      break ;

    case 2:
      button = interface.weapons_menu.my_button2 ;
      break ;

    case 3:
      button = interface.weapons_menu.my_button3 ;
      break ;

    case 4:
      button = interface.weapons_menu.my_button4 ;
      break ;

    case 5:
      button = interface.weapons_menu.my_button5 ;
      break ;

  default:
      button = interface.weapons_menu.my_button1 ;
      break ;
  }

  // Button display

#ifdef CL
   button.set_scale(scale, scale);
   button.draw((int)(c_x - 0.5 * BUTTON_ICO_WIDTH * scale),
	      (int)(c_y - 0.5 * BUTTON_ICO_HEIGHT * scale));
   
#else
  
   button->Scale( scale, scale);
   button->Blit( app.sdlwindow, 
		 (int)(c_x - 0.5 * BUTTON_ICO_WIDTH * scale), 
		 (int)(c_y - 0.5 * BUTTON_ICO_HEIGHT * scale));	
#endif
   
  // Weapon display

#ifdef CL
  CL_Surface icon = weapon->icone;

  icon.set_scale(scale, scale);

  icon.draw((int)(c_x - 0.5 * WEAPON_ICO_WIDTH * scale),
	    (int)(c_y - 0.5 * WEAPON_ICO_HEIGHT * scale));
#else
  weapon_icon->Scale( scale, scale);
  weapon_icon->Blit( app.sdlwindow,
		     (int)(c_x - 0.5 * WEAPON_ICO_WIDTH * scale),
		     (int)(c_y - 0.5 * WEAPON_ICO_HEIGHT * scale));
#endif
   
  // Amunitions display
  nb_bullets = ActiveTeam().ReadNbAmmos(weapon->GetName());
  txt.str ("");
  if (nb_bullets ==  INFINITE_AMMO)
    txt << ("§");
  else
    txt << nb_bullets;
#ifdef CL
  police_weapon.Acces().set_scale(0.4+scale,0.4+scale);
  police_weapon.WriteLeftBottom ((int)(c_x - 0.5 * WEAPON_ICO_WIDTH * scale),
				(int)(c_y + 0.5 * WEAPON_ICO_HEIGHT * scale),
				 txt.str());
#else
// TODO
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
WeaponsMenu::WeaponsMenu()
{
  display = false;
  show = false;
  hide = false;
  nbr_weapon_type = 0;
}

//-----------------------------------------------------------------------------
// Add a new weapon to the weapon menu.
void WeaponsMenu::NewItem(Weapon* new_item, uint num_sort)
{
  WeaponMenuItem item(num_sort);
  item.x = 0;
  item.y = 0;
  item.weapon = new_item;
#ifndef CL
  item.weapon_icon = new Sprite( new_item->icone);
#endif
  boutons.push_back (item);

  if(num_sort>nbr_weapon_type)
    nbr_weapon_type = num_sort;
}

//-----------------------------------------------------------------------------
// Weapon menu display (init of the animation)
void WeaponsMenu::Show()
{
  if(hide)
    motion_start_time = temps.Lit() - (ICONS_DRAW_TIME - (temps.Lit()-motion_start_time));
  else
    motion_start_time = temps.Lit();

  display = true;
  show = true;
  hide = false;
}

//-----------------------------------------------------------------------------
// Compute maximum number of icons in weapon menu columns.
void WeaponsMenu::ComputeSize()
{
  max_weapon = 0;
  uint nbr_current_type = 0;

  iterator it=boutons.begin(), fin=boutons.end();
  for (; it != fin; ++it)
  {
    if(it != boutons.begin())
    if(((it-1)->weapon_type) != (it->weapon_type))
    {
      if(nbr_current_type > max_weapon)
        max_weapon = nbr_current_type;
      nbr_current_type = 0;
    }
    if(ActiveTeam().ReadNbAmmos(it->weapon->GetName())>0
       || ActiveTeam().ReadNbAmmos(it->weapon->GetName())==INFINITE_AMMO)
      nbr_current_type++;
  }

  if(nbr_current_type > max_weapon)
    max_weapon = nbr_current_type;
}

//-----------------------------------------------------------------------------
void WeaponsMenu::Hide()
{
  if(show)
    motion_start_time = temps.Lit() - (ICONS_DRAW_TIME - (temps.Lit()-motion_start_time));
  else
    motion_start_time = temps.Lit();

  hide = true;
  show = false;
}

//-----------------------------------------------------------------------------
void WeaponsMenu::ChangeAffichage()
{
  if(display && !hide)
    Hide();
  else
    Show();
}

//-----------------------------------------------------------------------------
int WeaponsMenu::GetX() const
{
  return video.GetWidth()-GetWidth();
}

//-----------------------------------------------------------------------------
int WeaponsMenu::GetY() const 
{
#ifdef CL
  int y = video.GetHeight();

  if (interface.EstAffiche())
    y -= interface.GetHeight();
  return y;
#else
   return video.GetHeight() - GetHeight() - ( interface.EstAffiche() ? interface.GetHeight() : 0 );
#endif
}

//-----------------------------------------------------------------------------
int WeaponsMenu::GetWidth() const
{
  return BUTTON_ICO_GAP + ((nbr_weapon_type +1) * BUTTON_WIDTH) ;
}

//-----------------------------------------------------------------------------
int WeaponsMenu::GetHeight() const
{
#ifdef CL
   return -((BUTTON_ICO_GAP + BUTTON_HEIGHT * max_weapon));
#else
   return BUTTON_ICO_GAP + BUTTON_HEIGHT * max_weapon;
#endif
}

//-----------------------------------------------------------------------------
bool WeaponsMenu::IsDisplayed() const
{
  return display;
}

//-----------------------------------------------------------------------------
void WeaponsMenu::Init()
{
#ifdef CL
  CL_ResourceManager* res=graphisme.LitRes();

  my_button1 = CL_Surface("interface/button1_icon", res);
  my_button2 = CL_Surface("interface/button2_icon", res);
  my_button3 = CL_Surface("interface/button3_icon", res);
  my_button4 = CL_Surface("interface/button4_icon", res);
  my_button5 = CL_Surface("interface/button5_icon", res);
#else
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");
  my_button1 = new Sprite( resource_manager.LoadImage(res,"interface/button1_icon"));
  my_button2 = new Sprite( resource_manager.LoadImage(res,"interface/button2_icon"));
  my_button3 = new Sprite( resource_manager.LoadImage(res,"interface/button3_icon"));
  my_button4 = new Sprite( resource_manager.LoadImage(res,"interface/button4_icon"));
  my_button5 = new Sprite( resource_manager.LoadImage(res,"interface/button5_icon"));
#endif
}

//-----------------------------------------------------------------------------
void WeaponsMenu::ShowMotion(int nr_buttons,int button_no,iterator it,int column)
{
  int delta_t=ICONS_DRAW_TIME/(2*nr_buttons);

  if((temps.Lit() > motion_start_time + (delta_t*button_no))
     && (temps.Lit() < motion_start_time + (ICONS_DRAW_TIME/2)+(delta_t*(button_no))))
    {
      double delta_sin = -(asin((column+1.0)/(column+2.0)) - (M_PI/2));
      
      uint tps = temps.Lit() - (motion_start_time + delta_t*button_no);
      
      double tps_sin = ((double)tps * ((M_PI/2) + delta_sin)/(ICONS_DRAW_TIME/2));
      
      it-> x -= (int)(sin(tps_sin) * BUTTON_WIDTH * (column+2.0));
      it-> x += (BUTTON_WIDTH * (column+1));
    }
  else
    if(temps.Lit() < motion_start_time + (delta_t*button_no))
      {
	it-> x += (BUTTON_WIDTH * (column+1));
      }
  
  if(temps.Lit() > motion_start_time + ICONS_DRAW_TIME)
    {
      show = false;
    }
}

//-----------------------------------------------------------------------------

bool WeaponsMenu::HideMotion(int nr_buttons,int button_no,iterator it,int column)
{
  int delta_t=ICONS_DRAW_TIME/(2*nr_buttons);

  if((temps.Lit() > motion_start_time + (delta_t*(nr_buttons-button_no)))
     && (temps.Lit() < motion_start_time + (ICONS_DRAW_TIME/2)+(delta_t*(nr_buttons-button_no))))
    {
      double delta_sin = -(asin((column+1.0)/(column+2.0)) - (M_PI/2));
      
      uint tps = temps.Lit() - (motion_start_time + delta_t*(nr_buttons-button_no));
      double tps_sin = ((double)tps * ((M_PI/2) + delta_sin)/(ICONS_DRAW_TIME/2));
      tps_sin = ((M_PI/2) + delta_sin) - tps_sin;
      
      it-> x -= (int)(sin(tps_sin) * BUTTON_WIDTH * (column+2.0));
      it-> x += BUTTON_WIDTH * (column+1);
    }
  else
    if(temps.Lit() > motion_start_time + (delta_t*(nr_buttons-button_no)))
      {
	it-> x += BUTTON_WIDTH * (column+1);
	it-> Reset();
      }
  
  if(temps.Lit() > motion_start_time + ICONS_DRAW_TIME)
    {
      hide = false;
      display = false;
      return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
void WeaponsMenu::Draw()
{
  if (!display)
    return;

  int mouse_x = mouse.GetPosMonde().x - camera.GetX();
  int mouse_y = mouse.GetPosMonde().y - camera.GetY();

  ReactionSouris(mouse_x,mouse_y);
  ComputeSize();
   
  uint nr_buttons = max_weapon * nbr_weapon_type;
  uint button_no = 0;
  uint current_type = 0;

  iterator it=boutons.begin(), fin=boutons.end();
  for (it=boutons.begin(); it != fin; ++it)
  {
    if(ActiveTeam().ReadNbAmmos(it->weapon->GetName())<=0
       && ActiveTeam().ReadNbAmmos(it->weapon->GetName())!=INFINITE_AMMO)
      continue;

    if(it->weapon_type!=current_type)
    {
      button_no = 0;
      current_type = it->weapon_type;
    }

    int column = nbr_weapon_type - current_type;
    int row = button_no;

    it->x = GetWidth() - (int)(BUTTON_WIDTH * (column+0.5));
#ifdef CL
    it->y = GetHeight() + BUTTON_ICO_GAP + (row * BUTTON_HEIGHT);
#else
    it->y = BUTTON_ICO_GAP + (row * BUTTON_HEIGHT);     
#endif
     
    if(show)
    {
      ShowMotion(nr_buttons,(column * max_weapon) + row,it,column);
    }
    else
    if(hide)
    {
      if(HideMotion(nr_buttons,(column * max_weapon) + row,it,column))
        return;
    }

    it->Draw();
    button_no++;
  }
}

//-----------------------------------------------------------------------------
void WeaponsMenu::ReactionSouris (int x, int y)
{
  static int bouton_sous_souris = -1; //bouton survolé par la souris
 
#ifdef CL
  CL_Rect rect;

  rect.left = GetX();
  rect.top = GetY() + GetHeight() - (int)(0.5 * BUTTON_ICO_HEIGHT);
  rect.right = GetX() + GetWidth();
  rect.bottom = GetY();

  CL_Point point;
  point.x = x;
  point.y = y;

  if (rect.is_inside(point))
#else
  // Nothing to do
#endif  
  {
    //Bouton en cour d'analyse:
    int button_no=0;
    int nv_bouton_sous_souris=-1;
    iterator it=boutons.begin(), fin=boutons.end();
    interface.arme_pointe_souris = NULL;
    for (; it != fin; ++it)
    {
      if(it->MouseOn(x,y))
      {
          interface.arme_pointe_souris = it->weapon;
        nv_bouton_sous_souris = button_no;
        if(button_no != bouton_sous_souris)
        {//Le curseur arrive sur un nouveau bouton
          it->ChangeZoom();
        }
      }
      else
      if(button_no == bouton_sous_souris)
      {
        it->ChangeZoom();
      }
      button_no++;
    }
    bouton_sous_souris = nv_bouton_sous_souris;
  }
#ifdef CL
  else
  {
    iterator it=boutons.begin();
    if(bouton_sous_souris != -1)
    {
      (it+bouton_sous_souris)->ChangeZoom();
    }
    bouton_sous_souris = -1;
    interface.arme_pointe_souris = NULL;
  }
#else
  // Nothing to do
#endif
}

//-----------------------------------------------------------------------------

bool WeaponsMenu::ActionClic (int x, int y)
{
#ifdef CL
  CL_Rect rect;
#endif
  if (!display) return false;

  iterator it=boutons.begin(), fin=boutons.end();
  for (; it != fin; ++it)
  {
    if (it->MouseOn(x,y))
    {
	  action_handler.NewAction (ActionInt(
	    ACTION_CHANGE_WEAPON, 
		it -> weapon -> GetType()));
      ChangeAffichage();
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
