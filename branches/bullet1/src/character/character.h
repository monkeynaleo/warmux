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
 * Character of a team.
 *****************************************************************************/

#ifndef _CHARACTER_H
#define _CHARACTER_H

#include <limits>
#include <string>
#include "character/body.h"
#include "gui/energy_bar.h"
#include "include/base.h"
#include "physic/game_obj.h"
#include "character/body.h"
#include "character/walk_intention.h"

class Text;
class Team;
class ParticleEngine;
class DamageStatistics;
class Force;

#ifdef DEBUG
//#define DEBUG_SKIN
#endif

class Character : public GameObj
{
private:
  /* If you need this, implement it (correctly) */
  Character operator=(const Character&);
  /**********************************************/

  std::string character_name;

  Team &m_team;
  bool step_sound_played;
  bool prepare_shoot;
  bool back_jumping;
  bool death_explosion;
  double firing_angle;
  PhysicalShape *m_feet_shape;

  uint disease_damage_per_turn;
  uint disease_duration; // std::numeric_limits<uint>::max() means unlimited
  DamageStatistics *damage_stats;
  EnergyBar energy_bar;
  Force *m_force_walk_index;

  // survived games
  int survivals;

  // name
  Text* name_text;

  // chrono
  uint rl_motion_pause;  // pause for left/right motion
  uint do_nothing_time;
  uint walking_time;
  uint animation_time;
  int lost_energy;
  bool hidden; //The character is hidden (needed by teleportation)
  bool walking_slowly;

  // Channel used for sound
  int channel_step;

  // Generates green bubbles when the character is ill
  ParticleEngine *particle_engine;

  // this is needed because of network needing to know
  // if we have changed of active character
  bool is_playing;
  WalkIntention walk_intention;
  bool increase_fire_angle_pressed;
  bool increase_fire_angle_slowly_pressed;
  bool decrease_fire_angle_pressed;
  bool decrease_fire_angle_slowly_pressed;
  uint last_direction_change;

public:

  // Previous strength
  double previous_strength;

  Body* body;

private:
  void DrawEnergyBar(int dy) const;
  void DrawName(int dy) const;

  virtual void SignalDrowning();
  virtual void SignalGhostState(bool was_dead);
  virtual void SignalGroundCollision(const Point2d& speed_before);
  virtual void SignalObjectCollision(const Point2d& my_speed_before,
				     GameObj * obj,
				     const Point2d& obj_speed);
  void Collision(const Point2d& speed_vector);
  void SetBody(Body* char_body);

  void AddFiringAngle(double angle) { SetFiringAngle(firing_angle + angle); };

  void StartWalking(bool slowly);
  void StopWalking();
  bool IsWalking() const;
  void MakeSteps();
  bool IsChangingDirection();
  bool ComputeHeightMovement(int & height);
public:

  Character (Team& my_team, const std::string &name, Body *char_body);
  Character (const Character& acharacter);
  ~Character();

  virtual void SignalExplosion();

  void StartOrStopWalkingIfNecessary();
  WalkIntention & GetWalkIntention() { return walk_intention; };

  // Energy related
  void SetEnergyDelta(int delta, bool do_report = true);
  void SetEnergy(int new_energy);

  bool GotInjured() const { return lost_energy < 0; };
  void Die();
  void DisableDeathExplosion() { death_explosion = false; };
  bool IsActiveCharacter() const;
  // Disease handling
  bool IsDiseased() const { return (disease_duration > 0 && !IsDead()); };

  void SetDiseaseDamage(const uint damage_per_turn, const uint duration)
  {
    disease_damage_per_turn = damage_per_turn;
    disease_duration = duration;
  }
// Keep almost 1 in energy
  uint GetDiseaseDamage() const
  {
    if (disease_damage_per_turn < static_cast<uint>(GetEnergy()))
      return disease_damage_per_turn;
    return GetEnergy() - 1;
  }
  void DecDiseaseDuration()
  {
    if (disease_duration == std::numeric_limits<uint>::max()) return; // infinite disease duration

    if (disease_duration > 0) disease_duration--;
    else disease_damage_per_turn = 0;
  }

  void Draw();
  void Refresh();

  bool FootsInVacuum() const;

  void PrepareTurn();
  void StartPlaying();
  void StopPlaying();

  void PrepareShoot();
  bool IsPreparingShoot() const { return prepare_shoot; };
  void DoShoot();
  double GetFiringAngle() const;
  double GetAbsFiringAngle() const { return firing_angle; };
  void SetFiringAngle(double angle);

  void StartIncreasingFireAngle(bool slowly);
  void StopIncreasingFireAngle(bool slowly);
  void StartDecreasingFireAngle(bool slowly);
  void StopDecreasingFireAngle(bool slowly);

  // Show hide the Character
  void Hide() { hidden = true; };
  void Show() { hidden = false; };

  // ---- Movement  -----

  void UpdateLastMovingTime();

  bool HasGroundUnderFeets() const;
  bool CanJump() const { return HasGroundUnderFeets(); };

  void StopChangingWeaponAngle();

  // Jumps
  void Jump(double strength, double angle);
  void Jump();
  void HighJump();
  void BackJump();

  // Direction of the character ( -1 == looks to the left / +1 == looks to the right)
  void SetDirection(LRDirection direction);
  LRDirection GetDirection() const;

  // Team owner
  const Team& GetTeam() const { return m_team; };
  Team& AccessTeam() const { return m_team; };
  uint GetTeamIndex() const;
  uint GetCharacterIndex() const;

  // Access to character info
  const std::string& GetName() const;
  bool IsSameAs(const Character& other) const { return (GetName() == other.GetName()); }
  void SetCustomName(const std::string name);
   // Hand position
  void GetHandPosition(Point2i & result) const;
  void GetRelativeHandPosition(Point2i & result) const;

  // Damage report
  const DamageStatistics* GetDamageStats() const { return damage_stats; };
  void ResetDamageStats();

  // Body handling
  Body * GetBody() { return body; };
  void SetWeaponClothe();

  // "force" option forces to apply the clothe/movement even if character is dead
  void SetClothe(const std::string& name, bool force=false);
  void SetMovement(const std::string& name, bool force=false);
  void SetClotheOnce(const std::string& name, bool force=false);
  void SetMovementOnce(const std::string& name, bool force=false);

  // Keyboard handling
  void HandleKeyPressed_MoveRight(bool slowly);
  void HandleKeyReleased_MoveRight(bool slowly);

  void HandleKeyPressed_MoveLeft(bool slowly);
  void HandleKeyReleased_MoveLeft(bool slowly);

  void HandleKeyPressed_Up(bool slowly);
  void HandleKeyReleased_Up(bool slowly);

  void HandleKeyPressed_Down(bool slowly);
  void HandleKeyReleased_Down(bool slowly);

  void HandleKeyPressed_Jump();
  void HandleKeyReleased_Jump() const {};

  void HandleKeyPressed_HighJump();
  void HandleKeyReleased_HighJump() const { };

  void HandleKeyPressed_BackJump();
  void HandleKeyReleased_BackJump() const {};

};

#endif
