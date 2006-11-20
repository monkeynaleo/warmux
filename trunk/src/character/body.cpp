/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
 * Character of a team.
 *****************************************************************************/
#include "body.h"
#include <sstream>
#include <iostream>
#include <map>
#include "clothe.h"
#include "member.h"
#include "../team/teams_list.h"
#include "../game/time.h"
#include "../tool/debug.h"
#include "../tool/random.h"
#include "../tool/resource_manager.h"
#include "../tool/xml_document.h"
#include "../particles/body_member.h"
#include "../particles/teleport_member.h"

Body::Body(xmlpp::Element* xml, Profile* res)
{
  need_rebuild = true;
  current_clothe = NULL;
  current_mvt = NULL;
  walk_events = 0;
  animation_number = 0;
  direction = DIRECTION_RIGHT;
  main_rotation = 0;

  // Load members
  xmlpp::Node::NodeList nodes = xml -> get_children("sprite");
  xmlpp::Node::NodeList::iterator it=nodes.begin();

  while(it != nodes.end())
   {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it);
    std::string name;
    LitDocXml::LitAttrString( elem, "name", name);

    Member* member = new Member(elem, res);
    if(members_lst.find(name) != members_lst.end())
      std::cerr << "Warning !! The member \""<< name << "\" is defined twice in the xml file" << std::endl;
    else
      members_lst[name] = member;

    it++;
  }

  // Add a special weapon member to the body
  weapon_member = new WeaponMember();
  members_lst["weapon"] = weapon_member;

  // Load clothes
  xmlpp::Node::NodeList nodes2 = xml -> get_children("clothe");
  xmlpp::Node::NodeList::iterator it2=nodes2.begin();

  while(it2 != nodes2.end())
  {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it2);
    std::string name;
    LitDocXml::LitAttrString( elem, "name", name);

    Clothe* clothe = new Clothe(elem, members_lst);
    if (clothes_lst.find(name) != clothes_lst.end())
      std::cerr << "Warning !! The clothe \""<< name << "\" is defined twice in the xml file" << std::endl;
    else
      clothes_lst[name] = clothe;

    it2++;
  }

  // Load movements alias
  xmlpp::Node::NodeList nodes4 = xml -> get_children("alias");
  xmlpp::Node::NodeList::iterator it4=nodes4.begin();
  std::map<std::string, std::string> mvt_alias;
  while(it4 != nodes4.end())
   {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it4);
    std::string mvt, corresp;
    LitDocXml::LitAttrString( elem, "movement", mvt);
    LitDocXml::LitAttrString( elem, "correspond_to", corresp);
    mvt_alias.insert(std::make_pair(mvt,corresp));
    it4++;
  }

  // Load movements
  xmlpp::Node::NodeList nodes3 = xml -> get_children("movement");
  xmlpp::Node::NodeList::iterator it3=nodes3.begin();

  while(it3 != nodes3.end())
  {
    xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it3);
    std::string name;
    LitDocXml::LitAttrString( elem, "name", name);
    if(strncmp(name.c_str(),"animation", 9)==0)
      animation_number++;

    Movement* mvt = new Movement(elem);
    if(mvt_lst.find(name) != mvt_lst.end())
      std::cerr << "Warning !! The movement \""<< name << "\" is defined twice in the xml file" << std::endl;
    else
      mvt_lst[name] = mvt;

    for(std::map<std::string, std::string>::iterator it = mvt_alias.begin();
        it != mvt_alias.end();  ++it)
    if(it->second == name)
    {
      Movement* mvt = new Movement(elem);
      mvt->type = it->first;
      mvt_lst[it->first] = mvt;
    }
    it3++;
  }
}

Body::Body(Body *_body)
{
  need_rebuild = true;
  current_clothe = NULL;
  current_mvt = NULL;
  walk_events = 0;
  animation_number = _body->animation_number;
  direction = DIRECTION_RIGHT;
  main_rotation = 0;

  // Add a special weapon member to the body
  weapon_member = new WeaponMember();
  members_lst["weapon"] = weapon_member;

  // Make a copy of members
  std::map<std::string, Member*>::iterator it1 = _body->members_lst.begin();
  while(it1 != _body->members_lst.end())
  if(it1->second->name != "weapon")
  {
    std::pair<std::string,Member*> p;
    p.first = it1->first;
    p.second = new Member(it1->second);
    members_lst.insert(p);
    it1++;
  }
  else
    it1++;

  // Make a copy of clothes
  std::map<std::string, Clothe*>::iterator it2 = _body->clothes_lst.begin();
  while(it2 != _body->clothes_lst.end())
  {
    std::pair<std::string,Clothe*> p;
    p.first = it2->first;
    p.second = new Clothe(it2->second, members_lst);
    clothes_lst.insert(p);
    it2++;
  }

  // Movement are shared
  std::map<std::string, Movement*>::iterator it3 = _body->mvt_lst.begin();
  while(it3 != _body->mvt_lst.end())
  {
    std::pair<std::string,Movement*> p;
    p.first = it3->first;
    p.second = it3->second;
    mvt_lst.insert(p);
    it3++;
  }
}

