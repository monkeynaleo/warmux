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

#include "interface.h"
#include <iostream>
#include <SDL.h>
#include <sstream>
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../game/time.h"
#include "../graphic/colors.h"
#include "../graphic/video.h"
#include "../include/app.h"
#include "../include/global.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../tool/string_tools.h"
#include "../tool/i18n.h"
#include "../tool/resource_manager.h"
#include "../weapon/weapons_list.h"

WeaponStrengthBar weapon_strength_bar;


// Nom du ver
const uint NOM_VER_X = 32;
const uint NOM_VER_Y = 28;

// Energie du ver
const uint ENERGIE_VER_X = NOM_VER_X;
const uint ENERGIE_VER_Y = NOM_VER_Y+20;

// Nom de l'arme
const uint NOM_ARME_X = 508;
const uint NOM_ARME_Y = NOM_VER_Y;

// Munitions
const uint MUNITION_X = NOM_ARME_X;
const uint MUNITION_Y = ENERGIE_VER_Y;

// Ecusson de l'équipe
const uint ECUSSON_EQUIPE_X = 303;
const uint ECUSSON_EQUIPE_Y = 20;

// Icône de l'arme
const uint ICONE_ARME_X = 450;
const uint ICONE_ARME_Y = 20;

const uint INFO_VER_X2 = 296;

// Barre d'énergie
const uint BARENERGIE_X = 170;
const uint BARENERGIE_Y = ENERGIE_VER_Y+2;
const uint BARENERGIE_LARG = INFO_VER_X2-BARENERGIE_X;
const uint BARENERGIE_HAUT = 15;

const uint MARGIN = 10;

Interface interface;

Interface::Interface()
{
}

void Interface::Init()
{
  display = true;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  game_menu = resource_manager.LoadImage( res, "interface/menu_jeu");
  bg_time = resource_manager.LoadImage( res, "interface/fond_compteur");
  weapons_menu.Init();
  weapon_box_button = resource_manager.LoadImage( res, "interface/weapon_box_button");
   
  barre_energie.InitVal (0, 0, game_mode.character.init_energy);
  barre_energie.InitPos (BARENERGIE_X, BARENERGIE_Y, 
			 BARENERGIE_LARG, BARENERGIE_HAUT);
  barre_energie.border_color = white_color;
  barre_energie.value_color = lightgray_color;
  barre_energie.background_color = gray_color;  
   
  // strength bar initialisation  
  weapon_strength_bar.InitPos (0, 0, 400, 10);
  weapon_strength_bar.InitVal (0, 0, 100);

  weapon_strength_bar.SetValueColor(WeaponStrengthBarValue);
  weapon_strength_bar.SetBorderColor(WeaponStrengthBarBorder);
  weapon_strength_bar.SetBackgroundColor(WeaponStrengthBarBackground);

  // constant text initialisation
  t_NAME = new Text( _("Name:"), white_color, &global().normal_font());
  t_ENERGY = new Text( _("Energy:"),white_color, &global().normal_font());
  t_WEAPON = new Text( _("Weapon:"), white_color, &global().normal_font());
  t_STOCK = new Text( _("Stock:"), white_color, &global().normal_font());

  global_timer = new Text(ulong2str(0), white_color, &global().normal_font());
  timer = NULL;

  t_character_name = new Text("None", white_color, &global().normal_font());
  t_character_energy = new Text("Dead", white_color, &global().normal_font());
  t_weapon_name = new Text("None", white_color, &global().normal_font());
  t_weapon_stock = new Text("0", white_color, &global().normal_font());;
}

Interface::~Interface()
{
  delete t_NAME;
  delete t_ENERGY;
  delete t_WEAPON;
  delete t_STOCK;

  if (global_timer) delete global_timer;
  if (timer) delete timer;
  if (t_character_name) delete t_character_name;
  if (t_character_energy) delete t_character_energy;
  if (t_weapon_name) delete t_weapon_name;
  if (t_weapon_stock) delete t_weapon_stock;
}

