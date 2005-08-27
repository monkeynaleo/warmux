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
 * Police de caractère.
 *****************************************************************************/

#ifndef POLICE_H
#define POLICE_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include <ClanLib/display.h>
//-----------------------------------------------------------------------------

class Police
{
public:
  CL_Font *m_police;

public:
  Police();
  void Load (const std::string& resource_id, CL_ResourceManager* manager);
  void WriteLeft (int x, int y, const std::string &txt);
  void WriteLeftBottom (int x, int y, const std::string &txt);
  void WriteRight (int x, int y, const std::string &txt);
  void WriteCenterTop (int x, int y, const std::string &txt);
  void WriteCenter (int x, int y, const std::string &txt);
  uint GetWidth (const std::string &txt);
  uint GetHeight (const std::string &txt);
  CL_Font &Acces();
  const CL_Font &Read() const;
};

//-----------------------------------------------------------------------------
#endif
