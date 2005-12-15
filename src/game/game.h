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
 * Classe principale qui gêre le jeu : initialisation, dessin, gestion
 * des différents composants, et boucle de jeu.
 *****************************************************************************/

#ifndef JEU_H
#define JEU_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../team/character.h"
#include "../gui/question.h"
//-----------------------------------------------------------------------------

class Jeu
{
private:
  bool jeu_lance;
  Question question;

public:
  bool initialise;
  volatile bool fin_partie;

public:
  // Initialisation des données (valeurs par défaut)
  Jeu();

  // Est-ce la fin d'une partie ?
  bool EstFinPartie();

  // Messages ...
  void MsgChargement();
  void MsgFinPartie();

  // Lance le jeu
  void LanceJeu();

  // Change le tour d'un tour (en informe l'interface)
  void ChangeTpsTour (uint nv_tps_tour);

  bool JeuEstLance() const { return jeu_lance; }


  // Mode pause
  void Pause();

  int PoseQuestion (bool dessine=true);
};

extern Jeu jeu;
//-----------------------------------------------------------------------------
#endif
