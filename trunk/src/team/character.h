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
 * Character of a team.
 *****************************************************************************/

#ifndef _CHARACTER_H
#define _CHARACTER_H

#include <string>
#include <SDL.h>
#include "body.h"
#include "team.h"
#include "../gui/progress_bar.h"
#include "../graphic/sprite.h"
#include "../graphic/text.h"
#include "../include/base.h"
#include "../object/physical_obj.h"

class Body;
class Team;

// Un ver de terre :-)
class Character : public PhysicalObj
{
private:
  std::string character_name;
  Team& m_team;
  bool is_walking;

  // energy
  uint energy;
  int  damage_other_team;
  int  damage_own_team;
  int  max_damage;
  int  current_total_damage;
  BarreProg energy_bar;

  // survived games
  int survivals;

  // name
  Text* name_text;

  // chrono
  uint pause_bouge_dg;  // pause pour mouvement droite/gauche
  uint do_nothing_time;
  uint animation_time;
  int lost_energy;
  bool hidden; //The character is hidden (needed by teleportation)

  // Channel used for sound
  int channel_step;

public:

  // Previous strength 
  double previous_strength;

  Body* body;

private:
  void DrawEnergyBar (int dy);
  void DrawName (int dy) const;

  virtual void SignalDeath();
  virtual void SignalDrowning();
  virtual void SignalGhostState (bool was_dead);
  virtual void SignalFallEnding();

public:

  Character (Team& my_team, const std::string &name);
  ~Character();

  virtual void SignalExplosion();

  // Change le niveau d'�ergie
  void SetEnergyDelta (int delta, bool do_report=true);

  // Changement de direction
  void SetDirection (int direction);

  // Initialise un mouvement �droite ou �gauche
  void InitMouvementDG (uint pause);
  bool CanStillMoveDG (uint pause);
  bool IsWalking () const { return is_walking; };

  // Changement d'etat
  void HighJump ();
  void Jump ();

  void Draw();
  void Refresh();

  // Handle a key event on the character.
  void HandleKeyEvent(int key, int event_type) ;
  void DoShoot() ;
  void HandleShoot(int event_type) ;

  // Se prepare a un nouveau tour
  void PrepareTurn ();

  // Show hide the Character
  void StartPlaying();
  void StopPlaying();
  void Hide();
  void Show();

  // Les mouvements droite/gauche sont autoris� ? (pause assez longue ?)
  bool MouvementDG_Autorise() const;
  bool CanJump() const;

  // Lecture du niveau d'�ergie en pourcent
  uint GetEnergy() const;

  // Lecture de la direction d'un (+1 �droite, -1 �gauche)
  int GetDirection() const;

  // Acces �l'�uipe d'un ver
  Team& TeamAccess();
  const Team& GetTeam() const;

  // Access to character info
  const std::string& GetName() const { return character_name; }
  bool IsSameAs(const Character& other) { return (GetName() == other.GetName()); }

  // Hand position
  Point2i GetHandPosition();
  void GetHandPositionf (double &x, double &y);

  // Damage report
  void HandleMostDamage();
  void MadeDamage(const int Dmg, const Character &other);
  int  GetMostDamage() const { return max_damage; }
  int  GetOwnDamage() const { return damage_own_team; }
  int  GetOtherDamage() const { return damage_other_team; }

  // Physics handling
  virtual bool IsInVacuumXY(const Point2i &position) const;
  virtual bool FootsInVacuumXY(const Point2i &position) const;

  // Body handling
  void SetBody(Body* _body);
  void SetClothe(std::string name) { body->SetClothe(name); };
  void SetMovement(std::string name);
  void SetClotheOnce(std::string name) { body->SetClotheOnce(name); };
  void SetMovementOnce(std::string name);
};

#endif
