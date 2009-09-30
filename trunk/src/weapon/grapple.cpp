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
 * Grapple
 *****************************************************************************/

#include "weapon/grapple.h"
#include "weapon/weapon_cfg.h"

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
#include "sound/jukebox.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/math_tools.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

const uint DT_MVT = 15 ; //delta_t between 2 up/down/left/right mvt
const uint DST_MIN = 80 ;  //dst_minimal between 2 nodes

bool find_first_contact_point (Point2i from, double angle, uint length,
                               int skip, Point2i &contact_point)
{
  Point2d posd;
  double x_step, y_step ;

  x_step = cos(angle) ;
  y_step = sin(angle) ;

  posd.x = (double)from.x ;
  posd.y = (double)from.y ;

  posd.x += ((double)skip) * x_step;
  posd.y += ((double)skip) * y_step;

  from.x = (int)round(posd.x) ;
  from.y = (int)round(posd.y) ;

  contact_point.x = from.x ;
  contact_point.y = from.y ;

  length -= skip;

  // make it return the last point still in vacuum
  Point2i new_contact_point = contact_point;
  bool contact_point_uncertain = true;
  while(!GetWorld().IsOutsideWorld(new_contact_point) &&
        (length > 0))
    {
      if ( !GetWorld().IsInVacuum( new_contact_point ) )
      {
        ASSERT( contact_point_uncertain || GetWorld().IsInVacuum( contact_point ) );

        // for uncertain contact points, see if it's in vacuum
        if ( contact_point_uncertain && !GetWorld().IsInVacuum( contact_point ) )
        {
           // it's not, so try our best to return a contact point in vacuum
           // try searching in area NxN around our original point and return
           // the closest pixel in vacuum

           // FIXME: can be optimized!
           const int search_radius = 5; // 121 pixels to search

           Point2i closest_point;
           Point2i cur;
           int closest_point_distance = 2 * search_radius * search_radius + 1;// max
           bool found = false;

           for ( int i = -search_radius; i <= search_radius; i ++ )
           {
             for ( int j = -search_radius; j <= search_radius; j ++ )
             {
               cur = contact_point + Point2i( i, j );
               if ( GetWorld().IsInVacuum( cur ) )
               {
                 // check for new closest
                 int distance = i * i + j * j;
                 if ( distance < closest_point_distance )
                 {
                   closest_point_distance = distance;
                   closest_point = cur;
                   found = true;
                 }
               }
             }
           }

           if ( found )
             contact_point = closest_point;
        }
        return true ;
      }

      contact_point = new_contact_point;
      contact_point_uncertain = false; //now we know that it's in vacuum
      posd.x += x_step ;
      posd.y += y_step ;
      new_contact_point.x = (int)round(posd.x) ;
      new_contact_point.y = (int)round(posd.y) ;
      length--;
    }

  return false ;
}

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

  m_hook_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_hook");
  m_hook_sprite->EnableRotationCache(32);
  m_node_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_node");

  m_is_active = false;
  m_attaching = false;
  go_left = false ;
  go_right = false ;
  delta_len = 0 ;
  move_left_pressed = false;
  move_right_pressed = false;
  move_up_pressed = false;
  move_down_pressed = false;
}

void Grapple::UpdateTranslationStrings()
{
  m_name = _("Grapple");
  /* TODO: FILL IT */
  /* m_help = _(""); */
}

Grapple::~Grapple()
{
  if (m_hook_sprite) delete m_hook_sprite;
  if (m_node_sprite) delete m_node_sprite;
}

bool Grapple::p_Shoot()
{
  last_broken_node_angle = 100;

  m_attaching = true;
  m_launch_time = Time::GetInstance()->Read() ;
  m_initial_angle = ActiveCharacter().GetFiringAngle();
  last_mvt=Time::GetInstance()->Read();

  if (TryAttachRope()) JukeBox::GetInstance()->Play("default", "weapon/grapple_attaching");
  return true;
}

