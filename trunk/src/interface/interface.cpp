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
 * Interface affichant différentes informations sur la jeu.
 *****************************************************************************/

#include "interface.h"
//-----------------------------------------------------------------------------
#include <sstream>
#include "../map/camera.h"
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../team/teams_list.h"
#include "../tool/string_tools.h"
#include "../weapon/weapons_list.h"
#include "../graphic/graphism.h"
#include "../tool/i18n.h"
#include "../graphic/video.h"

//-----------------------------------------------------------------------------
WeaponStrengthBar weapon_strength_bar;
//-----------------------------------------------------------------------------

using namespace Wormux;
//-----------------------------------------------------------------------------

// Nom du ver
const uint NOM_VER_X = 32;
const uint NOM_VER_Y = 28;

// Energie du ver
const uint ENERGIE_VER_X = NOM_VER_X;
const uint ENERGIE_VER_Y = NOM_VER_Y+20;

// Nom de l'arme
const uint NOM_ARME_X = 508;
const uint NOM_ARME_Y = NOM_VER_Y;

// Munitions
const uint MUNITION_X = NOM_ARME_X;
const uint MUNITION_Y = ENERGIE_VER_Y;

// Ecusson de l'équipe
const uint ECUSSON_EQUIPE_X = 303;
const uint ECUSSON_EQUIPE_Y = 20;

// Icône de l'arme
const uint ICONE_ARME_X = 450;
const uint ICONE_ARME_Y = 20;

// Clipping du nom du ver + info
const uint INFO_VER_X1 = NOM_VER_X;
const uint INFO_VER_Y1 = NOM_VER_Y;
const uint INFO_VER_X2 = 296;
const uint INFO_VER_Y2 = 69;

// Barre d'énergie
const uint BARENERGIE_X = 170;
const uint BARENERGIE_Y = ENERGIE_VER_Y+2;
const uint BARENERGIE_LARG = INFO_VER_X2-BARENERGIE_X;
const uint BARENERGIE_HAUT = 15;

const uint CLIP_ARME_X1 = 447;
const uint CLIP_ARME_Y1 = 18;
const uint CLIP_ARME_X2 = 775;
const uint CLIP_ARME_Y2 = 67;

//-----------------------------------------------------------------------------
Interface interface;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Interface::Interface()
{
}

//-----------------------------------------------------------------------------

void Interface::Init()
{
  affiche = true;

  CL_ResourceManager* res=graphisme.LitRes();
  game_menu = CL_Surface("interface/menu_jeu", res);
  bg_time = CL_Surface("interface/fond_compteur", res);
  weapons_menu.Init();
  weapon_box_button = CL_Surface("interface/weapon_box_button", res);

  barre_energie.InitVal (0, 0, game_mode.character.init_energy);
  barre_energie.InitPos (BARENERGIE_X, BARENERGIE_Y, 
			 BARENERGIE_LARG, BARENERGIE_HAUT);
  barre_energie.border_color = CL_Color::white;
  barre_energie.value_color = CL_Color::darkgray;
  barre_energie.background_color = CL_Color::dimgray;  

  // strength bar initialisation  
  weapon_strength_bar.InitPos (0, 0, 400, 10);
  weapon_strength_bar.InitVal (0, 0, 100);
  weapon_strength_bar.value_color = CL_Color (255, 255, 255, 127);
  weapon_strength_bar.border_color = CL_Color(0, 0, 0, 127);
  weapon_strength_bar.background_color = CL_Color(255*6/10, 255*6/10, 255*6/10, 96);
}

//-----------------------------------------------------------------------------

void Interface::Reset()
{
  ver_pointe_souris = NULL;
  arme_pointe_souris = NULL;
  chrono = 0;
  barre_energie.InitVal (0, 0, game_mode.character.init_energy);
}

//-----------------------------------------------------------------------------

