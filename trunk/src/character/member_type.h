/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 *****************************************************************************/

#ifndef MEMBER_TYPE_H
#define MEMBER_TYPE_H

class MemberType
{
  static std::map<std::string, int> Map;
  static int count;
public:
  int type;
  MemberType(const std::string& name)
  {
    std::map<std::string, int>::const_iterator found = Map.find(name);
    if (found == Map.end()) { type = count; Map[name] = count++; }
    else { type = Map[name]; }
  }
  MemberType(const MemberType& other) { type = other.type; }
  operator int() const { return type; }
  bool operator ==(const std::string& name) const
  {
    std::map<std::string, int>::const_iterator found = Map.find(name);
    if (found == Map.end()) return false;
    return found->second == type;
  }
  bool operator==(const MemberType& other) const { return type == other.type; }
};

#endif //MEMBER_TYPE_H
