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
 * Functions to generate random datas (number, boolean, etc.).
 *****************************************************************************/

#include <time.h>
#include <WORMUX_debug.h>
#include <WORMUX_random.h>

RandomGenerator::RandomGenerator() :
  next(0), initialized(false), unrandom(false)
{
}

RandomGenerator::~RandomGenerator()
{
}

void RandomGenerator::UnRandom()
{
  next = 0x12345678;
  initialized = true;
  unrandom = true;
}

void RandomGenerator::InitRandom()
{
  if (unrandom) {
    MSG_DEBUG("random.set", "no, unrandom set");
    return;
  }
  SetRand(time(NULL));
}

void RandomGenerator::SetRand(uint seed)
{
  if (unrandom) {
    MSG_DEBUG("random.set", "no, unrandom set");
    return;
  }
  MSG_DEBUG("random.set", "srand: %d", seed);
  next = seed;
  initialized = true;
}

void RandomGenerator::SetSeed(uint seed)
{
  if (unrandom) {
    MSG_DEBUG("random.set", "no, unrandom set");
    return;
  }
  SetRand(seed);
}

uint RandomGenerator::GetSeed()
{
  return next;
}

/******************************************************************************
 * From "man 3 rand"
 * POSIX.1-2001  gives the following example of an implementation of rand() and
 * srand(), possibly useful when one needs the same sequence on two different
 * machines.
 ******************************************************************************/

/* RAND_MAX assumed to be 32767 */
uint RandomGenerator::GetRand()
{
  ASSERT(initialized == true);

  next = next * 1103515245 + 12345;
  return((uint)(next/65536) % 32768);
}

#define WORMUX_RAND_MAX 32767

bool RandomGenerator::GetBool()
{
  Double middle = WORMUX_RAND_MAX/2;
  Double random = GetRand();
  return (random <= middle);
}

/**
 * Get a random number between 0.0 and 1.0
 *
 * @return A number between 0.0 and 1.0
 */
Double RandomGenerator::GetDouble()
{
  return 1.0*GetRand()/(WORMUX_RAND_MAX + 1.0);
}

/**
 *  Get a random number between min and max
 */
int32_t RandomGenerator::GetLong(int32_t min, int32_t max)
{
  return min + (int32_t)GetDouble(max - min + 1);
}

int32_t RandomGenerator::GetInt(int32_t min, int32_t max)
{
  return GetLong(min, max);
}

uint RandomGenerator::GetUint(uint min, uint max)
{
  return (uint)GetLong(min, max);
}

Double RandomGenerator::GetDouble(Double min, Double max)
{
  return min + GetDouble(max - min);
}

Double RandomGenerator::GetDouble(Double max)
{
  return max * GetDouble();
}

/**
 * Return a random point in the given rectangle.
 *
 * @param rect The rectangle in which the returned point will be.
 * @return a random point.
 */
Point2i RandomGenerator::GetPoint(const Rectanglei &rect)
{
  Point2i topPoint = rect.GetPosition();
  Point2i bottomPoint = rect.GetBottomRightPoint();
  int32_t x = GetLong(topPoint.x, bottomPoint.x);
  int32_t y = GetLong(topPoint.y, bottomPoint.y);
  return Point2i( x, y );
}

Point2i RandomGenerator::GetPoint(const Point2i &pt)
{
  int32_t x = GetLong(0, pt.x - 1);
  int32_t y = GetLong(0, pt.y - 1);
  return Point2i( x, y );
}

int RandomGenerator::GetSign()
{
  if (GetBool()) return 1;
  else return -1;
}
