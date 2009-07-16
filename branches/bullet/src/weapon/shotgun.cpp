/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * Shotgun. Shoot a bunch of buckshot at each fire
 *****************************************************************************/

#include "weapon/explosion.h"
#include "weapon/shotgun.h"
#include "weapon/weapon_cfg.h"

#include <sstream>
#include "map/map.h"
#include "game/time.h"
#include "graphic/sprite.h"
#include "object/objects_list.h"
#include "interface/game_msg.h"
#include "network/randomsync.h"
#include "sound/jukebox.h"
#include "team/teams_list.h"

#include "tool/resource_manager.h"
#include "tool/xml_document.h"

//////////////////
// ShotgunConfig

class ShotgunConfig : public ExplosiveWeaponConfig
{
  public:
     double m_random_angle;
     double m_random_strength;
     int m_nbr_bullets;
     uint m_buckshot_speed;
     uint m_explosion_range;
     ShotgunConfig();
     void LoadXml(const xmlNode* elem);
};


ShotgunConfig::ShotgunConfig():
m_random_angle(0.04),
m_random_strength(2.0),
m_nbr_bullets(5),
m_buckshot_speed(60)
{
  explosion_range = 10;
}

void ShotgunConfig::LoadXml(const xmlNode* elem){
  WeaponConfig::LoadXml(elem);
  XmlReader::ReadUint(elem, "explosion_range", explosion_range);
  XmlReader::ReadInt(elem, "nbr_bullets", m_nbr_bullets);
  XmlReader::ReadDouble(elem, "random_angle", m_random_angle);
  XmlReader::ReadDouble(elem, "random_strength", m_random_strength);
  XmlReader::ReadUint(elem, "buckshot_speed", m_buckshot_speed);
}

class ShotgunBuckshot : public WeaponBullet
{
  public:
    ShotgunBuckshot(ShotgunConfig& cfg,
                    WeaponLauncher * p_launcher);
    bool IsOverlapping(const PhysicalObj* obj) const;
    ShotgunConfig& cfg();
  protected:
    void RandomizeShoot(double &angle,double &strength);
};

ShotgunBuckshot::ShotgunBuckshot(ShotgunConfig& cfg,
                                 WeaponLauncher * p_launcher) :
  WeaponBullet("buckshot", cfg, p_launcher)
{
}

ShotgunConfig& ShotgunBuckshot::cfg() {
  return static_cast<ShotgunConfig&>(WeaponBullet::cfg);
}

void ShotgunBuckshot::RandomizeShoot(double &angle,double &strength)
{
  angle += M_PI * RandomSync().GetDouble(-cfg().m_random_angle,cfg().m_random_angle);
  strength += RandomSync().GetDouble(-cfg().m_random_strength,cfg().m_random_strength);
}

bool ShotgunBuckshot::IsOverlapping(const PhysicalObj* obj) const
{
  if (GetName() == obj->GetName()) return true;

  return (GetOverlappingObject() == obj);
}

//-----------------------------------------------------------------------------

Shotgun::Shotgun() : WeaponLauncher(WEAPON_SHOTGUN, "shotgun", new ShotgunConfig())
{
  UpdateTranslationStrings();

  m_category = RIFLE;

  announce_missed_shots = false;
  m_weapon_fire = new Sprite(GetResourceManager().LoadImage(weapons_res_profile,m_id+"_fire"));
  m_weapon_fire->EnableRotationCache(32);

  ReloadLauncher();
}

ShotgunConfig& Shotgun::cfg() {
  return static_cast<ShotgunConfig&>(*extra_params);
}

void Shotgun::UpdateTranslationStrings()
{
  m_name = _("Shotgun");
  /* TODO: FILL IT */
  /* m_help = _(""); */
}

// Return a buckshot instance for the shotgun
WeaponProjectile * Shotgun::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new ShotgunBuckshot(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

void Shotgun::ShootSound() const
{
  JukeBox::GetInstance()->Play("default", "weapon/shotgun");
}

void Shotgun::IncMissedShots()
{
  if(missed_shots + 1 == cfg().m_nbr_bullets)
    announce_missed_shots = true;
  WeaponLauncher::IncMissedShots();
}

bool Shotgun::p_Shoot ()
{
  missed_shots = 0;
  announce_missed_shots = false;
  if (IsInUse())
    return false;

  for(int i = 0; i < cfg().m_nbr_bullets; i++) {
    projectile->Shoot(cfg().m_buckshot_speed);
    projectile = NULL;
    ReloadLauncher();
  }
  ShootSound();
  return true;
}

std::string Shotgun::GetWeaponWinString(const char *TeamName, uint items_count ) const
{
  return Format(ngettext(
            "%s team has won %u shotgun!",
            "%s team has won %u shotguns!",
            items_count), TeamName, items_count);
}

