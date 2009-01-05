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

#include "character/character.h"
#include "team/teams_list.h"
#include "graphic/sprite.h"
#include "physic/physical_engine.h"
#include "physic/physical_shape.h"

/*
 #include <math.h>
 #include "weapon/explosion.h"

 #include "game/config.h"
 #include "game/game.h"
 #include "game/time.h"

 #include "include/action_handler.h"
 #include "map/camera.h"
 #include "map/map.h"
 #include "network/network.h"

 #include "sound/jukebox.h"
 #include "team/team.h"

 #include "tool/math_tools.h"
 */

#include "tool/resource_manager.h"

#include "tool/xml_document.h"

const uint DT_MVT = 15; //delta_t between 2 up/down/left/right mvt
const uint DST_MIN = 80; //dst_minimal between 2 nodes


class GrappleConfig : public EmptyWeaponConfig
{
public:
  uint max_rope_length; // Max rope length in pixels
  uint automatic_growing_speed; // Pixel per 1/100 second.
  int push_force;

public:
  GrappleConfig();
  void
  LoadXml(const xmlNode* elem);
};

//-----------------------------------------------------------------------------

Grapple::Grapple() :
  Weapon(WEAPON_GRAPPLE, "grapple", new GrappleConfig()),
  m_rope(NULL)
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

void
Grapple::UpdateTranslationStrings()
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

bool
Grapple::p_Shoot()
{
  ASSERT(m_rope == NULL);
  m_rope = new Rope();
  m_rope->SetXY(ActiveCharacter().GetPosition());
  m_rope->Activate();
  m_initial_angle = ActiveCharacter().GetFiringAngle();
  m_rope->Impulse(2000, m_initial_angle);

  //m_attaching = true;
//  m_launch_time = Time::GetInstance()->Read() ;
  //
  //last_mvt=Time::GetInstance()->Read();




//   if (TryAttachRope()) JukeBox::GetInstance()->Play("share", "weapon/grapple_attaching");*/
  return true;
}

void
Grapple::Refresh()
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

