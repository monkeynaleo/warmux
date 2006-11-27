/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
 * Wormux enums ...
 *****************************************************************************/

#ifndef WORMUX_ENUM_H
#define WORMUX_ENUM_H
//-----------------------------------------------------------------------------

typedef enum
{
  WEAPON_BAZOOKA,
  WEAPON_AUTOMATIC_BAZOOKA,
  WEAPON_RIOT_BOMB,
  WEAPON_GRENADE,
  WEAPON_HOLLY_GRENADE,
  WEAPON_CLUSTER_BOMB,

  WEAPON_GUN,
  WEAPON_SHOTGUN,
  WEAPON_SUBMACHINE_GUN,
  WEAPON_BASEBALL,

  WEAPON_DYNAMITE,
  WEAPON_MINE,

  WEAPON_SUPERTUX,
  WEAPON_AIR_ATTACK,
  WEAPON_ANVIL,
  WEAPON_GNU,
  WEAPON_POLECAT,
  WEAPON_BOUNCE_BALL,

  WEAPON_TELEPORTATION,
  WEAPON_NINJA_ROPE,
  WEAPON_LOWGRAV,
  WEAPON_SUICIDE,
  WEAPON_SKIP_TURN,
  WEAPON_JETPACK,
  WEAPON_PARACHUTE,
  WEAPON_AIR_HAMMER,
  WEAPON_CONSTRUCT,
  WEAPON_SNIPE_RIFLE,
  WEAPON_BLOWTORCH,
  WEAPON_SYRINGE
} Weapon_type;

//-----------------------------------------------------------------------------

typedef enum
{
  DIRECTION_LEFT = -1,
  DIRECTION_RIGHT = 1
} Direction_t;

//-----------------------------------------------------------------------------

#endif
