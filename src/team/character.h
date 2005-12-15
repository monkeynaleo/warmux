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

#ifndef CHARACTER_H
#define CHARACTER_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../object/physical_obj.h"
#include "../gui/progress_bar.h"
#include "skin.h"
#include <string>
#include <SDL.h>
#include "../graphic/sprite.h"
#include "../graphic/text.h"
//-----------------------------------------------------------------------------

// Prédéfinition de la classe équipe
class Team;
//-----------------------------------------------------------------------------

// Un ver de terre :-)
class Character : public PhysicalObj
{
private:
  Team *m_team;
  bool desactive;       // Désactive le ver ? (ni affichage, ni gestion)
  bool is_walking;

  // energie
  uint energy;
  BarreProg energy_bar;

  // name
  Text* name_text;

  // chrono
  uint pause_bouge_dg;  // pause pour mouvement droite/gauche
  int losted_energy;

  Skin *skin;
  CfgSkin_Walking *walk_skin;
  std::string current_skin;

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
  void DrawEnergyBar (int dy) const;
  void DrawName (int dy) const;

  virtual void SignalDeath();
  virtual void SignalDrowning();
  virtual void SignalGhostState (bool was_dead);
  virtual void SignalFallEnding();

public:
  // Initialise les variables du ver
  Character ();

  // (Re)Initialise le ver
  void Init();
  void Reset();
  void InitTeam (Team *equipe, const std::string &nom, 
		 Skin *skin);
  void DebutPartie();

  // Change le niveau d'énergie
  void SetEnergyDelta (int delta);

  // Changement de direction
  void SetDirection (int direction);

  // Initialise un mouvement à droite ou à gauche
  void InitMouvementDG (uint pause);
  bool CanStillMoveDG (uint pause);

  // Changement d'etat
  void SuperSaut ();
  void Saute ();

  void Draw();
  void Refresh();

  // Handle a key event on the character.
  void HandleKeyEvent(int key, int event_type) ;
  void DoShoot() ;
  void HandleShoot(int event_type) ;

  // Se prepare a un nouveau tour
  void PrepareTour ();

  // Désactive/Réactive le ver
  void Desactive();
  void Reactive();
  void Hide();
  void Show();

  // Un ver est actif ? (affiché et ses données sont calculées ?)
  bool IsActive() const; 

  // Les mouvements droite/gauche sont autorisés ? (pause assez longue ?)
  bool MouvementDG_Autorise() const;
  bool CanJump() const;
  void FrameImageSuivante ();

  // Lecture du niveau d'énergie en pourcent
  uint GetEnergy() const;

  // Lecture de la direction d'un (+1 à droite, -1 à gauche)
  int GetDirection() const;

  // Acces à l'équipe d'un ver
  Team& TeamAccess();
  const Team& GetTeam() const;

  // Accès à l'avatar
  const Skin& GetSkin() const;
  Skin& AccessSkin();
  void SetSkin(std::string skin_name);

  // Hand position
  void GetHandPosition (int &x, int &y);
  void GetHandPositionf (double &x, double &y);

  void EndTurn();
};

//-----------------------------------------------------------------------------
#endif
