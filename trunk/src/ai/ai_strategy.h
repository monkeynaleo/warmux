/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * A factory for AICommands. It contains turn specfic data unlike AIIdea.
 * Unlike an AICommand an AIStrategy has a rating and has a much simpler structure.
 *****************************************************************************/

#ifndef AI_STRATEGY_H
#define AI_STRATEGY_H

#include "ai/ai_command.h"
#include "character/body.h"


class AIStrategy
{
  private:
    const float rating;
  public:
    typedef enum {LOWER_RATING, SIMILAR_RATING, HIGHER_RATING} CompareResult;
    AIStrategy(float rating);
    virtual ~AIStrategy() {}
    /** Creates a command which implements the strategy */
    virtual AICommand * CreateCommand() = 0;
    float GetRating() { return rating; }
    CompareResult CompareRatingWith(AIStrategy * other);
};

class DoNothingStrategy : public AIStrategy
{
  public:
    DoNothingStrategy();
    virtual AICommand * CreateCommand();
};

class SkipTurnStrategy : public AIStrategy
{
  public:
    SkipTurnStrategy();
    virtual AICommand * CreateCommand();
};

class ShootWithGunStrategy : public AIStrategy
{
  private:
    Character & shooter;
    Weapon::Weapon_type weapon;
    BodyDirection_t  direction;
    double angle;
    int bullets;
  public:
    virtual AICommand * CreateCommand();
    ShootWithGunStrategy(double rating, Character & shooter, Weapon::Weapon_type weapon, BodyDirection_t  direction, double angle, int bullets);
};

class LoadAndFireStrategy : public AIStrategy
{
  private:
    Character & shooter;
    Weapon::Weapon_type weapon;
    BodyDirection_t  direction;
    double angle;
    double strength;
  public:
    virtual AICommand * CreateCommand();
    LoadAndFireStrategy(double rating, Character & shooter, Weapon::Weapon_type weapon, BodyDirection_t  direction, double angle, double strength);
};


#endif
