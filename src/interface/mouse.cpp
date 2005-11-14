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
 * Mouse management
 *****************************************************************************/

#include "mouse.h"
//-----------------------------------------------------------------------------
#include "../team/macro.h"
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../map/camera.h"
# include "../graphic/video.h"
#include "../include/constant.h" // FOND_X, FOND_Y
#include "interface.h"
#include "cursor.h"
#include "../weapon/weapon.h"
#ifdef CL
# include "../tool/geometry_tools.h"
#else
#include "../tool/Point.h"
#include "../tool/Distance.h"
#endif

using namespace Wormux;
//-----------------------------------------------------------------------------
Mouse mouse;
//-----------------------------------------------------------------------------

// Vitesse du definalement à la souris
const uint SCROLL_MOUSE = 20;

// Largeur de la zone de sensibilite au camera a la souris
const uint SENSIT_SCROLL_MOUSE = 40; // pixels

// Active le mode tricheur ?
#ifdef DEBUG
#  define MODE_TRICHEUR
#endif

//-----------------------------------------------------------------------------

Mouse::Mouse()
{
  scroll_actif = false;
}


//-----------------------------------------------------------------------------

void Mouse::Reset()
{}

//-----------------------------------------------------------------------------

#ifdef CL

#ifdef PORT_CL07

bool Mouse::ClicG() const { return CL_Mouse::left_pressed(); }
bool Mouse::ClicD() const { return CL_Mouse::right_pressed(); }
bool Mouse::ClicM() const { return CL_Mouse::middle_pressed(); }

#else

bool Mouse::ClicG() const { return false; }
bool Mouse::ClicD() const { return false; }
bool Mouse::ClicM() const { return false; }

#endif

#endif // CL defined

//-----------------------------------------------------------------------------

bool Mouse::ActionClicD()
{ 
  if ( ActiveTeam().GetWeapon().CanChangeWeapon() )
  {
    interface.weapons_menu.ChangeAffichage();
  }
  return true;
}

//-----------------------------------------------------------------------------

bool Mouse::ActionClicG()
{
#ifdef CL
   const CL_Point pos_monde = GetPosMonde();
#else
   const Point2i pos_monde = GetPosMonde();   
#endif

#ifdef CL   
#ifdef MODE_TRICHEUR
  // Control gauche + clic gauche = téléportation !
  if (CL_Keyboard::get_keycode(CL_KEY_CONTROL) && ActiveCharacter().IsReady())
  {
    ActiveCharacter().SetXY (pos_monde.x, pos_monde.y);
    ActiveCharacter().UpdatePosition();
    return true;
  }
#endif
#endif
	
  // Action dans le menu des armes ?
  if (interface.weapons_menu.ActionClic (GetX(),GetY())) 
  {
    return true;
  } 


  // On peut changer de ver ?
  if (game_mode.AllowCharacterSelection())
  {
    // Sélection d'un ver se son équipe ?
    bool ver_choisi=false;
    Team::iterator 
      it=ActiveTeam().begin(),
      fin=ActiveTeam().end();
    uint index=0;
    for (; it != fin; ++it, ++index)
    {
      if (&(*it) != &ActiveCharacter()
	  && !it -> IsDead() 
	  && 
#ifdef CL	  
	  DansRect (it -> GetRect(), pos_monde))
#else
	  IsInside (it -> GetRect(), pos_monde))
#endif
	 {
	ver_choisi = true;
	break;
      }
    }
    if (ver_choisi)
    {
      ActiveTeam().SelectCharacterIndex (index);
      return true;
    }

#ifdef CL
   if (DansRect (ActiveCharacter().GetRect(), pos_monde))
#else
     if (IsInside (ActiveCharacter().GetRect(), pos_monde))
#endif
     {
      curseur_ver.SuitVerActif();
      return true;
    }
  }

  // Action dans le menu des armes ?
  if (interface.weapons_menu.ActionClic (GetX(),GetY())) 
  {
    return true;
  }

  // Choosing target for a weapon, many posibilities :
  // - Do nothing
  // - Choose a target but don't fire
  // - Choose a target and fire it !
  if (game_loop.ReadState() == gamePLAYING) {
    ActiveTeam().AccessWeapon().ChooseTarget();
    return true ;
  }
  
  return false;
}

//-----------------------------------------------------------------------------

void Mouse::ChoixVerPointe()
{
  if (game_loop.ReadState() != gamePLAYING) return;

#ifdef CL
  const CL_Point pos_monde = GetPosMonde();
#else
  const Point2i pos_monde = GetPosMonde();;
#endif
   
  // Quel ver est pointé par la souris ? (en dehors du ver actif)
  interface.ver_pointe_souris = NULL;
  POUR_TOUS_VERS_VIVANTS(equipe,ver)
  {
    if ((&(*ver) != &ActiveCharacter())
#ifdef CL
	&& DansRect (ver -> GetRect(), pos_monde))
#else
    	&& IsInside (ver -> GetRect(), pos_monde))   
#endif
    {
      interface.ver_pointe_souris = &(*ver);
    }
  }

  // Aucun ver n'est pointé ... et le ver actif alors ?
  if ((interface.ver_pointe_souris == NULL)
      && 
#ifdef CL
      DansRect (ActiveCharacter().GetRect(), pos_monde))
