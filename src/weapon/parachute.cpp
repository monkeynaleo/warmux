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
 * Parachute !
 *****************************************************************************/

#include "../weapon/parachute.h"
//-----------------------------------------------------------------------------
#include "../tool/i18n.h"
#include "../team/teams_list.h"
#include "../sound/jukebox.h"
#include "../game/game.h"
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../interface/game_msg.h"
#include "../object/physical_obj.h"
#include "../weapon/weapon_tools.h"

#ifdef CL
CL_Sprite image ;
#endif

//-----------------------------------------------------------------------------
namespace Wormux 
{
Parachute parachute;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

Parachute::Parachute() : Weapon(WEAPON_PARACHUTE, "parachute")
{
  m_name = _("Parachute");
  m_initial_nb_ammo = 2 ;
  air_resist_factor = 15.0 ;
  open_speed_limit = 5.0 ;
  extra_params = new WeaponConfig();
  use_unit_on_first_shoot = false;  
}

//-----------------------------------------------------------------------------

void Parachute::p_Select()
{
  m_is_active = true ;
  open = false ;
  closing = false ;
#ifdef CL
  image.set_play_backward(false);
  image.set_show_on_finish(CL_Sprite::show_last_frame);
  image.restart();
#else
  image->Start();
  image->SetShowOnFinish(Sprite::show_last_frame);
  //TODO
#endif
}

//-----------------------------------------------------------------------------

void Parachute::p_Deselect()
{
  ActiveCharacter().SetAirResistFactor(game_mode.character.air_resist_factor);
  ActiveCharacter().SetWindFactor(0);
  m_is_active = false;
}

//-----------------------------------------------------------------------------

void Parachute::p_Init()
{
  m_name = _("parachute");
#ifdef CL
  m_image = CL_Surface("mine", &graphisme.weapons);
  image = CL_Sprite("parachute_sprite", &graphisme.weapons);
  image.set_play_loop(false);
#else
//  m_image = resource_manager.LoadSprite(res,"mine");
  image = resource_manager.LoadSprite(weapons_res_profile,"parachute_sprite");
  //TODO : image.set_play_loop(false);
#endif
}


//-----------------------------------------------------------------------------

bool Parachute::p_Shoot()
{
  return true;
}

void Parachute::Draw()
{
  if (open)
    {
#ifdef CL
      image.update();
      image.draw(ActiveCharacter().GetX() - ActiveCharacter().GetWidth()/2,
		 ActiveCharacter().GetY() - image.get_height());
#else
      image->Update();
      image->Draw(ActiveCharacter().GetX() - ActiveCharacter().GetWidth()/2,
		 ActiveCharacter().GetY() - image->GetHeight());
#endif
    }
}

//-----------------------------------------------------------------------------

void Parachute::Refresh()
{
  DoubleVector speed ;

  ActiveCharacter().GetSpeedXY(speed);

  if (ActiveCharacter().FootsInVacuum())
    {
      if (!open && (speed.y > open_speed_limit))
	{
	  if (UseAmmo())
	    {
	      ActiveCharacter().SetAirResistFactor(air_resist_factor);
	      ActiveCharacter().SetWindFactor(2);
	      open = true ;
	    }
	}
    }
  else
    {
      /* We are on the ground */
      if (open)
	{
	  /* The parachute is opened */
	  if (!closing)
	    {
	      /* We have just hit the ground. Start closing animation */
#ifdef CL
	      image.set_play_backward(true);
	      image.set_show_on_finish(CL_Sprite::show_blank);
	      image.restart();
#else
	      image->SetPlayBackward(true);
        image->SetShowOnFinish(Sprite::show_blank);
        image->Start();
#endif
	      closing = true ;
	    }
	  else
	    {
	      /* The parachute is closing */
#ifdef CL
	      if (image.is_finished())
		{
		  /* The animation is finished...
		     We are done with the parachute */
		  open = false ;
		  closing = false ;
		  m_image = CL_Surface("mine", &graphisme.weapons);
		  UseAmmoUnit();
		}
#else
  //TODO
#endif
	    }
	}
    }
}

//-----------------------------------------------------------------------------

void Parachute::SignalTurnEnd()
{
  p_Deselect();
}

//-----------------------------------------------------------------------------
} // namespace Wormux