void Interface::AfficheInfoVer (Character &ver)
{

  int x = (video.GetWidth() - GetWidth())/2;
  int y = video.GetHeight() - GetHeight();

  std::ostringstream txt;

  CL_Display::push_cliprect(CL_Rect(x+INFO_VER_X1, 
					 y+INFO_VER_Y1,
					 x+INFO_VER_X2, 
					 y+INFO_VER_Y2));

  // Affiche le nom du ver
  txt.str ("");
  txt << _("Name") << " : " 
      << ver.m_name << " (" << ver.GetTeam().GetName() << ')';
  police_grand.WriteLeft (NOM_VER_X, NOM_VER_Y, txt.str());

  // Affiche l'énergie du ver
  txt.str ("");
  txt << _("Energy") << " : ";
  if (!ver.IsDead()) {
    txt << ver.GetEnergy() << '%';
    barre_energie.Actu (ver.GetEnergy());
  } else {
    txt << _("(dead)");
    barre_energie.Actu (0);
  }
  police_grand.WriteLeft (ENERGIE_VER_X, ENERGIE_VER_Y, txt.str());

  // Barre d'énergie
  barre_energie.Draw ();
  
  CL_Display::pop_cliprect();

  // Affiche l'écusson de l'équipe
  ver.TeamAccess().ecusson.draw (ECUSSON_EQUIPE_X, ECUSSON_EQUIPE_Y);
}

//-----------------------------------------------------------------------------

void Interface::AfficheInfoArme ()
{
  int x = (video.GetWidth() - GetWidth())/2;
  int y = video.GetHeight() - GetHeight();

  Weapon* arme_affiche;
  int nbr_munition;

  if(arme_pointe_souris==NULL)
  {
    arme_affiche = &ActiveTeam().AccessWeapon();
    nbr_munition = ActiveTeam().ReadNbAmmos();
  }
  else
  {
    arme_affiche = arme_pointe_souris;
    nbr_munition = ActiveTeam().ReadNbAmmos(arme_pointe_souris->GetName());
  }

  std::ostringstream txt;

  CL_Display::push_cliprect(CL_Rect(x+CLIP_ARME_X1, 
					 y+CLIP_ARME_Y1,
					 x+CLIP_ARME_X2, 
					 y+CLIP_ARME_Y2));

  // Nom de l'arme
  txt.str("");
  txt << _("Weapon") << " : ";
  txt << arme_affiche->GetName();
  police_grand.WriteLeft (NOM_ARME_X, NOM_ARME_Y, txt.str());

  // Icône de l'arme
  arme_affiche->icone.draw (ICONE_ARME_X, ICONE_ARME_Y);

  // Munitions
  txt.str ("");
  txt << _("Stock") << " : ";
  if (nbr_munition ==  INFINITE_AMMO)
    txt << _("(unlimited)");
  else
    txt << nbr_munition;
  police_grand.WriteLeft (MUNITION_X, MUNITION_Y, txt.str());

  CL_Display::pop_cliprect();

}

//-----------------------------------------------------------------------------

void Interface::Draw ()
{  
  bg_time.draw((video.GetWidth()/2)-40, 0, NULL);

  if ( game_loop.ReadState() == gamePLAYING && 
       weapon_strength_bar.visible) {
    // Position on the screen 
    uint barre_x = (video.GetWidth()-weapon_strength_bar.GetWidth())/2;
    uint barre_y = video.GetHeight()-weapon_strength_bar.GetHeight() 
      - interface.GetHeight()-10;
    
    // Drawing on the screen
    weapon_strength_bar.DrawXY (barre_x, barre_y);
  }
       
  weapons_menu.Draw();
  
  if (!affiche) return;

  int x = (video.GetWidth() - GetWidth())/2;
  int y = video.GetHeight() - GetHeight();

  // On a bien un ver et/ou une équipe pointée par la souris
  if (ver_pointe_souris == NULL) ver_pointe_souris = &ActiveCharacter();

  CL_Display::push_translate(x, y);
  
  // Redessine intégralement le fond ?
  game_menu.draw (0, 0, NULL);

  // Affiche le temps restant du tour ?
  if (0 <= chrono)
  {
    police_grand.WriteCenter (GetWidth()/2, GetHeight()/2, ulong2str(chrono));
  }

  // Affiche le nom du ver
  AfficheInfoVer (*ver_pointe_souris);

  // Affiche les informations sur l'arme
  AfficheInfoArme ();

  CL_Display::pop_modelview ();  
}

//-----------------------------------------------------------------------------

uint Interface::GetWidth() const { return 800; }
uint Interface::GetHeight() const { return 70; }

//-----------------------------------------------------------------------------

void Interface::ChangeAffiche (bool nv_affiche)
{
  affiche = nv_affiche;
  camera.CentreObjSuivi ();
}

//-----------------------------------------------------------------------------
