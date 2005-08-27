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
 * Classes aidant au débogage.
 *****************************************************************************/

#include "debug.h"
#ifdef DEBUG
//-----------------------------------------------------------------------------
#include "../interface/mouse.h"
#include "../map/camera.h"
#include "../team/macro.h"
#include "../object/objects_list.h"
#include <sstream>

//#define AFFICHE_COORD_SOURIS
//#define AFFICHE_INFO_OBJ
//#define AFFICHE_INFO_VER
//#define DISPLAY_HAND_POSITION

// Hauteur de la police de caractere "mini"
#define HAUT_POLICE_MINI 12 // pixels

// Interligne police "mini" (pour les messages)
#define INTERLIGNE_MINI 3 // pixels

//-----------------------------------------------------------------------------

#include "../graphic/graphism.h"

namespace Wormux
{
Debug debug;
//-----------------------------------------------------------------------------

Debug::Debug()
{
  repere_x = 0;  
  repere_y = 0;
  repere_x_monde = 0;  
  repere_y_monde = 0;
}

//-----------------------------------------------------------------------------

void Debug::Refresh()
{
  msg.clear();
  std::ostringstream ss;

  if (mouse.ClicD())
  {
    repere_x = mouse.GetX();
    repere_y = mouse.GetY();
    repere_x_monde = mouse.GetXmonde();
    repere_y_monde = mouse.GetYmonde();
  }

  // Coordonnées de la souris
#ifdef AFFICHE_COORD_MOUSE
  int sx = mouse.GetX();
  int sy = mouse.GetY();
  ss << "mouse = " << sx << "," << sy << endl;
  ss << "mouse monde = " << mouse.GetXmonde() << "," 
     << mouse.GetYmonde() << endl;
  ss << "mouse/repère = " << sx-repere_x << "," << sy-repere_y << endl;
  ss << "mouse/repère monde = " << sx-repere_x_monde 
     << "," << sy-repere_y_monde << endl;
  msg.push_back (ss.str());
#endif

#ifdef DISPLAY_HAND_POSITION
  ss.str("");
  int frame = ActiveCharacter().image.get_current_frame();
  ss << "x,y = "
     << ActiveCharacter().AccessSkin().walking.hand_position[frame].dx
     << ","
     << ActiveCharacter().AccessSkin().walking.hand_position[frame].dy
     << std::endl
     << "frame = " << (1+frame) << std::endl;
  msg.push_back (ss.str());
#endif

  // Informations sur les objets
#ifdef AFFICHE_INFO_OBJ
  msg.push_back ("[Objets]");
  POUR_TOUS_OBJETS(obj)
  {
    ss.str("");
    ss << obj -> ptr -> nom << " : "
       << " fantome=" << obj -> ptr -> IsGhost();
    if (!obj -> ptr -> IsGhost()) 
    {
      ss << ",  x=" << obj -> ptr -> GetX() 
	 << ", y=" << obj -> ptr -> GetY();
    }
    ss << ", efface=" << obj -> efface
       << ", pret=" << obj -> ptr -> IsReady();
    msg.push_back (ss.str());
  }
#endif

  // Informations sur les vers
#ifdef AFFICHE_INFO_VER
  msg.push_back ("[Vers]");
  POUR_TOUS_VERS(equipe,ver)
  {
    ss.str("");
    if (&(*ver) == &ActiveCharacter()) ss << "* ";
    ss << "Ver [" << ver -> GetTeam().LitNom() << "] " << ver -> m_name 
       << " : x=" << ver -> GetX() << ", y=" << ver -> GetY() 
       << ", vivant=";
    if (!ver -> IsDead())
      ss << "oui";
    else
      ss << "mort";
    ss << ", pret=" << ver->IsReady()?"oui":"non";
    ss << ", actif=" << ver->IsActive()?"oui":"non";
    msg.push_back (ss.str());
  }
#endif
}

//-----------------------------------------------------------------------------

void Debug::Draw()
{
  // Affichage des messages
  uint x=10, y=40;
  for (std::vector<std::string>::iterator i=msg.begin(), fin=msg.end(); 
       i != fin; ++i)
  {
    police_petit.WriteLeft (x, y, *i);
    y += HAUT_POLICE_MINI+INTERLIGNE_MINI;
  }
}

//-----------------------------------------------------------------------------
}
#endif // DEBUG