#else
      IsInside (ActiveCharacter().GetRect(), pos_monde))  
#endif
{
    interface.ver_pointe_souris = &ActiveCharacter();
  }

  // Dessine le curseur autour du ver pointé s'il y en a un
  if (interface.ver_pointe_souris != NULL) {
    curseur_ver.PointeObj (interface.ver_pointe_souris);
  } else {
    curseur_ver.PointeAucunObj();
  }
}

//-----------------------------------------------------------------------------
 
void Mouse::ScrollCamera() const
{
  //Move camera with mouse when cursor is on border of the screen
  if (uint(GetX()) < SENSIT_SCROLL_MOUSE) {
    camera.SetXY (-SCROLL_MOUSE, 0);
    camera.autorecadre = false;
  }

  if (uint(GetX()) > video.GetWidth() - SENSIT_SCROLL_MOUSE) {
    camera.SetXY (SCROLL_MOUSE, 0);
    camera.autorecadre = false;
  }

  if (uint(GetY()) < SENSIT_SCROLL_MOUSE) {
    camera.SetXY (0, -SCROLL_MOUSE);
    camera.autorecadre = false;
  }

  if (uint(GetY()) > video.GetHeight() - SENSIT_SCROLL_MOUSE) {
    camera.SetXY (0,SCROLL_MOUSE);
    camera.autorecadre = false;
  }
}

void Mouse::TestCamera()
{
#ifndef CL
   int _x, _y;       
   SDL_GetMouseState( &_x, &_y);
#endif
   
   //Move camera with mouse holding Ctrl key down
#ifdef CL
   const bool demande_scroll = CL_Keyboard::get_keycode(CL_KEY_CONTROL);
#else
   const bool demande_scroll = SDL_GetModState() & KMOD_CTRL;
#endif
   
   if (demande_scroll)
  {
    if (scroll_actif) {
#ifdef CL
      int dx = sauve_x - CL_Mouse::get_x();
      int dy = sauve_y - CL_Mouse::get_y();
#else
       int dx = sauve_x - _x;
       int dy = sauve_y - _y;
#endif 
      camera.SetXY (dx, dy);
      camera.autorecadre = false;
    } else {
      scroll_actif = true;
    }
#ifdef CL
    sauve_x = CL_Mouse::get_x();
    sauve_y = CL_Mouse::get_y();
#else
    sauve_x = _x;
    sauve_y = _y;
#endif
     return;
  } else {
    scroll_actif = false;
  }

  if(!interface.weapons_menu.IsDisplayed()) ScrollCamera();
}

//-----------------------------------------------------------------------------

void Mouse::Refresh()
{
  if (!scroll_actif) ChoixVerPointe();
}

//-----------------------------------------------------------------------------

int Mouse::GetX() const 
{
#ifdef CL
   return CL_Mouse::get_x(); 
#else
   int x;
   
   SDL_GetMouseState( &x, NULL);
   return x; 
#endif
}

int Mouse::GetY() const 
{ 
#ifdef CL
   return CL_Mouse::get_y(); 
#else
   int y;
   
   SDL_GetMouseState( NULL, &y);
   return y; 
#endif

}

int Mouse::GetXmonde() const 
{ 
   return GetX() -FOND_X +camera.GetX(); 
}

int Mouse::GetYmonde() const 
{ 
   return GetY() -FOND_Y +camera.GetY(); 
}

#ifdef CL
CL_Point Mouse::GetPosMonde() const
{ 
   return CL_Point (GetXmonde(), GetYmonde()); 
}
#else
Point2i Mouse::GetPosMonde() const
{ 
   return Point2i (GetXmonde(), GetYmonde());
}
#endif


//-----------------------------------------------------------------------------

#ifdef CL
void Mouse::DesinstallePilote()
{
  assert (pilote_installe);
  CL_Mouse::sig_key_up().disconnect(slot);
  pilote_installe = false;
}
#endif

//-----------------------------------------------------------------------------

#ifdef CL
void Mouse::InstallePilote()
{
  assert (!pilote_installe);
  slot = CL_Mouse::sig_key_up().connect(this, &Mouse::TraiteClic);
  pilote_installe = true;
}
#endif

//-----------------------------------------------------------------------------

// Traite une touche relachée
#ifdef CL
void Mouse::TraiteClic (const CL_InputEvent &event)
{
  // Clic gauche de la mouse ?
  if (event.id == CL_MOUSE_RIGHT)
  {
    ActionClicD();
    return;
  }

  // Clic gauche de la souris ?
  if (event.id == CL_MOUSE_LEFT)
  {
    ActionClicG();
    return;
  }
}
#else
void Mouse::TraiteClic (const SDL_Event *event)
{
   if ( event->type == SDL_MOUSEBUTTONDOWN )
     {
	
	if ( event->button.button == SDL_BUTTON_RIGHT)
	  {
	     ActionClicD();
	     return;
	  }
	
	// Clic gauche de la souris ?
	if ( event->button.button == SDL_BUTTON_LEFT)
	  {
	     ActionClicG();
	     return;
	  }
     }
}
#endif

//-----------------------------------------------------------------------------
