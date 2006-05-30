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
#include "skin.h"
#include "team.h"
#include "../gui/progress_bar.h"
#include "../graphic/sprite.h"
#include "../graphic/text.h"
#include "../include/base.h"
#include "../object/physical_obj.h"

class Team;

// Un ver de terre :-)
class Character : public PhysicalObj
{
private:
  std::string character_name;
  Team& m_team;
  bool desactive;       // Disable the characte ? (no display, nor handling)
  bool skin_is_walking; // True if the curent is a walking skin.
  bool is_walking;
  bool full_walk;

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
  int lost_energy;

  Skin *skin;
  CfgSkin_Walking *walk_skin;
 public:
  std::string current_skin;
 private:
  // Animation
  struct s_anim{
    Sprite *image;
    bool draw;
    uint time; // Time for next animation
  } anim;

  uint m_image_frame; // Current image frame
  uint m_frame_repetition; // Number of frame repetition (used for walking)
  bool hidden; //The character is hidden (needed by teleportation)

  // Channel used for sound
  int channel_step;

public:

  Sprite *image;

  // Previous strength 
  double previous_strength;

private:
  void DrawEnergyBar (int dy);
  void DrawName (int dy) const;
  void StartBreathing();
  void StartWalking();
  void StopWalking();

  virtual void SignalDeath();
  virtual void SignalDrowning();
  virtual void SignalGhostState (bool was_dead);
  virtual void SignalFallEnding();

public:
  Character (Team& my_team, const std::string &name, 
	     Skin *skin);

  // (Re)Initialise le ver
  void Reset();

  // Change le niveau d'�nergie
  void SetEnergyDelta (int delta, bool do_report=true);

  // Changement de direction
  void SetDirection (int direction);

  // Initialise un mouvement � droite ou � gauche
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

  // Un ver est actif ? (affich� et ses donn�es sont calcul�es ?)
  bool IsActive() const; 

  // Les mouvements droite/gauche sont autoris�s ? (pause assez longue ?)
  bool MouvementDG_Autorise() const;
  bool CanJump() const;
  void FrameImageSuivante ();

  // Lecture du niveau d'�nergie en pourcent
  uint GetEnergy() const;

  // Lecture de la direction d'un (+1 � droite, -1 � gauche)
  int GetDirection() const;

  // Acces � l'�quipe d'un ver
  Team& TeamAccess();
  const Team& GetTeam() const;

  // Access to character info
  const std::string& GetName() const { return character_name; }
  bool IsSameAs(const Character& other) { return (GetName() == other.GetName()); }

  // Acc�s � l'avatar
  const Skin& GetSkin() const;
  Skin& AccessSkin();
  bool SetSkin(const std::string& skin_name);

  // Hand position
  Point2i GetHandPosition();
  void GetHandPositionf (double &x, double &y);

  // Damage report
  void HandleMostDamage();
  void MadeDamage(const int Dmg, const Character &other);
  int  GetMostDamage() { HandleMostDamage(); return max_damage; }
  int  GetOwnDamage() { return damage_own_team; }
  int  GetOtherDamage() { return damage_other_team; }
};

#endif