Body::~Body()
{
  // Pointers inside those lists are freed from the body_list
  // Clean the members list
  std::map<std::string, Member*>::iterator it = members_lst.begin();
  while(it != members_lst.end())
  {
    delete it->second;
    it++;
  }

  // Clean the clothes list
  std::map<std::string, Clothe*>::iterator it2 = clothes_lst.begin();
  while(it2 != clothes_lst.end())
  {
    delete it2->second;
    it2++;
  }

  members_lst.clear();
  clothes_lst.clear();
  mvt_lst.clear();
}

void Body::ResetMovement()
{
  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
    current_clothe->layers[layer]->ResetMovement();
}

void Body::ApplyMovement(Movement* mvt, uint frame)
{
  // Move each member following the movement description
  // We do it using the order of the squeleton, as the movement of each
  // member affect the child members as well
  std::vector<junction>::iterator member = squel_lst.begin();
  for(;member != squel_lst.end();
       member++)
  {
    assert( frame < mvt->frames.size() );
    if(mvt->frames[frame].find(member->member->type) != mvt->frames[frame].end())
    {
      // This member needs to be moved :
      member_mvt mb_mvt = mvt->frames[frame].find(member->member->type)->second;
      if(mb_mvt.follow_crosshair && ActiveCharacter().body == this && ActiveTeam().AccessWeapon().UseCrossHair())
      {
        // Use the movement of the crosshair
        int angle = ActiveTeam().crosshair.GetAngle(); // returns -180 < angle < 180
        if(angle < 0)
          angle += 360; // so now 0 < angle < 360;
        if(ActiveCharacter().GetDirection() == DIRECTION_LEFT)
          angle = 180 - angle;

        mb_mvt.angle += angle ;
      }

      if(mb_mvt.follow_half_crosshair && ActiveCharacter().body == this && ActiveTeam().AccessWeapon().UseCrossHair())
      {
        // Use the movement of the crosshair
        int angle = ActiveTeam().crosshair.GetAngle(); // returns -180 < angle < 180
        if(ActiveCharacter().GetDirection() == DIRECTION_RIGHT)
          angle /= 2; // -90 < angle < 90
        else
        if(angle > 90)
          angle = 45 + (90 - angle) / 2;
        else
          angle = -45 + (-90 - angle) / 2;



        if(angle < 0)
          angle += 360; // so now 0 < angle < 360;

        mb_mvt.angle += angle ;
      }

      if(mb_mvt.follow_speed)
      {
        // Use the movement of the character
        int angle = (int)(owner->GetSpeedAngle()/M_PI*180.0);
        if(angle < 0)
          angle += 360; // so now 0 < angle < 360;
        if(owner->GetDirection() == DIRECTION_LEFT)
          angle = 180 - angle;

        mb_mvt.angle += angle;
      }

      if(mb_mvt.follow_direction)
      {
        // Use the direction of the character
        if(owner->GetDirection() == DIRECTION_LEFT)
          mb_mvt.angle += 180;
      }


      member->member->ApplyMovement(mb_mvt, squel_lst);
    }
  }
}

void Body::ApplySqueleton()
{
  // Move each member following the squeleton
  std::vector<junction>::iterator member = squel_lst.begin();
  // The first member is the body, we set it to pos:
  member->member->pos = Point2f(0,0);
  member->member->angle = 0;
  member++;

  for(;member != squel_lst.end();
       member++)
  {
    // Place the other members depending of the parent member:
    member->member->ApplySqueleton(member->parent);
  }
}

