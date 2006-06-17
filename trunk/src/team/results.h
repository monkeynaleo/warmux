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
 * A worms team.
 *****************************************************************************/

#ifndef RESULTS_H
#define RESULTS_H

#include <vector>
#include <map>
#include <string>

class Character;
class Team;

// Could be a 5-tuple
class TeamResults
{
 private:
    //std::couple<const sting&, int> ?
    const char*      teamName;
    const Character* mostViolent;
    const Character* mostUsefull;
    const Character* mostUseless;
    const Character* biggestTraitor;
 protected:
    TeamResults(const char*      name,
                const Character* MV,
                const Character* MUl,
                const Character* MUs,
                const Character* BT);
 public:
    static TeamResults* createTeamResults(Team* team);
    static TeamResults* createGlobalResults(std::vector<TeamResults*>* list);
    static std::vector<TeamResults*>* TeamResults::createAllResults(void);
    static void TeamResults::deleteAllResults(std::vector<TeamResults*>* results_list);
    void RenderText(std::string& str);
    const char*      getTeamName() const { return teamName; };
    const Character* getMostViolent() const { return mostViolent; };
    const Character* getMostUsefull() const { return mostUsefull; };
    const Character* getMostUseless() const { return mostUseless; };
    const Character* getBiggestTraitor() const { return biggestTraitor; };
};

typedef std::vector<TeamResults*>::iterator res_iterator;
typedef std::vector<TeamResults*>::const_iterator const_res_iterator;
#endif //RESULTS_H
