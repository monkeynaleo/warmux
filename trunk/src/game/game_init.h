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
 * Game init : Initialisations
 *****************************************************************************/

#ifndef GAME_INIT_H
#define GAME_INIT_H

class GameInit
{
public:
  static void Init();
private:
  // Initialization
  static void InitGameData_NetServer();
  static void InitGameData_NetClient();
  static void InitGameData_Local();

  static void InitMap();
  static void InitTeams();
  static void InitSounds();
  static void InitData();
};
#endif