void Body::Build()
{
  // Increase frame number if needed
  unsigned int last_frame = current_frame;
  if(walk_events > 0 || current_mvt->type!="walk")
  if(Time::GetInstance()->Read() > last_refresh + current_mvt->speed)
  {
    current_frame += (Time::GetInstance()->Read()-last_refresh) / current_mvt->speed;
    last_refresh += ((Time::GetInstance()->Read()-last_refresh) / current_mvt->speed) * current_mvt->speed;

    if(current_frame >= current_mvt->frames.size())
    {
      if(play_once_clothe_sauv)
        SetClothe(play_once_clothe_sauv->name);
      if(play_once_mvt_sauv)
      {
        SetMovement(play_once_mvt_sauv->type);
        current_frame = play_once_frame_sauv;
      }
    }
    current_frame %= current_mvt->frames.size();
  }

  need_rebuild |= (last_frame != current_frame);
  need_rebuild |= current_mvt->always_moving;

  if(!need_rebuild)
    return;

  ResetMovement();
  ApplySqueleton();
  ApplyMovement(current_mvt, current_frame);

  // Rotate each sprite, because the next part need to know the height of the sprite
  // once he is rotated
  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
  if(current_clothe->layers[layer]->name != "weapon")
    current_clothe->layers[layer]->RotateSprite();

  // Move the members to get the lowest member at the bottom of the skin rectangle
  member_mvt body_mvt;
  float y_max = 0;
  for(int lay=0;lay < (int)current_clothe->layers.size() ;lay++)
  if(current_clothe->layers[lay]->name != "weapon")
  {
    Member* member = current_clothe->layers[lay];
    if(member->pos.y + member->spr->GetHeightMax() + member->spr->GetRotationPoint().y > y_max
    && !member->go_through_ground)
      y_max = member->pos.y + member->spr->GetHeightMax() + member->spr->GetRotationPoint().y;
  }
  body_mvt.pos.y = (float)GetSize().y - y_max + current_mvt->test_bottom;
  body_mvt.pos.x = GetSize().x / 2.0 - squel_lst.front().member->spr->GetWidth() / 2.0;
  body_mvt.angle = main_rotation;
  squel_lst.front().member->ApplyMovement(body_mvt, squel_lst);

  need_rebuild = false;
}

void Body::Draw(const Point2i& _pos)
{
  Build();

  // update the weapon position
  if(direction == DIRECTION_RIGHT)
    weapon_pos = Point2i((int)weapon_member->pos.x,(int)weapon_member->pos.y);
  else
    weapon_pos = Point2i(GetSize().x - (int)weapon_member->pos.x,(int)weapon_member->pos.y);
  weapon_pos += _pos;

  // Finally draw each layer one by one
  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
    current_clothe->layers[layer]->Draw(_pos, _pos.x + GetSize().x/2, int(direction));
}

void Body::AddChildMembers(Member* parent)
{
  // Add child members of the parent member to the squeleton
  // and continue recursively with child members
  for(std::map<std::string, v_attached>::iterator child = parent->attached_members.begin();
      child != parent->attached_members.end();
      child++)
  {
    // Find if the current clothe uses this member:
    for(uint lay = 0; lay < current_clothe->layers.size(); lay++)
    {
      if(current_clothe->layers[lay]->type == child->first)
      {
        // This child member is attached to his parent
        junction body;
        body.member = current_clothe->layers[lay];
        body.parent = parent;
        squel_lst.push_back(body);

        // continue recursively
        AddChildMembers(current_clothe->layers[lay]);
      }
    }
  }
}

void Body::BuildSqueleton()
{
  // Find each member used by the current clothe
  // and set the parent member of each member
  squel_lst.clear();

  // Find the "body" member as its the top of the squeleton
  for(uint lay = 0; lay < current_clothe->layers.size(); lay++)
  if(current_clothe->layers[lay]->type == "body")
  {
    junction body;
    body.member = current_clothe->layers[lay];
    body.parent = NULL;
    squel_lst.push_back(body);
    break;
  }

  if(squel_lst.size() == 0)
  {
    std::cerr << "Unable to find the \"body\" member in the current clothe" << std::endl;
    assert(false);
  }

  AddChildMembers(squel_lst.front().member);
}

void Body::SetClothe(std::string name)
{
  MSG_DEBUG("body", " %s use clothe %s", owner->GetName().c_str(), name.c_str());
  if(current_clothe && current_clothe->name == name) return;

  if(clothes_lst.find(name) != clothes_lst.end())
  {
    current_clothe = clothes_lst.find(name)->second;
    BuildSqueleton();
    main_rotation = 0;
    need_rebuild = true;
  }
  else
    MSG_DEBUG("body","Clothe not found");


  play_once_clothe_sauv = NULL;

  assert(current_clothe != NULL);
}

void Body::SetMovement(std::string name)
{
  MSG_DEBUG("body", " %s use movement %s", owner->GetName().c_str(), name.c_str());
  if(current_mvt && current_mvt->type == name) return;

  // Dirty trick to get the "black" movement to be played fully
  if(current_clothe && current_clothe->name == "black") return;

  if(mvt_lst.find(name) != mvt_lst.end())
  {
    current_mvt = mvt_lst.find(name)->second;
    current_frame = 0;
    last_refresh = Time::GetInstance()->Read();
    main_rotation = 0;
    need_rebuild = true;
  }
  else
    MSG_DEBUG("body","Movement not found");

  play_once_mvt_sauv = NULL;

  assert(current_mvt != NULL);
}

