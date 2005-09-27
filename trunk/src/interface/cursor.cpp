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
 * Curseur clignotant montrant la position d'un ver actif.
 *****************************************************************************/

#include "cursor.h"
//-----------------------------------------------------------------------------
#ifndef CL
#include <SDL.h>
#include "../tool/resource_manager.h"
#endif
//#include "../graphic/graphism.h"
#include "../team/teams_list.h" // ActiveCharacter()
#include "../game/time.h"
#include "../game/game_loop.h"
#include "../object/physical_obj.h"
#include "../include/app.h"
#include "../tool/Point.h"

using namespace Wormux;
//-----------------------------------------------------------------------------

// Curseur clignotant
#define NBR_CLIGNOTEMENT_CURSEUR 3
#define TEMPS_CURSEUR 700 // ms

// Curseur fleche
#define NBR_BOUCLE_FLECHE		4
#define TEMPS_CURSEUR_FLECHE_M		30
#define TEMPS_CURSEUR_FLECHE_D		25
#define HAUTEUR_FLECHE	70

// Déplacement relatif du curseur par rapport à la position du ver
#define CURSEUR_VER_DX -28
#define CURSEUR_VER_DY -37

//-----------------------------------------------------------------------------
CurseurVer curseur_ver;
//-----------------------------------------------------------------------------

CurseurVer::CurseurVer()
{
  temps = Wormux::temps.Lit();
  Cache();
}

//-----------------------------------------------------------------------------

// Dessine le curseur
void CurseurVer::Draw()
{
  if (!actif) return;
  if (!affiche) return;
  if (obj_designe == NULL) return;
  if (obj_designe -> IsGhost()) return;

  // Dessine le curseur autour du ver
#ifdef CL
  CL_Point centre = obj_designe -> GetCenter();
#else
  Point2i centre = obj_designe->GetCenter();
#endif
  uint x = centre.x - image->GetWidth()/2;
  uint y = centre.y - image->GetHeight()/2 + y_mouvement;
#ifdef CL
   image.draw (x, y);
#else
   image->Draw(x,y); 
#endif
}

//-----------------------------------------------------------------------------

void CurseurVer::Refresh()
{
  if (!actif) return;

    // Dessine une fleche au dessus du ver
  if (nbr_boucle > 0) {
    affiche = true;
    if (y_mouvement > HAUTEUR_FLECHE+30)
    {
      monter = false;
      nbr_boucle--;
    }
    else if (y_mouvement < HAUTEUR_FLECHE)
      monter = true;

    if (monter)
    {
      if (TEMPS_CURSEUR_FLECHE_M <= Wormux::temps.Lit() - temps)
      {
        temps = Wormux::temps.Lit();
        y_mouvement++;
      }
    } else {
      if (TEMPS_CURSEUR_FLECHE_D <= Wormux::temps.Lit() - temps)
      {
        temps = Wormux::temps.Lit();
	y_mouvement--;
      }
    }
  } else {
    affiche = false;
  }

  if (affiche) 
  {
  }

#if 0
  // Dessine le curseur autour du ver
  if (clignote)
  {
    if (Wormux::temps.Lit() - temps <= TEMPS_CURSEUR) {
      affiche = true;
    } else {
      affiche = false;
      if (TEMPS_CURSEUR*2 < Wormux::temps.Lit() - temps) 
      {
	temps = Wormux::temps.Lit();
	nbr_clignot++;
      }
    }

    if (NBR_CLIGNOTEMENT_CURSEUR <= nbr_clignot) actif = false;
  }
#endif
}

//-----------------------------------------------------------------------------

// Cache le curseur
void CurseurVer::Cache()
{
  y_mouvement = HAUTEUR_FLECHE;
  //monter = false;
  nbr_boucle = 0;
  nbr_clignot = NBR_CLIGNOTEMENT_CURSEUR;
  affiche = false;
}

//-----------------------------------------------------------------------------

void CurseurVer::Init()
{
#ifdef CL
  CL_ResourceManager *res=graphisme.LitRes();
  image = CL_Sprite("gfx/curseur", res);
#else
   Profile *res = resource_manager.LoadXMLProfile( "graphism.xml");
   image = resource_manager.LoadSprite( res, "gfx/curseur");
#endif
}

//-----------------------------------------------------------------------------

void CurseurVer::Reset()
{
  actif = false;
  affiche = false;
  obj_designe = NULL;
  designe_ver_actif = false;

  y_mouvement = HAUTEUR_FLECHE;
  monter = false;
}

//-----------------------------------------------------------------------------

void CurseurVer::SuitVerActif()
{
  obj_designe = &ActiveCharacter();
  designe_ver_actif = true;
  nbr_clignot = 0;
  affiche = true;
  actif = true;
  temps = Wormux::temps.Lit();
  clignote = true;
  nbr_boucle = NBR_BOUCLE_FLECHE;
#ifdef CL
  image.set_frame (1);
#else
   image->SetCurrentFrame(1);
#endif
}

//-----------------------------------------------------------------------------

void CurseurVer::PointeAucunObj ()
{
  if (designe_ver_actif) return;
  actif = false;
}

//-----------------------------------------------------------------------------

void CurseurVer::PointeObj (PhysicalObj *obj)
{
  if (obj == &ActiveCharacter()) 
  {
    SuitVerActif();
    return;
  }
  designe_ver_actif = false;
  obj_designe = obj;
  affiche = true;  
  actif = true;
  clignote = false;
  nbr_boucle = NBR_BOUCLE_FLECHE;

  const Character* character = dynamic_cast<const Character*> (obj_designe);

#ifdef CL
   if (game_loop.character_already_chosen
      || ((character != NULL) && (&character -> GetTeam() != &ActiveTeam())))
    image.set_frame (2);
  else
    image.set_frame (0);
#else
   if (game_loop.character_already_chosen
      || ((character != NULL) && (&character -> GetTeam() != &ActiveTeam())))
    image->SetCurrentFrame (2);
  else
    image->SetCurrentFrame (0);
#endif
}

//-----------------------------------------------------------------------------