bool Grapple::TryAttachRope()
{
  Point2i pos;
  uint length;
  uint delta_time = Time::GetInstance()->Read() - m_launch_time;
  double angle ;

  // Remove the root node
  rope_nodes.clear();

  // The rope is being launching. Increase the rope length and check
  // collisions.

  ActiveCharacter().GetHandPosition(pos);

  length = cfg().automatic_growing_speed * delta_time / 10;
  if (length > cfg().max_rope_length)
    {
      // Hum the rope is too short !
      m_attaching = false;
      m_is_active = false;

      // Give back one ammo...
      int *ammo = &ActiveTeam().AccessNbAmmos();
      if (*ammo != INFINITE_AMMO) (*ammo)++;
      ASSERT (*ammo > 0 || *ammo == INFINITE_AMMO);

      return false;
    }

  angle = m_initial_angle;

  Point2i contact_point;
  if (find_first_contact_point(pos, angle, length, 4, contact_point))
    {
      AttachRope(contact_point);
      return true;
    }

  rope_node_t root_node;
  root_node.pos.x = pos.x + int(length * cos(angle));
  root_node.pos.y = pos.y + int(length * sin(angle));
  root_node.angle = 0;
  root_node.sense = 0;
  rope_nodes.push_back(root_node);

  return false;
}

bool Grapple::TryAddNode(int CurrentSense)
{
  uint lg;
  Point2d V;
  Point2i contact_point;
  double angle, rope_angle;
  Point2i handPos;

  ActiveCharacter().GetHandPosition(handPos);

  // Compute distance between hands and rope fixation point.

  V.x = handPos.x - m_fixation_point.x;
  V.y = handPos.y - m_fixation_point.y;
  angle = V.ComputeAngle();
  lg = static_cast<uint>(V.Norm());

  if (lg < DST_MIN)
    return false;

  // Check if the rope collide something

  if (find_first_contact_point(m_fixation_point, angle, lg, 2, contact_point))
    {
      rope_angle = ActiveCharacter().GetRopeAngle() ;

      if ( (last_broken_node_sense * CurrentSense > 0) &&
           (fabs(last_broken_node_angle - rope_angle) < 0.1))
        return false ;

      // if contact point is the same as position of the last node
      // (can happen because of jitter applied in find_first_contact_point),
      // give up adding such node
      if ( rope_nodes.size() > 0 && rope_nodes.back().pos == contact_point )
        return false;

      // The rope has collided something...
      // Add a node on the rope and change the fixation point
      AttachNode(contact_point, rope_angle, CurrentSense);

      return true;
    }

  return false;
}

bool Grapple::TryRemoveNodes(int currentSense)
{
  if ( rope_nodes.size() < 2 )
    return false;

  // [RCL]: nodeSense check seems to be useless... either remove node senses at all or
  // find an example where it is required
  double currentAngle = ActiveCharacter().GetRopeAngle();
  Point2i mapRopeStart;
  ActiveCharacter().GetHandPosition(mapRopeStart);

  const int max_nodes_per_turn = 100; // safe value, was used to avoid network congestion
  int nodes_to_remove = 0;

  TraceResult tr;

  for ( std::list<rope_node_t>::reverse_iterator it = rope_nodes.rbegin();
       it != rope_nodes.rend(); it++ )
  {
    if ( nodes_to_remove >= max_nodes_per_turn )
        break;

    // try tracing to current node:
    // if we cannot trace, this means that previous node shouldn't have been removed
    // (NOTE: since nodes are often in ground, we're ignoring traces hitting ground
    // right at the end)
    const float end_proximity_threshold = 0.95f;
    if ( GetWorld().TraceRay( mapRopeStart, it->pos, tr ) && tr.m_fraction < end_proximity_threshold )
    {
        // collision detected!
        if ( nodes_to_remove > 0 )
            nodes_to_remove--; // undo the node remove

        // now we can stop removing the nodes as we don't have the clear "sight"
        // to current node
        break;
    };

    double nodeAngle = it->angle;

    int currentAngleSign = ( currentAngle < 0 ) ? -1 : 1;
    int nodeAngleSign = ( nodeAngle < 0 ) ? -1 : 1;

    if ( currentAngleSign != nodeAngleSign && rope_nodes.size() > 2 )
        nodes_to_remove++;
    else
        break;

  };

  if ( nodes_to_remove > 0 )
    MSG_DEBUG( "grapple.break", "nodes to remove %d", nodes_to_remove );

  for ( int i = 0; i < nodes_to_remove; i ++ )
  {
     last_broken_node_angle = currentAngle;
     last_broken_node_sense = currentSense;

     // remove last node
     DetachNode();
  }

  return nodes_to_remove > 0;
}