void Interface::Reset()
{
  character_under_cursor = NULL;
  weapon_under_cursor = NULL;
  weapons_menu.Reset();
  barre_energie.InitVal (0, 0, game_mode.character.init_energy);
}

void Interface::DisplayCharacterInfo ()
{

  int x = (app.video.window.GetWidth() - GetWidth())/2;
  int y = app.video.window.GetHeight() - GetHeight();

  // Get the character
  if (character_under_cursor == NULL) character_under_cursor = &ActiveCharacter();

  // Display name
  t_NAME->DrawTopLeft(bottom_bar_ox+NOM_VER_X, 
		      bottom_bar_oy+NOM_VER_Y);

  std::string s(character_under_cursor->m_name+" ("+character_under_cursor->GetTeam().GetName()+" )");
  t_character_name->Set(s);

  t_character_name->DrawTopLeft(bottom_bar_ox+NOM_VER_X+t_NAME->GetWidth()+MARGIN,
				bottom_bar_oy+NOM_VER_Y);
  
  // Display energy
  t_ENERGY->DrawTopLeft(bottom_bar_ox+ENERGIE_VER_X,
			bottom_bar_oy+ENERGIE_VER_Y);

  if (!character_under_cursor->IsDead()) {
    s = ulong2str(character_under_cursor->GetEnergy())+"%";
    t_character_energy->Set(s);

    barre_energie.Actu (character_under_cursor->GetEnergy());
  } else {
    std::string txt( _("(dead)") );
    t_character_energy->Set( txt );
    barre_energie.Actu (0);
  }

  t_character_energy->DrawTopLeft(bottom_bar_ox+ENERGIE_VER_X+t_ENERGY->GetWidth()+MARGIN,
				  bottom_bar_oy+ENERGIE_VER_Y);
   
  barre_energie.DrawXY (bottom_bar_ox+BARENERGIE_X,bottom_bar_oy+BARENERGIE_Y);
   
  // Display team logo
  Point2i dst(x + ECUSSON_EQUIPE_X, y + ECUSSON_EQUIPE_Y);
  app.video.window.Blit( character_under_cursor->TeamAccess().ecusson, dst);
}

void Interface::DisplayWeaponInfo ()
{
  Weapon* weapon;
  int nbr_munition;

  // Get the weapon
  if(weapon_under_cursor==NULL)
  {
    weapon = &ActiveTeam().AccessWeapon();
    nbr_munition = ActiveTeam().ReadNbAmmos();
  }
  else
  {
    weapon = weapon_under_cursor;
    nbr_munition = ActiveTeam().ReadNbAmmos(weapon_under_cursor->GetName());
  }

  // Display the name of the weapon
  t_WEAPON->DrawTopLeft(bottom_bar_ox+NOM_ARME_X, 
		     bottom_bar_oy+NOM_ARME_Y);

  std::string tmp( _(weapon->GetName().c_str()) );
  t_weapon_name->Set( tmp );

  t_weapon_name->DrawTopLeft(bottom_bar_ox+NOM_ARME_X+t_WEAPON->GetWidth()+MARGIN, 
			  bottom_bar_oy+NOM_ARME_Y);
  
  // Display number of ammo
  if (nbr_munition ==  INFINITE_AMMO)
    tmp = _("(unlimited)");
  else
    tmp = Format("%i", nbr_munition);
 
  t_STOCK->DrawTopLeft(bottom_bar_ox+MUNITION_X, 
		       bottom_bar_oy+MUNITION_Y);

  t_weapon_stock->Set(tmp);
  t_weapon_stock->DrawTopLeft(bottom_bar_ox+MUNITION_X+t_STOCK->GetWidth()+MARGIN, 
			      bottom_bar_oy+MUNITION_Y);

  // Display weapon icon
  if( !weapon->icone.IsNull() ){
      Point2i dest (bottom_bar_ox + ICONE_ARME_X, bottom_bar_oy + ICONE_ARME_Y);
      app.video.window.Blit( weapon->icone, dest);
  }else
      std::cout << "Can't blit weapon->icone => NULL " << std::endl;

  // Display CURRENT weapon icon on top
  weapon = &ActiveTeam().AccessWeapon();
  if (weapon != NULL) weapon->DrawWeaponBox();
}

