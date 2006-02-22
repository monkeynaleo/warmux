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
 * Arme dynamite : lorqu'on "tire", un baton de dynamite est l�ch�. Celui
 * explos apr�s un laps de temps. La baton fait alors un gros trou dans la
 * carte, souffle les vers qui sont autour en leur faisant perdre de l'�nergie.
 *****************************************************************************/

#include "dynamite.h"
#include "weapon_tools.h"
#include "../game/config.h"
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../include/app.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../tool/resource_manager.h"

#ifdef __MINGW32__
#undef LoadImage
#endif

BatonDynamite::BatonDynamite(GameLoop &p_game_loop, ExplosiveWeaponConfig& cfg) :
  WeaponProjectile(p_game_loop, "dynamite_bullet", cfg)
{
  channel = -1;

  image->animation.SetLoopMode(false);
  SetSize(image->GetSize());

  SetTestRect (0, 0, 2, 3);

  explosion = resource_manager.LoadSprite(weapons_res_profile, "explosion");
  explosion->animation.SetLoopMode(false);
}

void BatonDynamite::Reset()
{
  Ready();
  is_active = false;
  explosion_active = false;

  unsigned int delay = (1000 * cfg.timeout)/image->GetFrameCount();
  image->SetFrameSpeed(delay);

  image->Scale(ActiveCharacter().GetDirection(), 1);
  image->SetCurrentFrame(0);
  image->Start();
}

void BatonDynamite::Refresh()
{
  if (!is_active) return;
  bool fin;
  assert (!IsGhost());
  if (!explosion_active) {
    image->Update(); 
    fin = image->IsFinished();
    if (fin) explosion_active = true;
  } else {
    explosion->Update();
    fin = explosion->IsFinished();
    if (fin) is_active = false;
  }
}

void BatonDynamite::Draw()
{
  if (!is_active) return;
  assert (!IsGhost());

  if (!explosion_active)
    image->Draw(GetPosition());
  else
    explosion->Draw(GetPosition() - explosion->GetSize()/2);
}

void BatonDynamite::ShootSound()
{
  channel = jukebox.Play("share","weapon/dynamite_fuze", -1);
}

void BatonDynamite::Explosion()
{
  jukebox.Stop(channel);
  channel = -1;
  explosion->SetCurrentFrame(0);
  explosion->Start();
  explosion_active = true;
  WeaponProjectile::Explosion();
}

void BatonDynamite::SignalCollision() 
{
  if (IsGhost()) is_active = false;
}

//-----------------------------------------------------------------------------

Dynamite::Dynamite() :
  WeaponLauncher(WEAPON_DYNAMITE, "dynamite", new ExplosiveWeaponConfig(), VISIBLE_ONLY_WHEN_INACTIVE)
{
  m_name = _("Dynamite");
  
  projectile = new BatonDynamite(game_loop, cfg());
}

void Dynamite::p_Select()
{
  projectile->Reset();
}

// Pose une dynamite
bool Dynamite::p_Shoot ()
{
  Point2d speed_vector;

  projectile->Reset();
  projectile->Shoot(0);

  // Ajoute la vitesse actuelle du ver
  ActiveCharacter().GetSpeedXY (speed_vector);
  projectile->SetSpeedXY (speed_vector);

  return true;
}
