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
 * Arme "batte de baseball" : permet de donner un coup � un autre ver.
 *****************************************************************************/

#include "baseball.h"
//-----------------------------------------------------------------------------
#include <ClanLib/core.h>
#include "../team/macro.h"
#include "../tool/geometry_tools.h"
#include "../game/game_loop.h"
#include "../tool/i18n.h"
//-----------------------------------------------------------------------------
namespace Wormux 
{
Baseball baseball;

Baseball::Baseball() : Weapon(WEAPON_BASEBALL, "baseball")
{
  m_name = _("Baseball");
  extra_params = new BaseballConfig();
}

//-----------------------------------------------------------------------------

bool Baseball::p_Shoot ()
{
  double angle = ActiveTeam().crosshair.GetAngleRad();
  int ver_x, ver_y;
  int x,y;
  double dx,dy;
  double rayon = 0.0;
  bool fin = false;

  RotationPointXY (ver_x, ver_y);
  jukebox.Play ("weapon/baseball");

  do
  {
    // On a fini les calculs ?
    rayon += 1.0;
    if (cfg().longueur < rayon) 
    {
      rayon = cfg().longueur;
      fin = true;
    }

    // Calcul des coordonn�es du point
    dx = (int)(rayon*cos( angle ));
    dy = (int)(rayon*sin( angle ));
    x = ver_x +(int)dx;
    y = ver_y +(int)dy;

    // Teste un ver apr�s l'autre
    POUR_TOUS_VERS_VIVANTS(equipe,ver)
    if (&(*ver) != &ActiveCharacter())
    {
      // On a touch� un ver ?
      if (ObjTouche(*ver, CL_Point(x, y)))
      {
	// Inflige les d�gats au ver touch�
	(*ver).SetEnergyDelta (-cfg().damage);
	(*ver).SetSpeed (cfg().force, angle);
	return true;
      }
    }
  } while (!fin);

  return true;
}

//-----------------------------------------------------------------------------

void Baseball::Refresh()
{
  if (m_is_active)
    m_is_active = false;
}

//-----------------------------------------------------------------------------

BaseballConfig& Baseball::cfg() 
{ return static_cast<BaseballConfig&>(*extra_params); }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BaseballConfig::BaseballConfig()
{ 
  longueur =  70;
  force = 250;
}

void BaseballConfig::LoadXml(xmlpp::Element *elem)
{
  WeaponConfig::LoadXml(elem);
  LitDocXml::LitUint (elem, "longueur", longueur);
  LitDocXml::LitUint (elem, "force", force);
}

//-----------------------------------------------------------------------------
} // namespace Wormux