void
Grapple::Draw()
{
  Weapon::Draw();

  if (!IsInUse())
  {
  return ;
  }

  if(m_rope) {
    m_rope->Draw();
  }
  /* int x, y;
   double angle, prev_angle;

   struct CL_Quad {Sint16 x1,x2,x3,x4,y1,y2,y3,y4;} quad;



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

void
Grapple::HandleKeyPressed_Up(bool shift)
{
  if (IsInUse())
    {
      cable_sound.Play("share", "weapon/grapple_cable", -1);
      //  GoUp();
    }
  // else
   ActiveCharacter().HandleKeyPressed_Up(shift);
}

void
Grapple::HandleKeyRefreshed_Up(bool shift)
{
  //if (IsInUse())
  // GoUp();
  // else
    ActiveCharacter().HandleKeyRefreshed_Up(shift);
}

void
Grapple::HandleKeyReleased_Up(bool shift)
{
  //if (IsInUse())
  // StopUp();
  // else
   ActiveCharacter().HandleKeyReleased_Up(shift);
}

void
Grapple::HandleKeyPressed_Down(bool shift)
{
  if (IsInUse())
    {
      cable_sound.Play("share", "weapon/grapple_cable", -1);
      // GoDown();
    }// else
    ActiveCharacter().HandleKeyPressed_Down(shift);
}

void
Grapple::HandleKeyRefreshed_Down(bool shift)
{
  //if (IsInUse())
  //GoDown();
  //else
   ActiveCharacter().HandleKeyRefreshed_Down(shift);
}

void
Grapple::HandleKeyReleased_Down(bool shift)
{
  //if (IsInUse())
  //StopDown();
  //else
    ActiveCharacter().HandleKeyReleased_Down(shift);
}

void
Grapple::HandleKeyPressed_MoveLeft(bool shift)
{
  // if (IsInUse())
  // GoLeft();
  // else
    ActiveCharacter().HandleKeyPressed_MoveLeft(shift);
}

void
Grapple::HandleKeyRefreshed_MoveLeft(bool shift)
{
  // if (!IsInUse())
   ActiveCharacter().HandleKeyRefreshed_MoveLeft(shift);
}

void
Grapple::HandleKeyReleased_MoveLeft(bool shift)
{
  // if (IsInUse())
  // StopLeft();
  // else
   ActiveCharacter().HandleKeyReleased_MoveLeft(shift);
}

void
Grapple::HandleKeyPressed_MoveRight(bool shift)
{
  // if (IsInUse())
  // GoRight();
  // else
   ActiveCharacter().HandleKeyPressed_MoveRight(shift);
}

void
Grapple::HandleKeyRefreshed_MoveRight(bool shift)
{
  //
	 ActiveCharacter().HandleKeyRefreshed_MoveRight(shift);
}

void
Grapple::HandleKeyReleased_MoveRight(bool shift)
{
  // if (IsInUse())
  // StopRight();
  //else
   ActiveCharacter().HandleKeyReleased_MoveRight(shift);
}

void
Grapple::HandleKeyPressed_Shoot(bool)
{
  if (IsInUse())
    {
      NewActionWeaponStopUse();
    }
  else
    NewActionWeaponShoot();
}

void
Grapple::PrintDebugRope()
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

std::string
Grapple::GetWeaponWinString(const char *TeamName, uint items_count) const
{
  return Format(ngettext("%s team has won %u grapple!",
      "%s team has won %u grapples!", items_count), TeamName, items_count);
}

//-----------------------------------------------------------------------------

GrappleConfig&
Grapple::cfg()
{
  return static_cast<GrappleConfig&> (*extra_params);
}
//-----------------------------------------------------------------------------

GrappleConfig::GrappleConfig()
{
  max_rope_length = 450;
  automatic_growing_speed = 12;
  push_force = 10;
}

void
GrappleConfig::LoadXml(const xmlNode* elem)
{
  EmptyWeaponConfig::LoadXml(elem);
  XmlReader::ReadUint(elem, "max_rope_length", max_rope_length);
  XmlReader::ReadUint(elem, "automatic_growing_speed", automatic_growing_speed);
  XmlReader::ReadInt(elem, "push_force", push_force);
}






//-----------------------------------------------------------------------------
// Rope

Rope::Rope():PhysicalObj("rope")
{

  for(int i = 0; i< 50 ; i++) {
   m_rope_nodes.push_back(new RopeNode());

  }
  Profile *weapons_res_profile = GetResourceManager().LoadXMLProfile( "weapons.xml", false);
  m_hook_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_hook");
  m_hook_sprite->EnableRotationCache(32);

  //m_body->
  //Hook shape
  PhysicalCircle *hook_shape = new PhysicalCircle();

  b2FilterData filter_data = {0,0,0};
    filter_data.categoryBits = 0x0001;
    filter_data.maskBits = 0x0000;
    filter_data.groupIndex = -1;
  // Shape position is relative to body
  hook_shape->SetRadius(0.5);
  hook_shape->SetMass(1);
  hook_shape->SetPosition(Point2d(0,0));
  hook_shape->SetFriction(1.2f);
  hook_shape->SetFilter(filter_data);
  hook_shape->SetName("hook");

  ClearShapes();
  m_shapes.push_back(hook_shape);

  SetCollisionModel(true,false, true);
  SetFixedRotation(false);
}

Rope::~Rope()
{
  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    delete m_rope_nodes[i];
   }

}

void Rope::Activate()
{
  PhysicalObj::Activate();

  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    m_rope_nodes[i]->SetY(GetY());
    m_rope_nodes[i]->SetX(GetX()+ i*5);

    m_rope_nodes[i]->Activate();
   }

     b2RevoluteJointDef jointDef;
     //jointDef.Initialize(myBody1, myBody2, myBody1->GetWorldCenter());

 // b2DistanceJointDef jointDef;
  jointDef.collideConnected = false;
 // jointDef.dampingRatio = 10.0;
  for(unsigned i = 1; i< m_rope_nodes.size() ; i++) {

     // jointDef.Initialize(m_rope_nodes[i]->GetBody(), m_rope_nodes[i-1]->GetBody(), m_rope_nodes[i]->GetBody()->GetWorldCenter(), m_rope_nodes[i-1]->GetBody()->GetWorldCenter());
    jointDef.Initialize(m_rope_nodes[i]->GetBody(), m_rope_nodes[i-1]->GetBody(), m_rope_nodes[i]->GetBody()->GetWorldCenter());
      PhysicalEngine::GetInstance()->CreateJoint(&jointDef);

   }

  //Attach Hook
  //jointDef.Initialize(m_rope_nodes[0]->GetBody(), GetBody(), m_rope_nodes[0]->GetBody()->GetWorldCenter(), GetBody()->GetWorldCenter());
  jointDef.Initialize(m_rope_nodes[0]->GetBody(), GetBody(), m_rope_nodes[0]->GetBody()->GetWorldCenter());
  PhysicalEngine::GetInstance()->CreateJoint(&jointDef);

  //Attach character
  //jointDef.Initialize(m_rope_nodes[m_rope_nodes.size()-1]->GetBody(), ActiveCharacter().PhysicalObj::GetBody(), GetBody()->GetWorldCenter(), ActiveCharacter().PhysicalObj::GetBody()->GetWorldCenter());
  jointDef.Initialize(m_rope_nodes[m_rope_nodes.size()-1]->GetBody(), ActiveCharacter().PhysicalObj::GetBody(), GetBody()->GetWorldCenter());
 // PhysicalEngine::GetInstance()->CreateJoint(&jointDef);


  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {

    m_rope_nodes[i]->SetY(GetY());
    m_rope_nodes[i]->SetX(GetX());

   }

 // m_rope_nodes[m_rope_nodes.size()-1]->SetY(ActiveCharacter().GetY());
 // m_rope_nodes[m_rope_nodes.size()-1]->SetX(ActiveCharacter().GetX());
}

void Rope::Generate()
{
  PhysicalObj::Generate();
  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
       m_rope_nodes[i]->Generate();
   }
}

void Rope::Desactivate()
{
  PhysicalObj::Desactivate();
  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
       m_rope_nodes[i]->Desactivate();
  }
}

void Rope::Draw()
{
  /*
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
*/

  m_hook_sprite->SetRotation_rad(-GetAngle());
   m_hook_sprite->Draw( GetPosition() - m_hook_sprite->GetSize()/2);

   for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
       m_rope_nodes[i]->Draw();
      }
