/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Grapple
 *****************************************************************************/

#include "weapon/grapple.h"
#include "weapon/weapon_cfg.h"
/*
#include <math.h>
#include "weapon/explosion.h"
#include "character/character.h"
#include "game/config.h"
#include "game/game.h"
#include "game/time.h"
#include "graphic/sprite.h"
#include "include/action_handler.h"
#include "map/camera.h"
#include "map/map.h"
#include "network/network.h"
#include "physic/physical_engine.h"
#include "sound/jukebox.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/math_tools.h"
*/
/*
#include "tool/resource_manager.h"
*/
#include "tool/xml_document.h"

const uint DT_MVT = 15 ; //delta_t between 2 up/down/left/right mvt
const uint DST_MIN = 80 ;  //dst_minimal between 2 nodes


class GrappleConfig : public EmptyWeaponConfig
{
 public:
  uint max_rope_length; // Max rope length in pixels
  uint automatic_growing_speed; // Pixel per 1/100 second.
  int push_force;

 public:
  GrappleConfig();
  void LoadXml(const xmlNode* elem);
};

//-----------------------------------------------------------------------------

Grapple::Grapple() : Weapon(WEAPON_GRAPPLE, "grapple", new GrappleConfig())
{
  UpdateTranslationStrings();

  m_category = MOVE;
  use_unit_on_first_shoot = false;
/*
  m_hook_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_hook");
  m_hook_sprite->EnableRotationCache(32);
  m_node_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_node");
*/
  m_is_active = false;
  //m_attaching = false;
  //go_left = false ;
  //go_right = false ;
  //delta_len = 0 ;
}

void Grapple::UpdateTranslationStrings()
{
  m_name = _("Grapple");
  /* TODO: FILL IT */
  /* m_help = _(""); */
}

Grapple::~Grapple()
{
  /*if (m_hook_sprite) delete m_hook_sprite;
  if (m_node_sprite) delete m_node_sprite;*/
}

bool Grapple::p_Shoot()
{
 /* last_broken_node_angle = 100;

  m_attaching = true;
  m_launch_time = Time::GetInstance()->Read() ;
  m_initial_angle = ActiveCharacter().GetFiringAngle();
  last_mvt=Time::GetInstance()->Read();

  if (TryAttachRope()) JukeBox::GetInstance()->Play("share", "weapon/grapple_attaching");*/
  return true;
}


void Grapple::Refresh()
{
  /*
  if (!IsInUse())
    return ;

  if (m_attaching)
    TryAttachRope();

  if (!ActiveTeam().IsLocal() && !ActiveTeam().IsLocalAI())
    return;

if (IsInUse() && !m_attaching)
  {
    ActiveCharacter().SetMovement("ninja-rope");
    ActiveCharacter().UpdatePosition();
    SendActiveCharacterInfo(true);
  }*/
}

void Grapple::Draw()
{
 /* int x, y;
  double angle, prev_angle;

  struct CL_Quad {Sint16 x1,x2,x3,x4,y1,y2,y3,y4;} quad;

  Weapon::Draw();

  if (!IsInUse())
  {
    return ;
  }

  if (m_attaching) {
    angle = m_initial_angle + M_PI/2;
  }
  else {
	angle = 0;
    //    angle = ActiveCharacter().GetRopeAngle();
  }

  prev_angle = angle;

  // Draw the rope.
  Point2i handPos = ActiveCharacter().GetHandPosition();
  x = handPos.x;
  y = handPos.y;

  quad.x1 = (int)round((double)x - 2 * cos(angle));
  quad.y1 = (int)round((double)y + 2 * sin(angle));
  quad.x2 = (int)round((double)x + 2 * cos(angle));
  quad.y2 = (int)round((double)y - 2 * sin(angle));

  for (std::list<rope_node_t>::reverse_iterator it = rope_nodes.rbegin();
       it != rope_nodes.rend(); it++)
    {
      quad.x3 = (int)round((double)it->pos.x + 2 * cos(angle));
      quad.y3 = (int)round((double)it->pos.y - 2 * sin(angle));
      quad.x4 = (int)round((double)it->pos.x - 2 * cos(angle));
      quad.y4 = (int)round((double)it->pos.y + 2 * sin(angle));

      float dx = sin(angle) * (float)m_node_sprite->GetHeight();
      float dy = cos(angle) * (float)m_node_sprite->GetHeight();
      int step = 0;
      int size = (quad.x1-quad.x4) * (quad.x1-quad.x4)
                +(quad.y1-quad.y4) * (quad.y1-quad.y4);
      size -= m_node_sprite->GetHeight();
      while( (step*dx*step*dx)+(step*dy*step*dy) < size )
      {
        if(m_attaching)
          m_node_sprite->Draw(Point2i(quad.x1 + (int)((float) step * dx),
                                      quad.y1 - (int)((float) step * dy)));
        else
          m_node_sprite->Draw(Point2i(quad.x4 + (int)((float) step * dx),
                                      quad.y4 + (int)((float) step * dy)));
        step++;
      }
      quad.x1 = quad.x4 ;
      quad.y1 = quad.y4 ;
      quad.x2 = quad.x3 ;
      quad.y2 = quad.y3 ;
      prev_angle = angle;
      angle = it->angle ;
    }

  m_hook_sprite->SetRotation_rad(-prev_angle);
  m_hook_sprite->Draw( rope_nodes.front().pos - m_hook_sprite->GetSize()/2);*/
}

