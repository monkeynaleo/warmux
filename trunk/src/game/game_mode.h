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
 * Mode de jeu : temps d'un tour, configuration des armes, nombre de ver
 * maximum par �quipe, etc.
 *****************************************************************************/

#ifndef MODE_JEU_H
#define MODE_JEU_H
//-----------------------------------------------------------------------------
#include <string>
#include "../include/base.h"
#include "../tool/xml_document.h"
//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

class GameMode 
{
public:
  uint max_characters;
  uint max_teams;
  uint duration_turn;
  uint duration_move_player;
  uint duration_exchange_player;
  double gravity;
  double safe_fall ;
  double damage_per_fall_unit ;

  struct s_character
  {
	uint init_energy;
    uint max_energy;
    uint mass;
    double air_resist_factor;
    uint jump_strength;
    int jump_angle;
    uint super_jump_strength;
    int super_jump_angle;
  } character;

  enum {
    ALWAYS,
    BEFORE_FIRST_ACTION,
    BEFORE_FIRST_ACTION_AND_END_TURN,
    CHANGE_ON_END_TURN,
    NEVER
  } allow_character_selection;
 
private:
  std::string m_current;
    
public:
  GameMode();
  bool Load(const std::string &mode);
  bool AllowCharacterSelection() const;

protected:
  bool LoadXml (xmlpp::Element *xml);
};

extern GameMode game_mode;
} // namespace Wormux
#endif