#ifdef DEBUG
   DrawPolygon(Color(0,0,0));
#endif
}

void Rope::SignalGroundCollision(const Point2d&)
{
  SetFixed(true);
}


//-----------------------------------------------------------------------------
// RopeNode

RopeNode::RopeNode():PhysicalObj("rope_node")
{
  Profile *weapons_res_profile = GetResourceManager().LoadXMLProfile( "weapons.xml", false);
  m_node_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_node");

  m_node_sprite->EnableRotationCache(32);

   //Node shape
   PhysicalCircle *node_shape = new PhysicalCircle();

   b2FilterData filter_data = {0,0,0};
     filter_data.categoryBits = 0x0001;
     filter_data.maskBits = 0x0000;
     filter_data.groupIndex = -1;
   // Shape position is relative to body
   node_shape->SetRadius(0.1);
   node_shape->SetMass(0.01);
   node_shape->SetPosition(Point2d(0,0));
   node_shape->SetFriction(1.2f);
   node_shape->SetFilter(filter_data);
   node_shape->SetName("node");

   ClearShapes();
   m_shapes.push_back(node_shape);

   SetCollisionModel(true,false, true);
   SetFixedRotation(false);

}


void RopeNode::Draw()
{

  m_node_sprite->SetRotation_rad(-GetAngle());
	  m_node_sprite->Draw( GetPosition() - m_node_sprite->GetSize()/2);
#ifdef DEBUG
   DrawPolygon(Color(0,0,0));
#endif
}


