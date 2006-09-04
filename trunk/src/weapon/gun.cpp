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
 * Arme gun : la balle part tout droit dans la direction donn� par
 * le viseur. Si la balle ne touche pas un ver, elle va faire un trou dans
 * le terrain. La balle peut �alement toucher les objets du plateau du jeu.
 *****************************************************************************/

#include "../weapon/gun.h"
#include <sstream>
#include "../map/map.h"
#include "../game/time.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../interface/game_msg.h"
#include "../interface/game_msg.h"
#include "../weapon/gun.h"
#include "../tool/math_tools.h"
#include "../weapon/explosion.h"

const uint GUN_BULLET_SPEED = 20;

GunBullet::GunBullet(ExplosiveWeaponConfig& cfg,
                     WeaponLauncher * p_launcher) :
  WeaponBullet("gun_bullet", cfg, p_launcher)
{
}

void GunBullet::ShootSound()
{
  jukebox.Play("share","weapon/gun");
}

//-----------------------------------------------------------------------------

Gun::Gun() : WeaponLauncher(WEAPON_GUN, "gun", new ExplosiveWeaponConfig())
{
  m_name = _("Gun");
  gun_fire = new Sprite(resource_manager.LoadImage(weapons_res_profile,m_id+"_fire"));
  gun_fire->EnableRotationCache(32);
  last_fire = 0;
  ReloadLauncher();
}

WeaponProjectile * Gun::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new GunBullet(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

Point2i Gun::GetGunHolePosition()
{
  int rayon = m_image->GetWidth();
  double angleRAD = Deg2Rad(ActiveTeam().crosshair.GetAngleVal());
  Point2i hole_position = Point2i(rayon, rayon) * Point2d(cos(angleRAD), sin(angleRAD));
  hole_position.x += position.dx;
  hole_position.y += position.dy;
  Point2i tmp = ActiveCharacter().GetHandPosition() + (hole_position * Point2i(ActiveCharacter().GetDirection(),1));
  return tmp;
}

void Gun::Draw()
{
  WeaponLauncher::Draw();
  if (last_fire + 100 < Time::GetInstance()->Read()) return;
  Point2i size = gun_fire->GetSize();
  size.x = (ActiveCharacter().GetDirection() == 1 ? 0 : size.x);
  size.y /= 2;
  gun_fire->SetRotation_deg (ActiveTeam().crosshair.GetAngle());
  gun_fire->Draw( GetGunHolePosition() - size );
}

bool Gun::p_Shoot ()
{
  if (m_is_active)
    return false;  

  m_is_active = true;
  last_fire = Time::GetInstance()->Read();
  ReloadLauncher();
  projectile->Shoot (GUN_BULLET_SPEED);
  launcher_is_loaded = false;

  return true;
}

