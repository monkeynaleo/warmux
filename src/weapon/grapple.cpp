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
  Point2d pos(ActiveCharacter().GetPhysic()->GetPosition().x,ActiveCharacter().GetPhysic()->GetPosition().y-10);
  m_rope->SetPosition(pos);
  m_rope->GetPhysic()->Activate();
  m_initial_angle = ActiveCharacter().GetFiringAngle();
  m_rope->GetPhysic()->Impulse(500, m_initial_angle);


  //m_attaching = true;
  //  m_launch_time = Time::GetInstance()->Read() ;
  //
  //last_mvt=Time::GetInstance()->Read();




  //   if (TryAttachRope()) JukeBox::GetInstance()->Play("default", "weapon/grapple_attaching");*/
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

void Grapple::SignalTurnEnd()
{
  p_Deselect();
}

void Grapple::p_Deselect()
{

 // DetachRope();
  delete m_rope;
  m_rope = NULL;
}

void Grapple::Draw()
{
  Weapon::Draw();

  if (!IsInUse()) {
    return ;
  }

  if (m_rope) {
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
      cable_sound.Play("default", "weapon/grapple_cable", -1);
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
      cable_sound.Play("default", "weapon/grapple_cable", -1);
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

Rope::Rope():GameObj("rope")
{

  for(int i = 0; i< 70 ; i++) {
    m_rope_nodes.push_back(new RopeNode());
  }

  Profile *weapons_res_profile = GetResourceManager().LoadXMLProfile( "weapons.xml", false);
  m_hook_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_hook");
  m_hook_sprite->EnableRotationCache(32);

  //m_body->
  //Hook shape
//  PhysicalCircle *hook_shape = new PhysicalCircle();
/*
  b2FilterData filter_data;
  filter_data.categoryBits = 0x0001;
  filter_data.maskBits = 0x0000;
  filter_data.groupIndex = -1;
*/
  // Shape position is relative to body
/*  hook_shape->SetRadius(0.8);
  hook_shape->SetPosition(Point2d(0,0));
  hook_shape->SetFriction(0.02f);
  hook_shape->SetFilter(filter_data);
  hook_shape->SetName("hook");
*/
 /* ClearShapes();
  m_shapes.push_back(hook_shape);
*/
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_GROUND,true);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_CHARACTER,false);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_ITEM,true);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_PROJECTILE,false);
}

Rope::~Rope()
{
  /*for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    delete m_rope_nodes[i];
  }
  for(unsigned i = 0; i< m_rope_anchor.size() ; i++) {
    PhysicalEngine::GetInstance()->GetPhysicWorld()->DestroyBody(m_rope_anchor[i]);
  }*/

}
/*
void Rope::Activate()
{
  GameObj::Activate();
*/
  //place nodes for generation
 /* for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    m_rope_nodes[i]->SetY(GetY() - i*5);
    m_rope_nodes[i]->SetX(GetX());
    m_rope_nodes[i]->Activate();
  }*/