void Grapple::NotifyMove(bool collision)
{
  bool addNode = false;
  int currentSense;

  if (!IsInUse() || m_attaching)
    return;

  // Check if the character collide something.
  if (collision)
    {
      // Yes there has been a collision.
      if (delta_len != 0)
        {
          // The character tryed to change the rope size.
          // There has been a collision, so we cancel the rope length change.
          ActiveCharacter().ChangePhysRopeSize (-delta_len);
          delta_len = 0;
        }
      return;
    }

  currentSense = ActiveCharacter().GetAngularSpeed() >= 0 ? 1: -1;

  // While there is nodes to add, we add !
  while (TryAddNode(currentSense))
    addNode = true;

  // If we have created nodes, we exit to avoid breaking what we
  // have just done !
  if (addNode)
    return;

  TryRemoveNodes( currentSense );
}

void Grapple::Refresh()
{
  if (!IsInUse())
    return ;

  if (move_left_pressed && !move_right_pressed) {
    GoLeft();
  } else if (move_right_pressed && !move_left_pressed) {
    GoRight();
  }

  if (move_up_pressed && !move_down_pressed) {
    GoUp();
  } else if (move_down_pressed && !move_up_pressed) {
    GoDown();
  }


  if (m_attaching)
    TryAttachRope();

  if (IsInUse() && !m_attaching)
  {
    ActiveCharacter().SetMovement("ninja-rope");
    ActiveCharacter().UpdatePosition();
  }
}

void Grapple::Draw()
{
  int x, y;
  double angle, prev_angle;
  Point2i handPos;

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
    angle = ActiveCharacter().GetRopeAngle();
  }

  prev_angle = angle;

  // Draw the rope.
  ActiveCharacter().GetHandPosition(handPos);
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
  m_hook_sprite->Draw( rope_nodes.front().pos - m_hook_sprite->GetSize()/2);
}

void Grapple::AttachRope(const Point2i& contact_point)
{
  MSG_DEBUG("grapple.hook", "** AttachRope %d,%d", contact_point.x, contact_point.y);

  m_attaching = false;
  m_is_active = true;
  move_left_pressed = false;
  move_right_pressed = false;
  move_up_pressed = false;
  move_down_pressed = false;

  rope_nodes.clear();

  // The rope reaches the fixation point. Let's fix it !
  Point2i pos;
  ActiveCharacter().GetRelativeHandPosition(pos);

  ActiveCharacter().SetPhysFixationPointXY(
                                           contact_point.x / PIXEL_PER_METER,
                                           contact_point.y / PIXEL_PER_METER,
                                           (double)pos.x / PIXEL_PER_METER,
                                           (double)pos.y / PIXEL_PER_METER);

  m_fixation_point = contact_point;

  rope_node_t root_node;
  root_node.pos = m_fixation_point;
  root_node.angle = 0;
  root_node.sense = 0;
  rope_nodes.push_back(root_node);

  ActiveCharacter().ChangePhysRopeSize (-10.0 / PIXEL_PER_METER);
  m_hooked_time = Time::GetInstance()->Read();
  ActiveCharacter().SetMovement("ninja-rope");

  ActiveCharacter().SetFiringAngle(-M_PI / 3);

  // Camera should focus on it!
  Camera::GetInstance()->FollowObject(&ActiveCharacter());
}