// =========================== Moves management
/*
void Grapple::SetRopeSize(double length) const
{

}
*/

// =========================== Keys management

void Grapple::HandleKeyPressed_Up(bool /*shift*/)
{
  if (IsInUse())  {
    cable_sound.Play("share", "weapon/grapple_cable", -1);
  //  GoUp();
  }
 // else
   // ActiveCharacter().HandleKeyPressed_Up(shift);
}

void Grapple::HandleKeyRefreshed_Up(bool /*shift*/)
{
  //if (IsInUse())
   // GoUp();
 // else
  //  ActiveCharacter().HandleKeyRefreshed_Up(shift);
}

void Grapple::HandleKeyReleased_Up(bool /*shift*/)
{
  //if (IsInUse())
   // StopUp();
 // else
   // ActiveCharacter().HandleKeyReleased_Up(shift);
}

void Grapple::HandleKeyPressed_Down(bool /*shift*/)
{
  if (IsInUse()) {
    cable_sound.Play("share", "weapon/grapple_cable", -1);
   // GoDown();
  }// else
  //  ActiveCharacter().HandleKeyPressed_Down(shift);
}

void Grapple::HandleKeyRefreshed_Down(bool /*shift*/)
{
  //if (IsInUse())
    //GoDown();
  //else
   // ActiveCharacter().HandleKeyRefreshed_Down(shift);
}

void Grapple::HandleKeyReleased_Down(bool/* shift*/)
{
  //if (IsInUse())
    //StopDown();
  //else
  //  ActiveCharacter().HandleKeyReleased_Down(shift);
}

void Grapple::HandleKeyPressed_MoveLeft(bool/* shift*/)
{
 // if (IsInUse())
   // GoLeft();
 // else
  //  ActiveCharacter().HandleKeyPressed_MoveLeft(shift);
}

void Grapple::HandleKeyRefreshed_MoveLeft(bool /*shift*/)
{
 // if (!IsInUse())
   // ActiveCharacter().HandleKeyRefreshed_MoveLeft(shift);
}

void Grapple::HandleKeyReleased_MoveLeft(bool /*shift*/)
{
 // if (IsInUse())
   // StopLeft();
 // else
   // ActiveCharacter().HandleKeyReleased_MoveLeft(shift);
}

void Grapple::HandleKeyPressed_MoveRight(bool /*shift*/)
{
 // if (IsInUse())
   // GoRight();
 // else
   // ActiveCharacter().HandleKeyPressed_MoveRight(shift);
}

void Grapple::HandleKeyRefreshed_MoveRight(bool /*shift*/)
{
 //
}

void Grapple::HandleKeyReleased_MoveRight(bool /*shift*/)
{
 // if (IsInUse())
   // StopRight();
  //else
   // ActiveCharacter().HandleKeyReleased_MoveRight(shift);
}

void Grapple::HandleKeyPressed_Shoot(bool)
{
  if (IsInUse()) {
    NewActionWeaponStopUse();
  } else
    NewActionWeaponShoot();
}

void Grapple::PrintDebugRope()
{
//   printf("%05d %05d %03.3f\n",
//          ActiveCharacter().GetX(),
//          ActiveCharacter().GetY(),
// 	          ActiveCharacter().GetRopeAngle());

/*
  for (std::list<rope_node_t>::iterator it = rope_nodes.begin();
       it != rope_nodes.end();
       it++) {

    printf("%05d %05d %03.3f %d\n", it->pos.x, it->pos.y,
           it->angle, it->sense);
  }*/
}

std::string Grapple::GetWeaponWinString(const char *TeamName, uint items_count ) const
{
  return Format(ngettext(
            "%s team has won %u grapple!",
            "%s team has won %u grapples!",
            items_count), TeamName, items_count);
}

//-----------------------------------------------------------------------------

GrappleConfig& Grapple::cfg()
{
  return static_cast<GrappleConfig&>(*extra_params);
}
//-----------------------------------------------------------------------------

GrappleConfig::GrappleConfig()
{
  max_rope_length = 450;
  automatic_growing_speed = 12;
  push_force = 10;
}

void GrappleConfig::LoadXml(const xmlNode* elem)
{
  EmptyWeaponConfig::LoadXml(elem);
  XmlReader::ReadUint(elem, "max_rope_length", max_rope_length);
  XmlReader::ReadUint(elem, "automatic_growing_speed", automatic_growing_speed);
  XmlReader::ReadInt(elem, "push_force", push_force);
}