void Interface::Draw ()
{    
  // display global timer
  Rectanglei dest ( (app.video.window.GetWidth()/2)-40, 0, bg_time.GetWidth(), bg_time.GetHeight() );	
  app.video.window.Blit( bg_time, dest.GetPosition() );
  std::string tmp(global_time.GetString());
  global_timer->Set(tmp);
  global_timer->DrawCenterTop(app.video.window.GetWidth()/2, 10); 
  
  world.ToRedrawOnScreen(dest);

  if ( game_loop.ReadState() == gamePLAYING && weapon_strength_bar.visible)
  {
    // Position on the screen 
    uint barre_x = (app.video.window.GetWidth()-weapon_strength_bar.GetWidth())/2;
    uint barre_y = app.video.window.GetHeight()-weapon_strength_bar.GetHeight() 
                   - interface.GetHeight()-10;
     
    // Drawing on the screen
     weapon_strength_bar.DrawXY (barre_x, barre_y);
  }
       
  weapons_menu.Draw();
  
  if (!display) return;

  int x = (app.video.window.GetWidth() - GetWidth())/2;
  int y = app.video.window.GetHeight() - GetHeight();

  bottom_bar_ox = x;
  bottom_bar_oy = y;
   
  Rectanglei dr( x, y, game_menu.GetWidth(), game_menu.GetHeight() );
  app.video.window.Blit( game_menu, dr.GetPosition());

  world.ToRedrawOnScreen(dr);
  
  // display time left in a turn ?
  if (timer != NULL && display_timer)
    timer->DrawCenter(x+GetWidth()/2, y+GetHeight()/2+3);
  
  // display character info
  DisplayCharacterInfo();

  // display weapon info
  DisplayWeaponInfo();
}

uint Interface::GetWidth() const { return 800; }
uint Interface::GetHeight() const { return 70; }

void Interface::EnableDisplay (bool _display)
{
  display = _display;
  camera.CentreObjSuivi ();
}

void Interface::UpdateTimer(uint utimer)
{
  if (utimer >= 0){
    if (timer!= NULL) {
      std::string s(ulong2str(utimer));
      timer->Set(s);
    }
    else 
	  timer = new Text(ulong2str(utimer), white_color, &global().big_font());
  } else
    timer = NULL;
}

void AbsoluteDraw(Surface &s, int x, int y)
{
  world.ToRedrawOnMap(Rectanglei(x, y, s.GetWidth(), s.GetHeight()));

  if( x + s.GetWidth() < (int)camera.GetX() 
	|| x > (int)camera.GetX()+(int)camera.GetWidth()
  	|| y + s.GetHeight() < (int)camera.GetY()
	|| y > (int)camera.GetY()+(int)camera.GetHeight() )
    return; //Drawing out of camera area

  Rectanglei src(0, 0, s.GetWidth(), s.GetHeight());
  Rectanglei dst(x - (int)camera.GetX(), y - (int)camera.GetY(), s.GetWidth(), s.GetHeight());

  if( dst.GetPositionX() < 0 ){
    src.SetSizeX( src.GetSizeX() + src.GetPositionX() );
    src.SetPositionX( 0 );
  }

  if( dst.GetPositionX() + src.GetSizeX() > camera.GetX() )
    src.SetSizeX( camera.GetWidth() - src.GetPositionX() );

  if( dst.GetPositionY() > 0 ){
    src.SetSizeY( src.GetSizeY() + src.GetPositionY() );
    src.SetPositionY(0);
  }

  if( dst.GetPositionY() + src.GetSizeY() > camera.GetY() )
    src.SetSizeY( camera.GetHeight() - src.GetPositionY() );

  //TODO:blit only the displayed part of the Surface
  app.video.window.Blit(s, src, dst.GetPosition() );
}

