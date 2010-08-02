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
 * FootBomb Bomb : similar to cluster bomb, but much stronger
 *****************************************************************************/

#include "weapon/footbomb.h"
#include "weapon/weapon_cfg.h"
#include <sstream>
#include <math.h>
#include "weapon/explosion.h"
#include "graphic/sprite.h"
#include "interface/game_msg.h"
#include "map/camera.h"
#include "network/randomsync.h"
#include "object/objects_list.h"
#include "team/teams_list.h"
#include "tool/math_tools.h"
#include "tool/xml_document.h"
#include "game/time.h"

class FootBombConfig : public ExplosiveWeaponConfig
{
public:
  uint nb_fragments;
  uint nb_recursions;
  double nb_angle_dispersion;
  double nb_min_speed;
  double nb_max_speed;

  FootBombConfig();
  virtual void LoadXml(const xmlNode *elem);
};

class FootBomb : public WeaponProjectile
{
  uint m_recursions;

public:
  FootBomb(FootBombConfig& cfg,
              WeaponLauncher * p_launcher);
  void Refresh();
  virtual void SetEnergyDelta(int delta, bool do_report = true);
  void Shoot(const Point2i & pos, double strength, double angle, int recursions);

protected:
  void DoExplosion();
  void SignalOutOfMap();
};

//-----------------------------------------------------------------------------

FootBomb::FootBomb(FootBombConfig& cfg,
                         WeaponLauncher * p_launcher)
  : WeaponProjectile ("footbomb", cfg, p_launcher)
{
  m_rebound_sound = "weapon/footbomb_bounce";
  explode_with_collision = false;
  m_recursions = cfg.nb_recursions;

  // ensure that all football bomb have the same UniqueId
  // it prevents bomb to be blasted by explosion of its parent (fix network bug #11154)
  SetUniqueId("footbomb");
}

void FootBomb::Shoot(const Point2i & pos, double strength, double angle, int recursions)
{
  m_recursions = recursions;

  // a bit hackish...
  // we do need to collide with objects, but if we allow for this, the clusters
  // will explode on spawn (because of colliding with each other)
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_GROUND,true);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_CHARACTER,true);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_ITEM,false);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_PROJECTILE,false);

  begin_time = Time::GetInstance()->Read(); // this resets timeout
  Camera::GetInstance()->FollowObject(this);
  // TODO physic
  //ResetConstants();
  SetPosition( pos );
  GetPhysic()->SetSpeed(strength, angle);
}

void FootBomb::Refresh()
{
  WeaponProjectile::Refresh();
  image->SetRotation_rad(-GetPhysic()->GetAngle());
}

void FootBomb::SignalOutOfMap()
{
  GameMessages::GetInstance()->Add (_("The Football Bomb has left the battlefield before it could explode."));
  WeaponProjectile::SignalOutOfMap();
}

void FootBomb::DoExplosion()
{
  WeaponProjectile::DoExplosion();

  if ( 0 == m_recursions )
    return;

  const uint fragments = static_cast<FootBombConfig &>(cfg).nb_fragments;
  FootBomb * cluster;

  double half_angle_range = static_cast<FootBombConfig &>(cfg).nb_angle_dispersion * M_PI / 180;
  Point2i pos = GetPhysic()->GetPosition();
  for (uint i = 0; i < fragments; ++i )
  {
    double angle = -M_PI / 2; // this angle is "upwards" here
    double cluster_deviation = RandomSync().GetDouble( -half_angle_range, half_angle_range );
    double speed = RandomSync().GetDouble( static_cast<FootBombConfig &>(cfg).nb_min_speed,
        static_cast<FootBombConfig &>(cfg).nb_max_speed );

    cluster = new FootBomb(static_cast<FootBombConfig &>(cfg), launcher);
    cluster->Shoot( pos, speed, angle + cluster_deviation, m_recursions - 1 );
    cluster->SetTimeOut( cfg.timeout + m_timeout_modifier );

    ObjectsList::GetRef().AddObject(cluster);
  }
}

void FootBomb::SetEnergyDelta(int /* delta */, bool /* do_report */){};

//-----------------------------------------------------------------------------

FootBombLauncher::FootBombLauncher() :
  WeaponLauncher(WEAPON_FOOTBOMB, "footbomb", new FootBombConfig())
{
  UpdateTranslationStrings();

  m_category = THROW;
  ignore_collision_signal = true;
  ReloadLauncher();
}

void FootBombLauncher::UpdateTranslationStrings()
{
  m_name = _("Football Bomb");
  m_help = _("Timeout : Mouse wheel or Page Up/Down\nAngle : Up/Down\nFire : keep the space key pressed until the desired strength\nan ammo per turn");
}

WeaponProjectile * FootBombLauncher::GetProjectileInstance()
{
  return dynamic_cast<WeaponProjectile *>
      (new FootBomb(cfg(),dynamic_cast<WeaponLauncher *>(this)));
}

FootBombConfig& FootBombLauncher::cfg()
{
  return static_cast<FootBombConfig&>(*extra_params);
}

std::string FootBombLauncher::GetWeaponWinString(const char *TeamName, uint items_count ) const
{
  return Format(ngettext(
            "%s team has won %u football bomb!",
            "%s team has won %u football bombs!",
            items_count), TeamName, items_count);
}
//-----------------------------------------------------------------------------

FootBombConfig::FootBombConfig() :
  ExplosiveWeaponConfig()
{
  nb_fragments = 2;
  nb_recursions = 2;
}

void FootBombConfig::LoadXml(const xmlNode *elem)
{
  ExplosiveWeaponConfig::LoadXml(elem);
  XmlReader::ReadUint(elem, "nb_fragments", nb_fragments);
  XmlReader::ReadUint(elem, "nb_recursions", nb_recursions);
  XmlReader::ReadDouble(elem, "nb_angle_dispersion", nb_angle_dispersion);
  XmlReader::ReadDouble(elem, "nb_min_speed", nb_min_speed);
  XmlReader::ReadDouble(elem, "nb_max_speed", nb_max_speed);
}