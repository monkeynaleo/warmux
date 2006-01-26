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
 * Caisse de bonus : tombe du ciel après un temps d'apparition aléatoire.
 * Lorsqu'un ver touche la caisse, il peut gagner différents bonus : munition,
 * énergie (gain ou perte!), etc.
 *****************************************************************************/

#include "../object/bonus_box.h"
//-----------------------------------------------------------------------------
#include "../include/constant.h" // NBR_BCL_MAX_EST_VIDE
#include "../map/map.h"
#include "../team/macro.h"
#include "../game/time.h"
#include "../interface/game_msg.h"
#include "../tool/i18n.h"
#include "../tool/random.h"
#include "../tool/resource_manager.h"
#include "../graphic/sprite.h"
#include "../include/app.h"
#include <sstream>
#include <iostream>
//-----------------------------------------------------------------------------

#ifdef DEBUG

//#define APPARAIT_VITE
//#define DEBUG_CADRE_TEST
//#define MSG_DBG_CAISSE
//#define DBG_PLACEMENT

#define COUT_DBG0 std::cout << "[bonus_box]"
#define COUT_DBG COUT_DBG0 " "
#define COUT_PLACEMENT COUT_DBG0 "[placement "<<bcl<<"] "
#endif

// Combien de temps avant qu'on fabrique une caisse ?
#ifdef APPARAIT_VITE
  const uint TEMPS_MIN_CREATION = 1; // secondes
  const uint TEMPS_MAX_CREATION = 3; // secondes
#else
  const uint TEMPS_MIN_CREATION = 10; // secondes
  const uint TEMPS_MAX_CREATION = 5*60; // secondes
#endif

const uint VITESSE_CAISSE = 5; // meter / seconde
const uint VITESSE_REPLI_PARACHUTE = 170; // ms par image
const uint NB_MAX_TRY = 20;

// Bonus offert par la caisse
const uint BONUS_ENERGIE=100;
const uint BONUS_PIEGE=75;
const uint BONUS_TELEPORTE=5;
const uint BONUS_DYNAMITE=10;
const uint BONUS_AERIENNE=1;
const uint BONUS_BAZ_TETE_C=5;

//-----------------------------------------------------------------------------
BonusBox bonus_box;
//-----------------------------------------------------------------------------

BonusBox::BonusBox()
  : PhysicalObj("BonusBox", 0.0)
{
  SetTestRect (29, 29, 63, 6);
  m_allow_negative_y = true;
  enable = false;
  pos_valide = false;
  m_wind_factor = 0.0;
}

//-----------------------------------------------------------------------------

void BonusBox::Init()
{
  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  anim = resource_manager.LoadSprite( res, "objet/caisse");
  SetSize (anim->GetWidth(), anim->GetHeight());
}

//-----------------------------------------------------------------------------

void BonusBox::FreeMem()
{
  delete anim;
}

//-----------------------------------------------------------------------------
void BonusBox::Reset()
{
}

//-----------------------------------------------------------------------------
void BonusBox::Draw()
{ 
  if (!still_visible) return;
  anim->Draw(GetX(), GetY());
}

//-----------------------------------------------------------------------------

// Signale la fin d'une chute
void BonusBox::SignalFallEnding()
{
#ifdef MSG_DBG_CAISSE
  COUT_DBG << "Fin de la chute : parachute=" << parachute << std::endl;
#endif
  if (!parachute) return;

#ifdef MSG_DBG_CAISSE
  COUT_DBG << "Début de l'animation 'repli du parachute'." << std::endl;
#endif
  parachute = false;

  anim->SetCurrentFrame(0);
  anim->Finish();
}

//-----------------------------------------------------------------------------

void BonusBox::AppliqueBonus (Team &equipe, Character &ver)
{
  std::ostringstream txt;

  switch (bonus)
  {
  case bonusTELEPORTE: 
    txt << Format(ngettext(
                "%s team has won %u teleportation.", 
                "%s team has won %u teleportations.", 
                BONUS_TELEPORTE),
            ActiveTeam().GetName().c_str(), BONUS_TELEPORTE);
    equipe.m_nb_ammos[ _("Teleportation") ] += BONUS_TELEPORTE;
    break;

  case bonusENERGIE:
    txt << Format(ngettext(
                "%s has won %u point of energy!",
                "%s has won %u points of energy!",
                BONUS_ENERGIE),
            ver.m_name.c_str(), BONUS_ENERGIE);
    ver.SetEnergyDelta (BONUS_ENERGIE);
    break;

  case bonusPIEGE:
    txt << Format(ngettext(
                "%s has lost %u point of energy.",
                "%s has lost %u points of energy.",
                BONUS_PIEGE),
            ver.m_name.c_str(), BONUS_PIEGE);
    ver.SetEnergyDelta (-BONUS_PIEGE);
    break;

  case bonusAERIENNE:
    txt << Format(ngettext(
                "'%s has won %u air attack",
                "'%s has won %u air attacks",
                BONUS_AERIENNE),
            ActiveTeam().GetName().c_str(), BONUS_AERIENNE);
    equipe.m_nb_ammos[ _("Air attack") ] += BONUS_AERIENNE;
    break;

  case bonusBAZ_TETE_C:
    txt << Format(ngettext(
                "%s team has won %u automatic bazooka!",
                "%s team has won %u automatic bazookas!",
                BONUS_BAZ_TETE_C),
		  ActiveTeam().GetName().c_str(), BONUS_BAZ_TETE_C);
    equipe.m_nb_ammos[ _("Automatic bazooka") ] += BONUS_BAZ_TETE_C;
    break;

  default: std::cout << bonus << std::endl; assert (false);
  case bonusDYNAMITE:  
    txt << Format(ngettext(
                "%s team has won %u stick of dynamite!",
                "%s team has won %u sticks of dynamite!",
                BONUS_DYNAMITE),
            ActiveTeam().GetName().c_str(), BONUS_DYNAMITE);
    equipe.m_nb_ammos[ _("Dynamite") ] += BONUS_DYNAMITE; 
    break;
  }

  game_messages.Add (txt.str());
}

