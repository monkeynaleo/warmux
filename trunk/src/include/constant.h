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
 * Diverses constantes.
 *****************************************************************************/

#ifndef CONSTANTE_H
#define CONSTANTE_H
//-----------------------------------------------------------------------------
#include "base.h"
#include <string>
#include <vector>
//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

void InitCst();

// Numéro de version de Wormux
//extern const std::string VERSION;

// Répertoire d'installation (avec slash)
extern const std::string DEFAULT_DATADIR;
extern const std::string DEFAULT_LOCALEDIR;

// Taille (et position) de l'image de fond (et du plateau de jeu)
extern uint FOND_X;
extern uint FOND_Y;

// Nombre de boucles maximum pour le test ...
extern const uint NBR_BCL_MAX_EST_VIDE; // Un objet est dans le vide ?

// Liste des auteurs
extern std::vector<std::string> AUTHORS;

// Web site address
extern const std::string WEB_SITE;

//-----------------------------------------------------------------------------

// Dimensions min/max du terrain (en pixel)
extern const uint LARG_MIN_TERRAIN;
extern const uint HAUT_MIN_TERRAIN;
extern const uint TAILLE_MAX_TERRAIN;

// Taille des cellules du terrain
extern const uint LARG_CELLULE_TERRAIN;
extern const uint HAUT_CELLULE_TERRAIN;

// Hauteur (en pixel) minimale libre pour que le terrain
// soit qualifié " d'ouvert "
extern const uint HAUT_MIN_TERRAIN_OUVERT;

//-----------------------------------------------------------------------------
}
#endif
