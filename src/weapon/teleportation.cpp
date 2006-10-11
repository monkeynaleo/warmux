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
 * T��ortation : d�lacement d'un ver n'importe o sur le terrain.
 *****************************************************************************/

#include "teleportation.h"
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../game/time.h"
#include "../graphic/effects.h"
#include "../include/action_handler.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"

double ZOOM_MAX = 10; // zoom maximum durant le petit effet graphique
uint ESPACE = 4;

Teleportation::Teleportation() : Weapon(WEAPON_TELEPORTATION, "teleportation",
					new WeaponConfig(),
					VISIBLE_ONLY_WHEN_INACTIVE)
{  
  m_name = _("Teleportation");
  target_chosen = false;
}

bool Teleportation::p_Shoot ()
{
  if(!target_chosen)
	return false;

  // V�ifie qu'on se t��orte dans le vide !
  if( ActiveCharacter().IsOutsideWorldXY(dst) )
	 return false;

  Rectanglei rect = ActiveCharacter().GetTestRect();
  rect.SetPosition(dst); 

  if(!world.ParanoiacRectIsInVacuum(rect))
	 return false; 

  GameLoop::GetInstance()->interaction_enabled = false;

  jukebox.Play("share", "weapon/teleport_start");
  
  temps = Time::GetInstance()->Read();
  retour = false;

  return true;
}

void Teleportation::Refresh()
{
  if (!m_is_active) return;

  double dt = Time::GetInstance()->Read() - temps;

  // On a fait le chemin retour ?
  if (retour) {
    // Oui, c'est la fin de la t��ortation
    m_is_active = false;
    ActiveCharacter().SetSpeed(0.0,0.0);
    jukebox.Play("share","weapon/teleport_end");
    GameLoop::GetInstance()->interaction_enabled = true;
    return;
  }

  // Fin du chronometre ?
  if (GameMode::GetInstance()->duration_move_player * 1000 < dt)
  {
    // Non, on fait le chemin retour en 
    // commen�nt par d�lacer le ver
    retour = true;
    camera.SetXYabs(dst - camera.GetSize()/2);
    ActiveCharacter().SetXY(dst);
    temps = Time::GetInstance()->Read();
    dt = 0.0;
    return;
  }
}

void Teleportation::Draw()
{
  if (m_is_active) {
  } else {
    Weapon::Draw();
  }
}

void Teleportation::ChooseTarget(Point2i mouse_pos)
{
  target_chosen = true;
  dst = mouse_pos - ActiveCharacter().GetSize()/2;
  Shoot();
}

WeaponConfig& Teleportation::cfg()
{ return static_cast<WeaponConfig&>(*extra_params); }

