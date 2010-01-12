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
 * A factory for AI strategies. It contains no turn specfic data.
 *****************************************************************************/

#include "ai/ai_idea.h"
#include "ai/trajectory.h"
#include "character/character.h"
#include "game/game.h"
#include "game/game_mode.h"
#include "map/map.h"
#include "map/wind.h"
#include "object/objects_list.h"
#include "team/team.h"
#include "team/macro.h"
#include "tool/math_tools.h"
#include "weapon/weapon_launcher.h"
#include "weapon/weapons_list.h"
#include "weapon/shotgun.h"

// This constant defines how how much damage is worth killing one character?
// e.g. Killing one Character with 20 health is about the same worth like doing a sum of 120 damage (60 each) to two characters without killing them. Both cases would get a rating of 120 when this constant is 100.
const double BONUS_FOR_KILLING_CHARACTER = 100;
const double MALUS_PER_UNUSED_DAMGE_POINT = 0.1;

bool AIIdea::CanUseWeapon(Weapon * weapon)
{
  bool correct_weapon = weapon == &(ActiveTeam().GetWeapon());
  bool can_change_weapon = ActiveTeam().GetWeapon().CanChangeWeapon()
    && (Game::GetInstance()->ReadState() == Game::PLAYING);
  return correct_weapon || (can_change_weapon && weapon->EnoughAmmo());
}

bool AIIdea::CanUseCharacter(Character & character)
{
  if (character.IsDead())
    return false;

  bool can_change_character = GameMode::GetInstance()->AllowCharacterSelection()
    && (Game::GetInstance()->ReadState() == Game::PLAYING)
    && !Game::GetInstance()->IsCharacterAlreadyChosen();
  return (character.IsActiveCharacter() || can_change_character);
}

LRDirection AIIdea::XDeltaToDirection(double delta)
{
  if (delta < 0)
    return DIRECTION_LEFT;
  else
    return DIRECTION_RIGHT;
}

double AIIdea::GetDirectionRelativeAngle(LRDirection direction, double angle)
{
  if (direction == DIRECTION_LEFT)
    return InverseAngleRad(angle);
  else
    return angle;
}

double AIIdea::RateDamageDoneToEnemy(int damage, Character & enemy)
{
  double rating = std::min(damage, enemy.GetEnergy());
  if (damage >= enemy.GetEnergy()) {
    rating += BONUS_FOR_KILLING_CHARACTER;
    double unused_damage = damage - enemy.GetEnergy();
    rating -= MALUS_PER_UNUSED_DAMGE_POINT * unused_damage;
  }
  return rating;
}

AIStrategy * SkipTurnIdea::CreateStrategy()
{
  WeaponsList * weapons_list = Game::GetInstance()->GetWeaponsList();
  Weapon * weapon = weapons_list->GetWeapon(Weapon::WEAPON_SKIP_TURN);
  if (!CanUseWeapon(weapon))
    return NULL;
  return new SkipTurnStrategy();
}

AIStrategy * WasteAmmoUnitsIdea::CreateStrategy()
{
  if (ActiveTeam().GetWeapon().CanChangeWeapon())
    return NULL;
  Weapon::Weapon_type weapon_type = ActiveTeam().GetWeapon().GetType();
  int used_ammo_units = ActiveTeam().ReadNbUnits(weapon_type);
  double max_angle = -ActiveTeam().GetWeapon().GetMinAngle();
  return new ShootWithGunStrategy(-0.1, ActiveCharacter(), weapon_type, ActiveCharacter().GetDirection(), max_angle, used_ammo_units);
}

ShootDirectlyAtEnemyIdea::ShootDirectlyAtEnemyIdea(WeaponsWeighting & weapons_weighting, Character & shooter, Character & enemy, Weapon::Weapon_type weapon_type, double max_distance):
  weapons_weighting(weapons_weighting),
  shooter(shooter),
  enemy(enemy),
  weapon_type(weapon_type),
  max_distance(max_distance)
{
  // do nothing
}

static PhysicalObj* GetObjectAt(const Point2i & pos)
{
  ObjectsList * objects = ObjectsList::GetInstance();
  ObjectsList::iterator it = objects->begin();
  while(it != objects->end()) {
    PhysicalObj* object = *it;
    if (object->GetTestRect().Contains(pos) && !object->IsDead())
      return object;
    it++;
  }
  FOR_ALL_CHARACTERS(team, character) {
    if (character->GetTestRect().Contains(pos) && !character->IsDead())
      return &(*character);
  }
  return NULL;
}