/*
  //place hook for generation
  SetY(GetY() - m_rope_nodes.size()*5  );

  //prepare joint template
  b2BodyDef body_def;


  b2MassData mass;
  b2RevoluteJointDef joint_def_revolution;
  joint_def_revolution.collideConnected = false;

  b2DistanceJointDef joint_def_distance;
  joint_def_distance.collideConnected = false;
  joint_def_distance.dampingRatio = 0;


  b2PrismaticJointDef joint_def_prismatic;
  joint_def_prismatic.enableLimit = true;
  joint_def_prismatic.lowerTranslation = 0.1;

  ///////////////////////////
  //Attach character to nodes
  b2Body *node_anchor;
  body_def.position = ActiveCharacter().GameObj::GetBody()->GetWorldCenter();

  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    //create anchor
    node_anchor = PhysicalEngine::GetInstance()->GetPhysicWorld()->CreateBody(&body_def);
    m_rope_anchor.push_back(node_anchor);
    //set anchor mass
    mass.mass = m_rope_nodes[i]->GetBody()->GetMass();
    mass.I = m_rope_nodes[i]->GetBody()->GetInertia();
    node_anchor->SetMass(&mass);

    //attach anchor
    joint_def_revolution.Initialize(node_anchor, ActiveCharacter().GameObj::GetBody(), ActiveCharacter().GameObj::GetBody()->GetWorldCenter() );
    PhysicalEngine::GetInstance()->CreateJoint(&joint_def_revolution);

    //attach prismatic link
    joint_def_prismatic.Initialize(node_anchor,m_rope_nodes[i]->GetBody(),m_rope_nodes[i]->GetBody()->GetWorldCenter(), b2Vec2(0, 1));
    joint_def_prismatic.upperTranslation = ( node_anchor->GetWorldCenter().y - m_rope_nodes[i]->GetBody()->GetWorldCenter().y);
  //  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_prismatic);
  }

  ///////////////////////////
  //Attach hook to nodes
  b2Body *node_hook;
  body_def.position = GetBody()->GetWorldCenter();
  joint_def_prismatic.enableLimit = true;
  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    //create anchor
    node_hook = PhysicalEngine::GetInstance()->GetPhysicWorld()->CreateBody(&body_def);
    m_rope_anchor.push_back(node_hook);
    //set anchor mass
    mass.mass = m_rope_nodes[i]->GetBody()->GetMass();
    mass.I = m_rope_nodes[i]->GetBody()->GetInertia();
    node_hook->SetMass(&mass);

    //attach anchor
    joint_def_revolution.Initialize(node_hook, GetBody(), GetBody()->GetWorldCenter() );
  //  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_revolution);

    //attach prismatic link
    joint_def_prismatic.Initialize(node_hook,m_rope_nodes[i]->GetBody(),m_rope_nodes[i]->GetBody()->GetWorldCenter(), b2Vec2(0, -1));
    joint_def_prismatic.upperTranslation = (  m_rope_nodes[i]->GetBody()->GetWorldCenter().y - node_anchor->GetWorldCenter().y);
  //  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_prismatic);
  }

  ///////////////////////////
  //Attach character to hook
  body_def.position = ActiveCharacter().GameObj::GetBody()->GetWorldCenter();
  //create anchor
  b2Body *hook_anchor = PhysicalEngine::GetInstance()->GetPhysicWorld()->CreateBody(&body_def);
  m_rope_anchor.push_back(hook_anchor);

  //set anchor mass
  mass.mass = GetBody()->GetMass();
  mass.I = GetBody()->GetInertia();
  hook_anchor->SetMass(&mass);


  //attach anchor
  joint_def_revolution.Initialize(hook_anchor, ActiveCharacter().GameObj::GetBody(), ActiveCharacter().GameObj::GetBody()->GetWorldCenter() );
  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_revolution);

  //attach prismatic link
  joint_def_prismatic.Initialize(hook_anchor,GetBody(),GetBody()->GetWorldCenter(), b2Vec2(0, 1));
  joint_def_prismatic.upperTranslation = ( hook_anchor->GetWorldCenter().y - GetBody()->GetWorldCenter().y );
  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_prismatic);

  ///////////////////////
  // Unify rope
  for(unsigned i = 1; i< m_rope_nodes.size() ; i++) {

    joint_def_distance.Initialize(m_rope_nodes[i]->GetBody(), m_rope_nodes[i-1]->GetBody(), m_rope_nodes[i]->GetBody()->GetWorldCenter(), m_rope_nodes[i-1]->GetBody()->GetWorldCenter());
    PhysicalEngine::GetInstance()->CreateJoint(&joint_def_distance);
    //joint_def_revolution.Initialize(m_rope_nodes[i]->GetBody(), m_rope_nodes[i-1]->GetBody(), m_rope_nodes[i]->GetBody()->GetWorldCenter());
    //PhysicalEngine::GetInstance()->CreateJoint(&joint_def_revolution);

  }

  /////////////////////
  // Attach rope to hook
  joint_def_distance.Initialize(m_rope_nodes[m_rope_nodes.size()-1]->GetBody(), GetBody(), m_rope_nodes[m_rope_nodes.size()-1]->GetBody()->GetWorldCenter() ,GetBody()->GetWorldCenter());
  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_distance);

  /////////////////////////
  // Attach rope to Character
  joint_def_distance.Initialize(m_rope_nodes[0]->GetBody(), ActiveCharacter().GameObj::GetBody(), m_rope_nodes[0]->GetBody()->GetWorldCenter() ,ActiveCharacter().GameObj::GetBody()->GetWorldCenter());
  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_distance);


  ///////////////////
  // Compact generated rope

  //compact hook
  SetY(GetY() + m_rope_nodes.size()*5 );
  //compact nodes
  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    m_rope_nodes[i]->SetY(GetY());
    m_rope_nodes[i]->SetX(GetX());
  }
*/
/*}
*/
//void Rope::Generate()
//{
/*  GameObj::Generate();
  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    m_rope_nodes[i]->Generate();
  }*/
//}

//void Rope::Desactivate()
//{
 /* GameObj::Desactivate();
  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    m_rope_nodes[i]->Desactivate();
  }*/
//}

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

 /* m_hook_sprite->SetRotation_rad(-GetAngle());
  m_hook_sprite->Draw( GetPhysic()->GetPosition() - m_hook_sprite->GetSize()/2);

  for(unsigned i = 0; i< m_rope_nodes.size() ; i++) {
    m_rope_nodes[i]->Draw();
  }
#ifdef DEBUG
  DrawPolygon(Color(0,0,0));
#endif*/
}

void Rope::SignalGroundCollision(const Point2d&)
{
 /* b2RevoluteJointDef joint_def_revolution;
  joint_def_revolution.collideConnected = false;

  joint_def_revolution.Initialize(PhysicalEngine::GetInstance()->GetPhysicWorld()->GetGroundBody(), GetBody(), GetBody()->GetWorldCenter() );
  PhysicalEngine::GetInstance()->CreateJoint(&joint_def_revolution);
*/
}


//-----------------------------------------------------------------------------
// RopeNode

RopeNode::RopeNode():GameObj("rope_node")
{
 /* Profile *weapons_res_profile = GetResourceManager().LoadXMLProfile( "weapons.xml", false);
  m_node_sprite = GetResourceManager().LoadSprite(weapons_res_profile,"grapple_node");

  m_node_sprite->EnableRotationCache(32);

  //Node shape
  PhysicalCircle *node_shape = new PhysicalCircle();

  b2FilterData filter_data;
  filter_data.categoryBits = 0x0001;
  filter_data.maskBits = 0x0000;
  filter_data.groupIndex = -1;

  // Shape position is relative to body
  node_shape->SetRadius(0.10);
  node_shape->SetPosition(Point2d(0,0));
  node_shape->SetFriction(0.01f);
  node_shape->SetFilter(filter_data);
  node_shape->SetName("node");

  ClearShapes();
  m_shapes.push_back(node_shape);

  SetCollisionModel(true,false, true, false);*/
}


void RopeNode::Draw()
{
/*
  m_node_sprite->SetRotation_rad(-GetAngle());
  m_node_sprite->Draw( GetPhysic()->GetPosition() - m_node_sprite->GetSize()/2);
#ifdef DEBUG
  DrawPolygon(Color(0,0,0));
#endif*/
}


