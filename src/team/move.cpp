/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either charactersion 2 of the License, or
 *  (at your option) any later charactersion.
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
 * Mouvement droite/gauche pour un character.
 *****************************************************************************/

#include "move.h"
//-----------------------------------------------------------------------------
#include <math.h>
#include "../include/action_handler.h"
#include "../interface/cursor.h"
#include "../game/config.h"
#include "../game/game_loop.h"
#include "../map/map.h"
#include "../map/camera.h"
#include "../sound/jukebox.h"
#include "teams_list.h"
using namespace Wormux;
//-----------------------------------------------------------------------------

#ifdef DEBUG
#  define DEBUG_FANTOME
#  define COUT_DBG std::cout << "[Mouvement DG] "
#endif

// Hauteur qu'un character peut grimper au maximum
#define HAUTEUR_GRIMPE_MAX 30

// Hauteur qu'un character peut chute (sans appeler la fonction Gravite)
#define HAUTEUR_CHUTE_MAX 20

// Pause entre deux deplacement
#define PAUSE_CHG_SENS 80 // ms

//-----------------------------------------------------------------------------

// Calcule la hauteur a chuter ou grimper lors d'un déplacement horizontal
// Renvoie si le mouvement est possible
bool CalculeHauteurBouge (Character &character, int &hauteur)
{
  int y_floor=character.GetY();

  // On peut descendre ?
  for (hauteur=0;
       character.IsInVacuum(character.GetDirection(), hauteur)
       && !character.FootsOnFloor(y_floor+hauteur);
       ++hauteur)
  {
    // C'est trop haut pour grimper :-p
    if (HAUTEUR_CHUTE_MAX < hauteur) 
    {
      // Alors le character chute (va dans le vide)
      character.SetX (character.GetX() +character.GetDirection());
      character.UpdatePosition();
      return false;
    }
  }
  if (hauteur != 0) return true;

  // On est libre : ok, bye !
  if (character.IsInVacuum(character.GetDirection(), -1)) return true;

  // Calcule la hauteur a grimper
  for (hauteur=-1;
/*       EstDansVide_haut(character,character.GetDirection(), hauteur)
	   && !EstDansVide_bas(character,character.GetDirection(), hauteur); */
       !character.IsInVacuum(character.GetDirection(), hauteur-1)
	 && monde.EstDansVide_haut(character,character.GetDirection(), hauteur-1);
       --hauteur)
  {
    // C'est trop haut pour grimper :-p
    if (hauteur < -HAUTEUR_GRIMPE_MAX) 
    {
      return false;
    }
  }
  if (!character.IsInVacuum(character.GetDirection(), hauteur-1)) return false;
  return true;
}

//-----------------------------------------------------------------------------

// Bouge un character characters la droite ou la gauche (selon le signe de direction)
void MoveCharacter (Character &character) 
{
  int hauteur;
  bool fantome;

  // Désactive le curseur
  curseur_ver.Cache();

  // On est bien dans le monde ? (sinon, pas besoin de tester !)
  if (character.GetDirection() == -1)
    fantome = character.IsOutsideWorld (-1, 0);
  else
    fantome = character.IsOutsideWorld (1, 0);
  if (fantome) 
  {
#ifdef DEBUG_FANTOME
    COUT_DBG << character.m_name << " devient un fantome." << std::endl;
#endif
    character.Ghost();
    return;
  }

  // Calcule la hauteur a descendre
  if (!CalculeHauteurBouge (character, hauteur)) return;

  do
  {
    // Bouge !
    game_loop.character_already_chosen = true;
    // Deplace enfin le character

    character.SetXY (character.GetX() +character.GetDirection(),
		character.GetY() +hauteur);

    // Gravite (s'il n'y a pas eu de collision
    character.UpdatePosition();

    // Passe a l'image suivante
    character.FrameImageSuivante();

  }
  while(character.CanStillMoveDG(PAUSE_BOUGE) && CalculeHauteurBouge (character, hauteur));

//    character.UpdatePosition();

}
//-----------------------------------------------------------------------------

// Move a character to the left
void MoveCharacterLeft (Character &character) 
{
  // Le character est pret a bouger ?
  if (!character.MouvementDG_Autorise()) return;

  bool bouge = (character.GetDirection() == -1);
  if (bouge) 
//    MoveCharacter (character); 
    action_handler.NewAction(Action(ACTION_WALK));

  else
  {
//    character.SetDirection(-1);
    action_handler.NewAction(ActionInt(ACTION_SET_CHARACTER_DIRECTION,-1));
    character.InitMouvementDG (PAUSE_CHG_SENS);
  }
}

//-----------------------------------------------------------------------------

// Move a character to the right
void MoveCharacterRight (Character &character) 
{ 
  // Le character est pret a bouger ?
  if (!character.MouvementDG_Autorise()) return;

  bool bouge = (character.GetDirection() == 1);
  if (bouge) 
//    MoveCharacter (character);
    action_handler.NewAction(Action(ACTION_WALK));
  else
  {
    action_handler.NewAction(ActionInt(ACTION_SET_CHARACTER_DIRECTION,1));
//    character.SetDirection(1);
    character.InitMouvementDG (PAUSE_CHG_SENS);
  }
}

//-----------------------------------------------------------------------------
