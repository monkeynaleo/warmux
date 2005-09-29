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
 * Téléportation : déplacement d'un ver n'importe où sur le terrain.
 *****************************************************************************/

#include "teleportation.h"
//-----------------------------------------------------------------------------
#include "../interface/mouse.h"
#include "../include/action_handler.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../game/game_loop.h"
#include "../map/map.h"
#include "../game/time.h"
//-----------------------------------------------------------------------------
namespace Wormux {
Teleportation teleportation;
//-----------------------------------------------------------------------------

const uint duree_animation = 1000; // ms
double ZOOM_MAX = 10; // zoom maximum durant le petit effet graphique
uint ESPACE = 4;

//-----------------------------------------------------------------------------

Teleportation::Teleportation() : Weapon(WEAPON_TELEPORTATION, "teleportation")
{  
  m_name = _("Teleportation");

  m_visibility = VISIBLE_ONLY_WHEN_INACTIVE;
  extra_params = new WeaponConfig();  
}

//-----------------------------------------------------------------------------

bool Teleportation::p_Shoot ()
{
  // Vérifie qu'on se téléporte dans le vide !
  dst = mouse.GetPosMonde();
  dst.x -= ActiveCharacter().GetWidth()/2;
  dst.y -= ActiveCharacter().GetHeight()/2;
  if (ActiveCharacter().IsOutsideWorldXY(dst.x, dst.y)) return false;
  if (!ActiveCharacter().IsInVacuumXY (dst.x, dst.y)) return false; 

  game_loop.interaction_enabled = false;

#ifdef CL
  jukebox.Play("weapon/teleport_start");
#else 
  jukebox.Play("share", "weapon/teleport_start");
#endif
  
  // Initialise les variables
  temps = Wormux::temps.Lit();
  retour = false;
  ActiveCharacter().Desactive();
  m_direction = ActiveCharacter().GetDirection();
  return true;
}

//-----------------------------------------------------------------------------

void Teleportation::Refresh()
{
  if (!m_is_active) return;

  double dt = Wormux::temps.Lit() - temps;

  // Fin du chronometre ?
  if (duree_animation < dt)
  {
    // On a fait le chemin retour ?
    if (retour) {
      // Oui, c'est la fin de la téléportation
      m_is_active = false;
      ActiveCharacter().image.set_scale (m_direction, 1);
      ActiveCharacter().SetSpeed(0.0,0.0);
      ActiveCharacter().Reactive();
      
#ifdef CL
      jukebox.Play("weapon/teleport_end");
#else
      jukebox.Play("share","weapon/teleport_end");
#endif
      game_loop.interaction_enabled = true;
      return;

    } else {
      // Non, on fait le chemin retour en 
      // commençant par déplacer le ver
      retour = true;
      ActiveCharacter().SetXY (dst.x, dst.y);
      temps = Wormux::temps.Lit();
      dt = 0.0;
    }
  }

  // Calcule le zoom
  if ((duree_animation/2) <= dt) {
    // Seconde partie
    dt = (duree_animation-dt)/duree_animation;
    if (retour)
      m_zoom = 1+dt*(ZOOM_MAX-1);
    else
      m_zoom = dt*ZOOM_MAX;
  } else {
    // Premier partie
    dt /= duree_animation;
    if (retour)
      m_zoom = dt*ZOOM_MAX;
    else
      m_zoom = 1+dt*(ZOOM_MAX-1);
  }

  uint larg=ActiveCharacter().GetWidth();
  uint haut=ActiveCharacter().GetHeight();
  uint nv_larg = (uint)(larg * m_zoom);
  uint nv_haut = (uint)(haut * m_zoom);
  m_x = ActiveCharacter().GetX() - (nv_larg-larg)/2;
  m_y = ActiveCharacter().GetY() - (nv_haut-haut)/2;
  if(ActiveCharacter().GetDirection() == -1)
    m_x += nv_larg;
}

//-----------------------------------------------------------------------------

void Teleportation::Draw()
{
  if (m_is_active) {
    ActiveCharacter().image.set_scale (m_zoom*m_direction, m_zoom);
    ActiveCharacter().image.draw (m_x, m_y);
  } else {
    Weapon::Draw();
  }
}

//-----------------------------------------------------------------------------

void Teleportation::ChooseTarget()
{
  ActiveTeam().GetWeapon().NewActionShoot();
}

//-----------------------------------------------------------------------------

WeaponConfig& Teleportation::cfg()
{ return static_cast<WeaponConfig&>(*extra_params); }

//-----------------------------------------------------------------------------
} // namespace Wormux