void Grapple::DetachRope()
{
  ActiveCharacter().UnsetPhysFixationPoint();
  rope_nodes.clear();
  m_is_active = false;

  cable_sound.Stop();
}

void Grapple::AttachNode(const Point2i& contact_point,
                         double angle,
                         int sense)
{
  // The rope has collided something...
  // Add a node on the rope and change the fixation point.
  Point2i pos;
  ActiveCharacter().GetRelativeHandPosition(pos);

  ActiveCharacter().SetPhysFixationPointXY(contact_point.x / PIXEL_PER_METER,
                                           contact_point.y / PIXEL_PER_METER,
                                           (double)pos.x / PIXEL_PER_METER,
                                           (double)pos.y / PIXEL_PER_METER);

  m_fixation_point = contact_point;
  rope_node_t node;
  node.pos = m_fixation_point;
  node.angle = angle;
  node.sense = sense;
  rope_nodes.push_back(node);

  MSG_DEBUG("grapple.node", "+ %d,%d %f %d", node.pos.x, node.pos.y, node.angle, node.sense);
}

void Grapple::DetachNode()
{
  ASSERT(rope_nodes.size() >= 1);

#ifdef DEBUG
  { // for debugging only
    rope_node_t node;
    node = rope_nodes.back();
    MSG_DEBUG("grapple.node", "- %d,%d %f %d", node.pos.x, node.pos.y, node.angle, node.sense);
  }
#endif

  // remove last node
  rope_nodes.pop_back();

  m_fixation_point = rope_nodes.back().pos ;

  Point2i pos;
  ActiveCharacter().GetRelativeHandPosition(pos);

  ActiveCharacter().SetPhysFixationPointXY(m_fixation_point.x / PIXEL_PER_METER,
                                           m_fixation_point.y / PIXEL_PER_METER,
                                           (double)pos.x / PIXEL_PER_METER,
                                           (double)pos.y / PIXEL_PER_METER);
}

// =========================== Moves management

void Grapple::SetRopeSize(double length) const
{
  double delta = length - ActiveCharacter().GetRopeLength();
  ActiveCharacter().ChangePhysRopeSize (delta);
}

void Grapple::GoUp()
{
  if(Time::GetInstance()->Read()<last_mvt+DT_MVT)
    return;
  last_mvt = Time::GetInstance()->Read();

  delta_len = -0.1 ;
  ActiveCharacter().ChangePhysRopeSize (delta_len);
  ActiveCharacter().UpdatePosition();
  delta_len = 0 ;
}

void Grapple::StopUp()
{
  cable_sound.Stop();
}

void Grapple::GoDown()
{
  if(Time::GetInstance()->Read()<last_mvt+DT_MVT)
    return;
  last_mvt = Time::GetInstance()->Read();

  if (ActiveCharacter().GetRopeLength() >= cfg().max_rope_length / PIXEL_PER_METER)
    return;

  delta_len = 0.1 ;
  ActiveCharacter().ChangePhysRopeSize (delta_len) ;
  ActiveCharacter().UpdatePosition() ;
  delta_len = 0 ;
}

void Grapple::StopDown()
{
  cable_sound.Stop();
}

void Grapple::GoRight()
{
  if (!go_right) {
    cable_sound.Play("default", "weapon/grapple_cable");
  }
  go_right = true ;
  ActiveCharacter().SetExternForce(cfg().push_force,0);
  ActiveCharacter().SetDirection(DIRECTION_RIGHT);
  ActiveCharacter().UpdatePosition() ;
}

void Grapple::StopRight()
{
  go_right = false ;

  if (go_left || go_right)
    return ;

  ActiveCharacter().SetExternForce(0,0);
}