//-----------------------------------------------------------------------------

bool BonusBox::FaitApparaitre()
{
  if (!enable) return true;

  if (still_visible || (Wormux::global_time.Read() < temps_caisse)) return false;

  uint bcl=0;
  bool ok;
#ifdef DBG_PLACEMENT
  COUT_PLACEMENT << "Cherche une place ..." << std::endl;
#endif
  do
  {
    ok = true;
    Ready();
    if (bcl >= NB_MAX_TRY) 
    {
#ifdef DBG_PLACEMENT
      COUT_PLACEMENT << "Impossible de trouver une position initiale." << std::endl;
#endif
      return false;
    }

    // Placement au hasard en X
    int x = RandomLong(0, world.GetWidth() - GetWidth());
    int y = -GetHeight()+1;
    SetXY (x, y);
#ifdef DBG_PLACEMENT
    COUT_PLACEMENT << "Test en " << x <<"," << y << std::endl;
#endif

    // Vérifie que la caisse est dans le vide
    ok = !IsGhost() && IsInVacuum(0,0) && IsInVacuum(0,1);
    if (!ok) 
    {
#ifdef DBG_PLACEMENT
      COUT_PLACEMENT << "Placement dans un mur" << std::endl;
#endif
      continue;
    }

    // Vérifie que la caisse ne tombe pas dans le vide
    DirectFall();
    ok &= !IsGhost();
    if (!ok)
    {
      continue;
#ifdef DBG_PLACEMENT
      COUT_PLACEMENT << "Placement dans le vide" << std::endl;
#endif
    }

    // Vérifie que le caisse ne touche aucun ver au début
    FOR_ALL_LIVING_CHARACTERS(equipe, ver)
    {
      if (ObjTouche(*this, *ver)) 
      {
#ifdef DBG_PLACEMENT
	COUT_PLACEMENT << "La caisse touche le ver " << (*ver).m_name << std::endl;
#endif
	ok = false;
      }
    }
    if (ok) SetXY (x,y);
  } while (!ok);

#ifdef DBG_PLACEMENT
  COUT_PLACEMENT << "Placée après " << bcl << " essai(s)" << std::endl;
#endif

  still_visible = true;
  parachute = true;
  anim->SetCurrentFrame(0);
  anim->Start();

  temps_caisse = RandomLong(TEMPS_MIN_CREATION, 
			    TEMPS_MAX_CREATION-TEMPS_MIN_CREATION);
  temps_caisse *= 1000;
  temps_caisse += Wormux::global_time.Read();
  bonus = RandomLong (1, nbr_bonus_diff);

  SetMass (30);
  //SetSpeed (VITESSE_CAISSE, M_PI_2);
  SetSpeed(0,0);
  Ready();
  return true;
}

//-----------------------------------------------------------------------------

void BonusBox::Refresh()
{
  if (!still_visible) return;

  UpdatePosition();
#ifdef MSG_DBG_CAISSE
  COUT_DBG << "Refresh() : " << GetX() << "; " << GetY() << std::endl;
#endif

  // Si un ver touche la caisse, on la réinitialise
  FOR_ALL_LIVING_CHARACTERS(equipe, ver)
  {
    if (ObjTouche(*this, *ver))
    {
      // Offre des dynamites
      AppliqueBonus (**equipe, *ver);

      // Creation d'un nouvelle caisse
      still_visible = false;
      return;
    }
  }

  // Refresh de l'animation
  if (!parachute) anim->Update();
  
  m_ready = anim->IsFinished();
}

//-----------------------------------------------------------------------------

void BonusBox::SignalGhostState (bool)
{
  if (!still_visible) return;
#ifdef MSG_DBG_CAISSE
  COUT_DBG << "Une caisse sort de l'écran !" << std::endl;
#endif
  still_visible = false;
}

//-----------------------------------------------------------------------------

// Active les caisses ?
void BonusBox::Active (bool actif)
{
  enable = actif;
  
  if (!enable) still_visible = false;
}

//-----------------------------------------------------------------------------