/* Returns the object the missile has collided with or NULL if the missile has collided with the ground. */
static PhysicalObj* GetCollisionObject(Character * character_to_ignore, const Point2i from, const Point2i to) {
  Point2i pos = from;
  Point2i delta = to - from;
  int steps_x = abs(delta.x);
  int steps_y = abs(delta.y);
  int step_x = delta.x < 0 ? -1 : 1;
  int step_y = delta.y < 0 ? -1 : 1;
  int done_x_mul_steps_y  = 0;
  int done_y_mul_steps_x  = 0;
  // explanation of done_x_mul_steps_y:
  // done_x = how often has step_x been added to pos.x
  // done_x_mul_steps_y = done_x * steps_y
  // example of algorithm:
  // given: departure = (0,0) arrival = (-2,7);
  // => steps_x = 2; steps_y = 7; step_x = -1; step_y = 1
  // progress:
  // pos.x | pos.y | done_x_mul_steps_y | done_y_mul_steps_x
  // -------------------------------------------------------
  // 0     | 0     | 0                  | 0
  // 0     | 1     | 0                  | 2
  // -1    | 2     | 7                  | 4
  // -1    | 3     | 7                  | 6
  // -1    | 4     | 7                  | 8
  // -1    | 5     | 7                  | 10
  // -2    | 6     | 14                 | 12
  // -2    | 7     | 14                 | 14
  // The algorithm tries to keep the difference small between:
  // done_x_mul_steps_y and done_y_mul_steps_x.
  // By doing so it gets assured that all intermediate positions pos form a straight line.
  // (Or at least something close to a straight line)
  while (pos != to) {
    int new_done_x_mul_steps_y = done_x_mul_steps_y + steps_y;
    int new_done_y_mul_steps_x = done_y_mul_steps_x + steps_x;
    int diff_after_step_x = abs(done_y_mul_steps_x - new_done_x_mul_steps_y);
    int diff_after_step_y = abs(new_done_y_mul_steps_x - done_x_mul_steps_y);

    if (diff_after_step_x <= diff_after_step_y) {
      pos.x += step_x;
      done_x_mul_steps_y = new_done_x_mul_steps_y;
    }
    if (diff_after_step_y <= diff_after_step_x) {
      pos.y += step_y;
      done_y_mul_steps_x = new_done_y_mul_steps_x;
    }

    if (GetWorld().IsOutsideWorld(pos))
      return NULL;

    if (!GetWorld().IsInVacuum(pos))
      return NULL;

    PhysicalObj* object = GetObjectAt(pos);
    if (object != NULL && object != character_to_ignore)
      return object;
  }
  return NULL;
}

AIStrategy * ShootDirectlyAtEnemyIdea::CreateStrategy() {
  if (enemy.IsDead())
    return NULL;

  if (!CanUseCharacter(shooter))
    return NULL;

  WeaponsList * weapons_list = Game::GetInstance()->GetWeaponsList();
  WeaponLauncher * weapon = dynamic_cast<WeaponLauncher*>(weapons_list->GetWeapon(weapon_type));

  if (!CanUseWeapon(weapon))
    return NULL;

  // We need to use center point, because gunholePosition is location
  // of last weapon of the ActiveTeam() and not the future gunholePos
  // which will be select.
  // TODO: Please find an alternative to solve this tempory solution
  Point2d departure = shooter.GetCenter();
  Point2d arrival = enemy.GetCenter();

  if (departure.Distance(arrival) > max_distance)
    return NULL;

  double original_angle = departure.ComputeAngle(arrival);

  LRDirection direction = XDeltaToDirection(arrival.x - departure.x);
  double shoot_angle = GetDirectionRelativeAngle(direction, original_angle);

  if (!weapon->IsAngleValid(shoot_angle))
    return NULL;

  PhysicalObj * collision_object = GetCollisionObject(&shooter, departure, arrival);
  if (collision_object != &enemy)
    return NULL;

  int available_ammo_units = ActiveTeam().ReadNbUnits(weapon_type);

  int damage_per_ammo_unit = weapon->GetDamage();
  if (weapon_type == Weapon::WEAPON_SHOTGUN) {
    damage_per_ammo_unit *= SHOTGUN_BULLETS;
  }
  int required_ammo_units = (enemy.GetEnergy() + damage_per_ammo_unit -1) / damage_per_ammo_unit;
  int used_ammo_units = std::min(required_ammo_units, available_ammo_units);
  int damage = used_ammo_units * damage_per_ammo_unit;

  double rating = RateDamageDoneToEnemy(damage, enemy);
  rating = rating * weapons_weighting.GetFactor(weapon_type);
  return new ShootWithGunStrategy(rating, shooter, weapon_type , direction, shoot_angle, used_ammo_units);
}