void Grapple::GoLeft()
{
  if (!go_left) {
    cable_sound.Play("default", "weapon/grapple_cable");
  }
  go_left = true ;
  ActiveCharacter().SetExternForce(-cfg().push_force,0);
  ActiveCharacter().SetDirection(DIRECTION_LEFT);
  ActiveCharacter().UpdatePosition() ;
}

void Grapple::StopLeft()
{
  go_left = false ;

  if (go_left || go_right)
    return ;

  ActiveCharacter().SetExternForce(0,0);
}

void Grapple::StartMovingLeft()
{
  move_left_pressed = true;
}

void Grapple::StopMovingLeft()
{
  move_left_pressed = false;
  StopLeft();
}

void Grapple::StartMovingRight()
{
  move_right_pressed = true;
}

void Grapple::StopMovingRight()
{
  move_right_pressed = false;
  StopRight();
}

void Grapple::StartMovingUp()
{
  move_up_pressed = true;
}

void Grapple::StopMovingUp()
{
  move_up_pressed = false;
  StopUp();
}

void Grapple::StartMovingDown()
{
  move_down_pressed = true;
  cable_sound.Play("default", "weapon/grapple_cable", -1);
}

void Grapple::StopMovingDown()
{
  move_down_pressed = false;
  StopDown();
}


// =========================== Keys management

void Grapple::HandleKeyPressed_Up(bool slowly)
{
  if (IsInUse())
    StartMovingUpForAllPlayers();
  else
    ActiveCharacter().HandleKeyPressed_Up(slowly);
}

void Grapple::HandleKeyReleased_Up(bool slowly)
{
  if (IsInUse())
    StopMovingUpForAllPlayers();
  else
    ActiveCharacter().HandleKeyReleased_Up(slowly);
}

void Grapple::HandleKeyPressed_Down(bool slowly)
{
  if (IsInUse())
    StartMovingDownForAllPlayers();
  else
    ActiveCharacter().HandleKeyPressed_Down(slowly);
}

void Grapple::HandleKeyReleased_Down(bool slowly)
{
  if (IsInUse())
    StopMovingDownForAllPlayers();
  else
    ActiveCharacter().HandleKeyReleased_Down(slowly);
}

void Grapple::HandleKeyPressed_MoveLeft(bool slowly)
{
  if (IsInUse())
    StartMovingLeftForAllPlayers();
  else
    ActiveCharacter().HandleKeyPressed_MoveLeft(slowly);
}

void Grapple::HandleKeyReleased_MoveLeft(bool slowly)
{
  if (IsInUse())
    StopMovingLeftForAllPlayers();
  else
    ActiveCharacter().HandleKeyReleased_MoveLeft(slowly);
}

void Grapple::HandleKeyPressed_MoveRight(bool slowly)
{
  if (IsInUse())
    StartMovingRightForAllPlayers();
  else
    ActiveCharacter().HandleKeyPressed_MoveRight(slowly);
}


void Grapple::HandleKeyReleased_MoveRight(bool slowly)
{
  if (IsInUse())
    StopMovingRightForAllPlayers();
  else
    ActiveCharacter().HandleKeyReleased_MoveRight(slowly);
}

void Grapple::StartShooting()
{
  if (!IsInUse())
    Weapon::StartShooting();
}

void Grapple::StopShooting()
{
  if (IsInUse())
    ActionStopUse();
  else
    Weapon::StopShooting();
}

void Grapple::PrintDebugRope()
{
  printf("%05d %05d %03.3f\n",
         ActiveCharacter().GetX(),
         ActiveCharacter().GetY(),
         ActiveCharacter().GetRopeAngle());

  for (std::list<rope_node_t>::iterator it = rope_nodes.begin();
       it != rope_nodes.end();
       it++) {

    printf("%05d %05d %03.3f %d\n", it->pos.x, it->pos.y,
           it->angle, it->sense);
  }
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
