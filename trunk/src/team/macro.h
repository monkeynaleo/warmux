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
 * Macros pour la gestion des diff�rentes �quipes.
 *****************************************************************************/

#ifndef TEAM_MACRO_H
#define TEAM_MACRO_H
//-----------------------------------------------------------------------------
#include "teams_list.h"
//-----------------------------------------------------------------------------

// Boucle pour chaque equipe
#define POUR_CHAQUE_EQUIPE(equipe) \
  for (TeamsList::iterator equipe=teams_list.list.begin(), \
       fin_pour_chaque_equipe=teams_list.list.end(); \
       equipe != fin_pour_chaque_equipe; \
       ++equipe)

// Boucle pour chaque ver d'une equipe (sauf les fantomes)
#define POUR_CHAQUE_VER(equipe,ver) \
  for (Team::iterator ver = (*(equipe)).begin(), \
       fin_pour_chaque_ver = (*(equipe)).end(); \
       ver != fin_pour_chaque_ver; \
       ++ver) \
  if (ver -> IsActive())

// Boucle pour chaque ver vivant d'une equipe
#define POUR_CHAQUE_VER_VIVANT(equipe,ver) \
  for (Team::iterator ver = (*(equipe)).begin(), \
       fin_pour_chaque_ver_vivant = (*(equipe)).end(); \
       ver != fin_pour_chaque_ver_vivant; \
       ++ver) \
  if (!ver -> IsDead())

// Boucle pour tous les vers (or fantomes)
#define POUR_TOUS_VERS(equipe,ver) \
  POUR_CHAQUE_EQUIPE(equipe) \
  POUR_CHAQUE_VER(*equipe,ver) 

// Boucle pour tous les vers vivants
#define POUR_TOUS_VERS_VIVANTS(equipe,ver) \
  POUR_CHAQUE_EQUIPE(equipe) \
  POUR_CHAQUE_VER_VIVANT(*equipe,ver) 

//-----------------------------------------------------------------------------
#endif