void Body::PlayAnimation()
{
  std::ostringstream name;
  name << "animation" << randomObj.GetLong(0, animation_number - 1);
  SetClotheOnce(name.str());
  SetMovementOnce(name.str());
}

void Body::SetClotheOnce(std::string name)
{
  MSG_DEBUG("body", " %s use clothe %s once", owner->GetName().c_str(), name.c_str());
  if(current_clothe && current_clothe->name == name) return;

  if(clothes_lst.find(name) != clothes_lst.end())
  {
    if(!play_once_clothe_sauv)
      play_once_clothe_sauv = current_clothe;
    current_clothe = clothes_lst.find(name)->second;
    BuildSqueleton();
    main_rotation = 0;
    need_rebuild = true;
  }
  else
    MSG_DEBUG("body","Clothe not found");

  assert(current_clothe != NULL);
}

void Body::SetMovementOnce(std::string name)
{
  MSG_DEBUG("body", " %s use movement %s once", owner->GetName().c_str(), name.c_str());
  if(current_mvt && current_mvt->type == name) return;

  // Dirty trick to get the "black" movement to be played fully
  if(current_clothe && current_clothe->name == "black"  && name != "black") return;

  if(mvt_lst.find(name) != mvt_lst.end())
  {
    if(!play_once_mvt_sauv)
    {
      play_once_mvt_sauv = current_mvt;
      play_once_frame_sauv = current_frame;
    }

    current_mvt = mvt_lst.find(name)->second;
    current_frame = 0;
    last_refresh = Time::GetInstance()->Read();
    main_rotation = 0;
    need_rebuild = true;
  }
  else
    MSG_DEBUG("body","Movement not found");

  assert(current_mvt != NULL);
}

void Body::GetTestRect(uint &l, uint&r, uint &t, uint &b)
{
  if(direction == DIRECTION_RIGHT)
  {
    l = current_mvt->test_left;
    r = current_mvt->test_right;
  }
  else
  {
    r = current_mvt->test_left;
    l = current_mvt->test_right;
  }
  t = current_mvt->test_top;
  b = current_mvt->test_bottom;
}

void Body::SetDirection(Direction_t dir)
{
  direction=dir;
}

const Direction_t Body::GetDirection()
{
  return direction;
}

const Point2i &Body::GetHandPosition() const
{
  return weapon_pos;
}

void Body::StartWalk()
{
  // walk events happens when the player hits the left/right key
  // counting how much they are pressed allow to get skin walking
  // if the key was hit while the character was jumping or using an other
  // animation than the walk animation
  walk_events++;
  if(walk_events == 1)
    last_refresh = Time::GetInstance()->Read();
}

void Body::StopWalk()
{
  if(walk_events > 0)
    walk_events--;
  if(current_mvt->type == "walk")
    SetFrame(0);
}

void Body::ResetWalk()
{
  walk_events = 0;
}

uint Body::GetMovementDuration()
{
  return current_mvt->frames.size() * current_mvt->speed;
}

uint Body::GetFrameCount()
{
  return current_mvt->frames.size();
}

void Body::SetFrame(uint no)
{
  assert(no < current_mvt->frames.size());
  current_frame = no;
  need_rebuild = true;
}

void Body::MakeParticles(const Point2i& pos)
{
  Build();

  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
  if(current_clothe->layers[layer]->type != "weapon")
    ParticleEngine::AddNow(new BodyMemberParticle(current_clothe->layers[layer]->spr,
                                                  current_clothe->layers[layer]->GetPos()+pos));
}

void Body::MakeTeleportParticles(const Point2i& pos, const Point2i& dst)
{
  Build();

  for(int layer=0;layer < (int)current_clothe->layers.size() ;layer++)
  if(current_clothe->layers[layer]->type != "weapon")
  {
    ParticleEngine::AddNow(new TeleportMemberParticle(current_clothe->layers[layer]->spr,
						      current_clothe->layers[layer]->GetPos()+pos,
						      current_clothe->layers[layer]->GetPos()+dst,
						      int(direction)));
  }
}

void Body::SetRotation(int angle)
{
  MSG_DEBUG("body", "%s -> new angle: %i", owner->GetName().c_str(), angle);
  main_rotation = angle;
  need_rebuild = true;
}

const std::string& Body::GetMovement() { return current_mvt->type; }
const std::string& Body::GetClothe() { return current_clothe->name; }
