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
 * Arme dynamite : lorqu'on "tire", un baton de dynamite est lâché. Celui
 * explos après un laps de temps. La baton fait alors un gros trou dans la
 * carte, souffle les vers qui sont autour en leur faisant perdre de l'énergie.
 *****************************************************************************/

#include "dynamite.h"
//-----------------------------------------------------------------------------
#include "../game/game_mode.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../game/config.h"
#include "../game/game_loop.h"
#include "../weapon/weapon_tools.h"
#include "../object/objects_list.h"
#include "../weapon/weapons_list.h"
#ifdef CL
#else
#include "../tool/resource_manager.h"
#include "../include/app.h"
#endif

#ifdef __MINGW32__
#undef LoadImage
#endif
//-----------------------------------------------------------------------------
namespace Wormux {

Dynamite dynamite;

#ifdef DEBUG
  //  #define DEBUG_CADRE_TEST
#endif

//-----------------------------------------------------------------------------

BatonDynamite::BatonDynamite() : WeaponProjectile("baton de dynamite")
{}

//-----------------------------------------------------------------------------

void BatonDynamite::Init()
{
  SetMass (dynamite.cfg().mass);

#ifdef CL
  image = CL_Sprite("dynamite_anim", &graphisme.weapons);

  double delay = dynamite.cfg().duree/image.get_frame_count()/1000.0 ;
  for (int i=0 ; i < image.get_frame_count(); i++)
    image.set_frame_delay(i, delay) ;

  image.set_play_loop(false);

  SetSize (image.get_width(), image.get_height());

  SetTestRect (0, 0, 2, 3);
   
  explosion = CL_Sprite("explosion", &graphisme.weapons);
  delay = 60/explosion.get_frame_count()/1000.0 ;
  for (int i=0 ; i < explosion.get_frame_count(); i++)
    explosion.set_frame_delay(i, delay) ;

  explosion.set_play_loop(false);

#else

   image = resource_manager.LoadSprite(weapons_res_profile,"dynamite_anim");

   double delay = dynamite.cfg().duree/image->GetFrameCount()/1000.0 ;
   for (uint i=0 ; i < image->GetFrameCount(); i++)
     ; // TODO // image.set_frame_delay(i, delay) ;

   image->Start();

   SetSize (image->GetWidth(), image->GetHeight());

   SetTestRect (0, 0, 2, 3);

   explosion = resource_manager.LoadSprite(weapons_res_profile, "explosion");
   delay = 60/explosion->GetFrameCount()/1000.0 ;
   for (uint i=0 ; i < explosion->GetFrameCount(); i++)
    ; // TODO explosion.set_frame_delay(i, delay) ;

  explosion->Start();
   
   
#endif

}

//-----------------------------------------------------------------------------

void BatonDynamite::Reset()
{
  Ready();
  is_active = false;
  explosion_active = false;

#ifdef CL
  image.restart();
  image.set_frame (0);

  explosion.restart();
  explosion.set_frame(0);
#else
  image->SetCurrentFrame(0);

  explosion->SetCurrentFrame(0);
#endif
}

//-----------------------------------------------------------------------------

void BatonDynamite::Refresh()
{
#ifdef CL
  if (!is_active) return;
  bool fin;
  assert (!IsGhost());
  if (!explosion_active) {
    image.update(); 
    fin = image.is_finished();
    if (fin) explosion_active = true;
  } else {
    explosion.update();
    fin = explosion.is_finished();
    if (fin) is_active = false;
  }
#else
  if (!is_active) return;
  bool fin;
  assert (!IsGhost());
  if (!explosion_active) {
    image->Update(); 
    fin = image->GetCurrentFrame() == image->GetFrameCount()-1;
    if (fin) explosion_active = true;
  } else {
    explosion->Update();
    fin = explosion->GetCurrentFrame() == explosion->GetFrameCount()-1;
    if (fin) is_active = false;
  }
#endif
}

//-----------------------------------------------------------------------------

#ifdef CL
void BatonDynamite::Draw()
{
  if (!is_active) return;
  assert (!IsGhost());

  int x = GetX();
  int y = GetY();
  if (!explosion_active)
    image.draw (x,y); 
  else {
    x -= explosion.get_width()/2;
    y -= explosion.get_height()/2;
    explosion.draw(x,y);
  }

#if defined(DEBUG_CADRE_TEST)
  CL_Display::draw_rect (LitRectTest(), CL_Color::red);
#endif
}
#else
void BatonDynamite::Draw()
{
  if (!is_active) return;
  assert (!IsGhost());

  int x = GetX();
  int y = GetY();
  if (!explosion_active)
    image->Draw(x,y);
  else {
    x -= explosion->GetWidth()/2;
    y -= explosion->GetHeight()/2;
    explosion->Draw(x,y);
  }
}
   
#endif
//-----------------------------------------------------------------------------

void BatonDynamite::SignalCollision() {}

void BatonDynamite::SignalGhostState (bool) { is_active = false; }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Dynamite::Dynamite() : Weapon(WEAPON_DYNAMITE, "dynamite")
{
  m_name = _("Dynamite");
  extra_params = new DynamiteConfig();
  channel = -1;

   m_visibility = VISIBLE_ONLY_WHEN_INACTIVE;
}

//-----------------------------------------------------------------------------

void Dynamite::p_Init()
{
  baton.Init();
#ifdef CL
   impact = CL_Surface("dynamite_impact", &graphisme.weapons);
#else
   impact = resource_manager.LoadImage(weapons_res_profile,"dynamite_impact");
#endif
}

//-----------------------------------------------------------------------------

void Dynamite::p_Select()
{
  baton.Reset();
}

//-----------------------------------------------------------------------------

// Pose une dynamite
bool Dynamite::p_Shoot ()
{
  DoubleVector speed_vector ;

  // Ajoute la représentation
  int x,y;
  PosXY (x,y);
  baton.Reset ();
  baton.PrepareTir();
  baton.SetXY (x, y);
  lst_objets.AjouteObjet (&baton, true);

  // Ajoute la vitesse actuelle du ver
  ActiveCharacter().GetSpeedXY (speed_vector);
  baton.SetSpeedXY (speed_vector);

  // Active l'animation
#ifdef CL
   jukebox.Play("weapon/dynamite_fuze");
#else
   jukebox.Play("share","weapon/dynamite_fuze");
   // TODO
#endif
   return true;
}

//-----------------------------------------------------------------------------

void Dynamite::Refresh()
{
  if (m_is_active) {
    // Fin de l'explwosion ?
    if (!baton.is_active) FinExplosion ();
  } else {
    // Change le sens de l'image si nécessaire
#ifdef CL
     m_image.set_scale(ActiveCharacter().GetDirection(), 1);
#else
     //TODO
     //m_image->Scale(ActiveCharacter().GetDirection(), 1);
#endif
  }
}

//-----------------------------------------------------------------------------

void Dynamite::FinExplosion ()
{
  m_is_active = false;

  lst_objets.RetireObjet (&baton);

#ifdef CL
   jukebox.Stop("weapon/dynamite_fuze");
#else
   jukebox.Stop(channel);
   channel = -1;
#endif
   
  // Si la dynamite est sortie de l'écran, on ne fait rien
  if (baton.IsGhost()) return;

  // Applique les degats aux vers
#ifdef CL
  CL_Point centre = baton.GetCenter();
  centre.y = baton.GetY()+baton.GetHeight();
  AppliqueExplosion (centre, centre, impact, cfg(), NULL, "weapon/dynamite_exp");
#else
  Point2i centre = baton.GetCenter();
  centre.y = baton.GetY()+baton.GetHeight();
  AppliqueExplosion (centre, centre, impact, cfg(), NULL, "weapon/dynamite_exp");
#endif
}

//-----------------------------------------------------------------------------

DynamiteConfig& Dynamite::cfg() 
{ return static_cast<DynamiteConfig&>(*extra_params); }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

DynamiteConfig::DynamiteConfig()
{
  duree = 2000;
}

void DynamiteConfig::LoadXml(xmlpp::Element *elem)
{
  ExplosiveWeaponConfig::LoadXml(elem);
  LitDocXml::LitUint (elem, "duree", duree);
}

//-----------------------------------------------------------------------------
} // namespace Wormux