FireMissileWithFixedDurationIdea::FireMissileWithFixedDurationIdea(WeaponsWeighting & weapons_weighting, Character & shooter, Character & enemy, double duration):
  weapons_weighting(weapons_weighting),
  shooter(shooter),
  enemy(enemy),
  duration(duration)
{
  // do nothing
}

static bool IsPositionEmpty(Character & character_to_ignore, Point2i pos)
{
  if (GetWorld().IsOutsideWorld(pos))
    return false;

  if (!GetWorld().IsInVacuum(pos))
    return false;

  PhysicalObj* object = GetObjectAt(pos);
  if (object != NULL && object != &character_to_ignore)
    return false;
  return true;
}

static const Point2i GetFirstContact(Character & character_to_ignore, Trajectory & trajectory)
{
  double time = 0;
  Point2i pos;
  do {
    pos = trajectory.GetPositionAt(time);
    double pixel_per_second = trajectory.GetSpeedAt(time);
    double seconds_per_pixel = 1 / pixel_per_second;
    time += seconds_per_pixel;
  } while(IsPositionEmpty(character_to_ignore, pos));
  return pos;
}

AIStrategy * FireMissileWithFixedDurationIdea::CreateStrategy()
{
  if (enemy.IsDead())
    return NULL;

  if (!CanUseCharacter(shooter))
    return NULL;

  const Weapon::Weapon_type weapon_type = Weapon::WEAPON_BAZOOKA;
  WeaponsList * weapons_list = Game::GetInstance()->GetWeaponsList();
  WeaponLauncher * weapon = dynamic_cast<WeaponLauncher*>(weapons_list->GetWeapon(weapon_type));

  if (!CanUseWeapon(weapon))
    return NULL;
  double g = GameMode::GetInstance()->gravity;
  double wind_factor = weapon->GetWindFactor();
  double mass = weapon->GetMass();
  Point2d a(Wind::GetRef().GetStrength() * wind_factor, g * mass);
  a *= 2; // Work around bug in physics engine.
  const Point2d pos_0 = shooter.GetCenter();
  const Point2d pos_t = enemy.GetCenter();
  double t = duration;
  // Calculate v_0 using "pos_t = 1/2 * a_x * t*t + v_0*t + pos_0":
  Point2d v_0 = (pos_t - pos_0)/t - 0.5*a * t;

  double strength = v_0.Norm() / PIXEL_PER_METER;
  double angle = v_0.ComputeAngle();
  LRDirection direction = XDeltaToDirection(v_0.x);
  double shoot_angle = GetDirectionRelativeAngle(direction, angle);
  if (!weapon->IsAngleValid(shoot_angle))
    return NULL;

  if (strength > weapon->GetMaxStrength())
    return NULL;

  Trajectory trajectory(pos_0, v_0, a);
  Point2i explosion_pos = GetFirstContact(shooter, trajectory);
  PhysicalObj * aim = GetObjectAt(explosion_pos);
  double rating;
  if (aim == NULL) {
    // Shooting at ground is better then doing nothing, so give it a low positive rating.
    rating = 0.1;
    // Shooter should not be to close to explosion:
    if (explosion_pos.Distance(shooter.GetCenter()) < (double)weapon->cfg().blast_range)
      return NULL;
  } else if (aim == &enemy) {
    // Trough collision damage the actual damage is higher then 50.
    // There is a good chance that the other character get killed.
    // That's why I have choosen a rating of 120.
    rating = 120;
  } else {
    return NULL;
  }
  rating = rating * weapons_weighting.GetFactor(weapon_type);
  return new LoadAndFireStrategy(rating, shooter, weapon_type, direction, shoot_angle, strength);
}
