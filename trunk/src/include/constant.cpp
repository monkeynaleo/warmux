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
 *  Constants.
 *****************************************************************************/

#include "constant.h"

const std::string VERSION("0.7beta3 (" __DATE__ ")");

#ifndef INSTALL_DATADIR
#  define INSTALL_DATADIR "data/"
#endif

// Installation directory (with slash)
const std::string DEFAULT_DATADIR(INSTALL_DATADIR);

#ifndef INSTALL_LOCALEDIR
#  define INSTALL_LOCALEDIR "locale/"
#endif

// Installation diretory (with slash)
const std::string DEFAULT_LOCALEDIR(INSTALL_LOCALEDIR);

// Nombre de boucles maximum pour le test ...
const uint NBR_BCL_MAX_EST_VIDE = 200; // Un objet est dans le vide ?

std::vector<std::string> AUTHORS;

const std::string WEB_SITE("http://www.wormux.org");
const std::string EMAIL("wormux-dev@gna.org");

// Dimensions min/max du terrain (en pixel)
const Point2i MAP_MIN_SIZE(100, 200);
const int MAP_MAX_SIZE = 4000*4000;

// Hauteur minimale libre pour que le terrain soit qualifié " d'ouvert "
const uint HAUT_MIN_TERRAIN_OUVERT = 40;

void InitConstants(){
  AUTHORS.push_back ("Lawrence AZZOUG");
  AUTHORS.push_back ("Laurent DEFERT SIMONNEAU");
  AUTHORS.push_back ("Jean-Christophe DUBERGA");
  AUTHORS.push_back ("Matthieu FERTRE");
  AUTHORS.push_back ("Renaud LOTTIAUX");
  AUTHORS.push_back ("Victor STINNER");
}
