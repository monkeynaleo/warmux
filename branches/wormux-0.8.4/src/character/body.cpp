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
 * Character of a team.
 *****************************************************************************/
#include <sstream>
#include <iostream>
#include <map>
#include "character/body.h"
#include "character/character.h"
#include "character/clothe.h"
#include "character/member.h"
#include "character/movement.h"
#include "game/time.h"
#include "graphic/sprite.h"
#include "interface/mouse.h"
#include "particles/body_member.h"
#include "particles/teleport_member.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/debug.h"
#include "tool/random.h"
#include "tool/resource_manager.h"
#include "tool/xml_document.h"

Body::Body(const xmlNode* xml, const std::string& main_folder):
  members_lst(),
  clothes_lst(),
  mvt_lst(),
  current_clothe(NULL),
  current_mvt(NULL),
  current_loop(0),
  current_frame(0),
  previous_clothe(NULL),
  previous_mvt(NULL),
  weapon_member(new WeaponMember()),
  weapon_pos(0,0),
  last_refresh(0),
  walk_events(0),
  main_rotation_rad(0),
  skel_lst(),
  direction(DIRECTION_RIGHT),
  animation_number(0),
  need_rebuild(false),
  owner(NULL)
{
  const xmlNode *skeletons = XmlReader::GetMarker(xml, "skeletons");
  ASSERT(skeletons);

  xmlNodeArray nodes = XmlReader::GetNamedChildren(skeletons, "sprite");
  xmlNodeArray::const_iterator it;

  // Load members
  MSG_DEBUG("body", "Found %i sprites", nodes.size());
  for (it = nodes.begin(); it != nodes.end(); ++it)
  {
    std::string name;
    XmlReader::ReadStringAttr(*it, "name", name);

    MSG_DEBUG("body", "Loading member %s", name.c_str());
    Member* member = new Member(*it, main_folder);
    if (members_lst.find(name) != members_lst.end()) {
      std::cerr << "Warning !! The member \""<< name << "\" is defined twice in the xml file" << std::endl;
      ASSERT(false);
    } else
      members_lst[name] = member;
  }

  members_lst["weapon"] = weapon_member;

  // Load clothes
  const xmlNode *clothes = XmlReader::GetMarker(xml, "clothes");
  ASSERT(clothes);

  nodes = XmlReader::GetNamedChildren(clothes, "clothe");
  MSG_DEBUG("body", "Found %i clothes", nodes.size());
  for (it = nodes.begin(); it != nodes.end(); ++it)
  {
    std::string name;
    XmlReader::ReadStringAttr(*it, "name", name);

    Clothe* clothe = new Clothe(*it, members_lst);
    if (clothes_lst.find(name) != clothes_lst.end())
      std::cerr << "Warning !! The clothe \""<< name << "\" is defined twice in the xml file" << std::endl;
    else
      clothes_lst[name] = clothe;
  }

  // Load movements alias
  const xmlNode *aliases = XmlReader::GetMarker(xml, "aliases");
  ASSERT(aliases);

  std::map<std::string, std::string> mvt_alias;
  nodes = XmlReader::GetNamedChildren(aliases, "alias");
  MSG_DEBUG("body", "Found %i aliases", nodes.size());
  for (it = nodes.begin(); it != nodes.end(); ++it)
  {
    std::string mvt, corresp;
    XmlReader::ReadStringAttr(*it, "movement", mvt);
    XmlReader::ReadStringAttr(*it, "correspond_to", corresp);
    mvt_alias.insert(std::make_pair(mvt, corresp));
    MSG_DEBUG("body", "  %s -> %s", mvt.c_str(), corresp.c_str());
  }

  // Load movements
  const xmlNode *movements = XmlReader::GetMarker(xml, "movements");
  ASSERT(movements);

  nodes = XmlReader::GetNamedChildren(movements, "movement");
  MSG_DEBUG("body", "Found %i movements", nodes.size());
  for (it = nodes.begin(); it != nodes.end(); ++it)
  {
    std::string name;
    XmlReader::ReadStringAttr(*it, "name", name);
    if (strncmp(name.c_str(),"animation", 9)==0)
      animation_number++;

    Movement* mvt = new Movement(*it);
    if(mvt_lst.find(name) != mvt_lst.end())
      std::cerr << "Warning !! The movement \""<< name << "\" is defined twice in the xml file" << std::endl;
    else
      mvt_lst[name] = mvt;

    for(std::map<std::string, std::string>::iterator iter = mvt_alias.begin();
        iter != mvt_alias.end();  ++iter)
      if (iter->second == name)
      {
        Movement* mvt = new Movement(*it);
        mvt->SetType(iter->first);
        mvt_lst[iter->first] = mvt;
      }
  }

  if ((mvt_lst.find("black") == mvt_lst.end() && clothes_lst.find("black") != clothes_lst.end())
      || (mvt_lst.find("black") != mvt_lst.end() && clothes_lst.find("black") == clothes_lst.end()))
  {
    std::cerr << "Error: The movement \"black\" or the clothe \"black\" is not defined!" << std::endl;
    exit(EXIT_FAILURE);
  }
}

Body::Body(const Body& _body):
  clothes_lst(),
  mvt_lst(),
  current_clothe(NULL),
  current_mvt(NULL),
  current_loop(0),
  current_frame(0),
  previous_clothe(NULL),
  previous_mvt(NULL),
  weapon_member(new WeaponMember()),
  weapon_pos(0,0),
  last_refresh(0),
  walk_events(0),
  main_rotation_rad(0),
  skel_lst(),
  direction(DIRECTION_RIGHT),
  animation_number(_body.animation_number),
  need_rebuild(true),
  owner(NULL)
{
  // Add a special weapon member to the body
  members_lst["weapon"] = weapon_member;

  // Make a copy of members
  std::map<std::string, Member*>::const_iterator it1 = _body.members_lst.begin();
  while (it1 != _body.members_lst.end())
    {
      if (it1->second->GetName() != "weapon")
	{
	  std::pair<std::string,Member*> p;
	  p.first = it1->first;
	  p.second = new Member(*it1->second);
	  members_lst.insert(p);
	  it1++;
	}
      else
	it1++;
    }

  // Make a copy of clothes
  std::map<std::string, Clothe*>::const_iterator it2 = _body.clothes_lst.begin();
  while (it2 != _body.clothes_lst.end())
  {
    std::pair<std::string,Clothe*> p;
    p.first = it2->first;
    p.second = new Clothe(it2->second, members_lst);
    clothes_lst.insert(p);
    it2++;
  }

  // Movements are shared
  std::map<std::string, Movement*>::const_iterator it3 = _body.mvt_lst.begin();
  while (it3 != _body.mvt_lst.end())
  {
    std::pair<std::string,Movement*> p;
    p.first = it3->first;
    p.second = it3->second;
    Movement::ShareMovement(p.second);
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

  // Unshare the movements
  std::map<std::string, Movement*>::iterator it3 = mvt_lst.begin();
  while(it3 != mvt_lst.end())
  {
    Movement::UnshareMovement(it3->second);
    it3++;
  }

  members_lst.clear();
  clothes_lst.clear();
  mvt_lst.clear();
}

void Body::ResetMovement() const
{
  for (size_t layer=0; layer < current_clothe->GetLayers().size(); layer++)
    current_clothe->GetLayers()[layer]->ResetMovement();
}

void Body::ApplyMovement(Movement* mvt, uint frame)
{
#ifdef DEBUG
  if (mvt->GetType() != "breathe")
    MSG_DEBUG("body_anim", " %s uses %s-%s:%u",
	      owner->GetName().c_str(),
	      current_clothe->GetName().c_str(),
	      mvt->GetType().c_str(),
	      frame);
#endif

  // Move each member following the movement description
  // We do it using the order of the skeleton, as the movement of each
  // member affects the child members as well
  std::vector<junction>::iterator member = skel_lst.begin();
  for (;member != skel_lst.end();
       member++)
  {
    ASSERT( frame < mvt->GetFrames().size() );
    if (mvt->GetFrames()[frame].find(member->member->GetType()) != mvt->GetFrames()[frame].end())
    {
      // This member needs to be moved :
      member_mvt mb_mvt = mvt->GetFrames()[frame].find(member->member->GetType())->second;
      if(mb_mvt.follow_crosshair && ActiveCharacter().body == this && ActiveTeam().AccessWeapon().UseCrossHair())
      {
        // Use the movement of the crosshair
        double angle = owner->GetFiringAngle(); /* Get -2 * M_PI < angle =< 2 * M_PI*/
        if(angle < 0)
          angle += 2 * M_PI; // so now 0 < angle < 2 * M_PI;
        if(ActiveCharacter().GetDirection() == DIRECTION_LEFT)
          angle = M_PI - angle;

        mb_mvt.SetAngle(mb_mvt.GetAngle() + angle);
      }

      if(mb_mvt.follow_half_crosshair && ActiveCharacter().body == this && ActiveTeam().AccessWeapon().UseCrossHair())
      {
        // Use the movement of the crosshair
        double angle_rad = owner->GetFiringAngle(); // returns -180 < angle < 180
        if(ActiveCharacter().GetDirection() == DIRECTION_RIGHT)
          angle_rad /= 2; // -90 < angle < 90
        else
        if(angle_rad > M_PI_2)
          angle_rad = M_PI_2 - angle_rad / 2;//formerly in deg to 45 + (90 - angle) / 2;
        else
          angle_rad = -M_PI_2 - angle_rad / 2;//formerly in deg to -45 + (-90 - angle) / 2;



        if(angle_rad < 0)
          angle_rad += 2 * M_PI; // so now 0 < angle < 2 * M_PI;

        mb_mvt.SetAngle(mb_mvt.GetAngle() + angle_rad);
      }

      if(mb_mvt.follow_speed)
      {
        // Use the movement of the character
        double angle_rad = (owner->GetSpeedAngle());
        if(angle_rad < 0)
          angle_rad += 2 * M_PI; // so now 0 < angle < 2 * M_PI;
        if(owner->GetDirection() == DIRECTION_LEFT)
          angle_rad = M_PI - angle_rad;

        mb_mvt.SetAngle(mb_mvt.GetAngle() + angle_rad);
      }

      if(mb_mvt.follow_direction)
      {
        // Use the direction of the character
        if(owner->GetDirection() == DIRECTION_LEFT)
          mb_mvt.SetAngle(mb_mvt.GetAngle() + M_PI);
      }

      member->member->ApplyMovement(mb_mvt, skel_lst);

      // This movement needs to know the position of the member before
      // being applied so it does a second ApplyMovement after being used
      if(mb_mvt.follow_cursor && Mouse::GetInstance()->GetVisibility() == Mouse::MOUSE_VISIBLE)
      {
	member_mvt angle_mvt;

	Point2i v = owner->GetPosition() + member->member->GetPos();
	v += member->member->GetAnchorPos();

	if( owner->GetDirection() == DIRECTION_LEFT)
	{
		v.x = 2 * (int)owner->GetPosition().x + GetSize().x/2 - v.x;
		//v.x -= member->member->GetSprite().GetWidth();
	}
	v = Mouse::GetInstance()->GetWorldPosition() - v;

	if( v.Norm() < mb_mvt.follow_cursor_limit)
	{
          double angle = v.ComputeAngle(Point2i(0, 0));
	  angle *= owner->GetDirection();
	  angle -= owner->GetDirection() == DIRECTION_RIGHT ? M_PI:0;

          angle_mvt.SetAngle(angle);
          member->member->ApplyMovement(angle_mvt, skel_lst);
	}
      }
    }
  }
}

void Body::ApplySqueleton()
{
  // Move each member following the skeleton
  std::vector<junction>::iterator member = skel_lst.begin();
  // The first member is the body, we set it to pos:
  member->member->SetPos(Point2f(0.0, 0.0));
  member->member->SetAngle(0.0);
  member++;

  for (;member != skel_lst.end();
       member++)
  {
    // Place the other members depending on the parent member:
    member->member->ApplySqueleton(member->parent);
  }
}

void Body::Build()
{
  // Increase frame number if needed
  unsigned int last_frame = current_frame;
  unsigned int last_loop = current_loop;

  if (walk_events > 0 || current_mvt->GetType() != "walk") {

    if (Time::GetInstance()->Read() > last_refresh + current_mvt->GetFrameDuration()) {

      // Compute the new frame number
      current_frame += (Time::GetInstance()->Read()-last_refresh) / current_mvt->GetFrameDuration();
      last_refresh += (current_frame-last_frame) * current_mvt->GetFrameDuration();

      // This is the end of the animation
      if (current_frame >= current_mvt->GetFrames().size()) {

	current_frame = 0;
	current_loop++;

	// Number of loops
	if (current_mvt->GetNbLoops() != 0
	    && current_loop >= current_mvt->GetNbLoops()) {

	  // animation is finished - set it to the very last frame
	  current_loop = current_mvt->GetNbLoops() -1;
	  current_frame = current_mvt->GetFrames().size() -1;

	  if (previous_clothe)
	    SetClothe(previous_clothe->GetName());
	  if (previous_mvt)
	    SetMovement(previous_mvt->GetType());
	}
      }
    }
  }
  need_rebuild |= (last_frame != current_frame);
  need_rebuild |= (last_loop != current_loop);
  need_rebuild |= current_mvt->IsAlwaysMoving();

  if (!need_rebuild)
    return;

  ResetMovement();
  ApplySqueleton();
  ApplyMovement(current_mvt, current_frame);

  // Rotate each sprite, because the next part need to know the height
  // of the sprite once it is rotated
  for (int layer=0;layer < (int)current_clothe->GetLayers().size() ;layer++) {
    if (current_clothe->GetLayers()[layer]->GetName() != "weapon")
      current_clothe->GetLayers()[layer]->RotateSprite();
  }

  // Move the members to get the lowest member at the bottom of the skin rectangle
  member_mvt body_mvt;
  float y_max = 0;

  for (int lay=0;lay < (int)current_clothe->GetLayers().size() ;lay++) {
    if (current_clothe->GetLayers()[lay]->GetName() != "weapon")
      {
	Member* member = current_clothe->GetLayers()[lay];
	if (member->GetPosFloat().y + member->GetSprite().GetHeightMax() + member->GetSprite().GetRotationPoint().y > y_max
	   && !member->IsGoingThroughGround())
	  y_max = member->GetPosFloat().y + member->GetSprite().GetHeightMax() + member->GetSprite().GetRotationPoint().y;
      }
  }
  body_mvt.pos.y = (float)GetSize().y - y_max + current_mvt->GetTestBottom();
  body_mvt.pos.x = GetSize().x / 2.0 - skel_lst.front().member->GetSprite().GetWidth() / 2.0;
  body_mvt.SetAngle(main_rotation_rad);
  skel_lst.front().member->ApplyMovement(body_mvt, skel_lst);

  need_rebuild = false;
}

std::string Body::GetFrameLoop() const
{
  char str[16];
  snprintf(str, 16, "%u-%u", current_loop, current_frame);

  return std::string(str);
}

void Body::UpdateWeaponPosition(const Point2i& _pos)
{
  // update the weapon position
  if (direction == DIRECTION_RIGHT)
    weapon_pos = weapon_member->GetPos();
  else
    weapon_pos = Point2i(GetSize().x - weapon_member->GetPos().x,weapon_member->GetPos().y);
  weapon_pos += _pos;
}

void Body::DrawWeaponMember(const Point2i& _pos)
{
  UpdateWeaponPosition(_pos);

  weapon_member->Draw(_pos, _pos.x + GetSize().x/2, int(direction));
}

void Body::Draw(const Point2i& _pos)
{
  Build();

  int draw_weapon_member = 0;

  // Finally draw each layer one by one
  for (int layer=0;layer < (int)current_clothe->GetLayers().size() ;layer++) {

    if (current_clothe->GetLayers()[layer]->GetName() == "weapon") {
      // We draw the weapon member only if currently drawing the active character
      if (owner->IsActiveCharacter()) {
	ASSERT(draw_weapon_member == 0);
	ASSERT(current_clothe->GetLayers()[layer] == weapon_member);
	DrawWeaponMember(_pos);
	draw_weapon_member++;
      }
    } else {
      current_clothe->GetLayers()[layer]->Draw(_pos, _pos.x + GetSize().x/2, int(direction));
    }
  }

  // if we are drawing the active character but current clothe does not contain a weapon member,
  // we should draw it else weapon ammos number will be not displayed (see bug #11479)
  if (owner->IsActiveCharacter() && draw_weapon_member == 0) {
    DrawWeaponMember(_pos);
  }
}

void Body::AddChildMembers(Member* parent)
{
  // Add child members of the parent member to the skeleton
  // and continue recursively with child members
  for (std::map<std::string, v_attached>::const_iterator child = parent->GetAttachedMembers().begin();
       child != parent->GetAttachedMembers().end();
       child++)
  {
    // Find if the current clothe uses this member:
    for (uint lay = 0; lay < current_clothe->GetLayers().size(); lay++)
    {
      if (current_clothe->GetLayers()[lay]->GetType() == child->first)
      {
        // This child member is attached to his parent
        junction body;
        body.member = current_clothe->GetLayers()[lay];
        body.parent = parent;
        skel_lst.push_back(body);

        // continue recursively
        AddChildMembers(current_clothe->GetLayers()[lay]);
      }
    }
  }
}

void Body::BuildSqueleton()
{
  // Find each member used by the current clothe
  // and set the parent member of each member
  skel_lst.clear();

  // Find the "body" member as it is the top of the skeleton
  for (uint lay = 0; lay < current_clothe->GetLayers().size(); lay++)
    if (current_clothe->GetLayers()[lay]->GetType() == "body")
    {
      junction body;
      body.member = current_clothe->GetLayers()[lay];
      body.parent = NULL;
      skel_lst.push_back(body);
      break;
    }

  if (skel_lst.size() == 0)
  {
    std::cerr << "Unable to find the \"body\" member in the current clothe" << std::endl;
    ASSERT(false);
  }

  AddChildMembers(skel_lst.front().member);
}

void Body::SetClothe(const std::string& name)
{
  MSG_DEBUG("body", " %s use clothe %s", owner->GetName().c_str(), name.c_str());
  if (current_clothe && current_clothe->GetName() == name)
    return;

  if (clothes_lst.find(name) != clothes_lst.end())
  {
    current_clothe = clothes_lst.find(name)->second;
    BuildSqueleton();
    main_rotation_rad = 0;
    need_rebuild = true;
    previous_clothe = NULL;
  }
  else
    MSG_DEBUG("body", "Clothe not found");

  ASSERT(current_clothe != NULL);
}

void Body::SetMovement(const std::string& name)
{
  MSG_DEBUG("body", " %s use movement %s", owner->GetName().c_str(), name.c_str());
  if (current_mvt && current_mvt->GetType() == name) return;

  // Dirty trick to get the "black" movement to be played fully
  if (current_clothe && current_clothe->GetName() == "black") return;

  if (mvt_lst.find(name) != mvt_lst.end())
  {
    current_mvt = mvt_lst.find(name)->second;
    current_frame = 0;
    current_loop = 0;
    last_refresh = Time::GetInstance()->Read();
    main_rotation_rad = 0;
    need_rebuild = true;
    previous_mvt = NULL;
  }
  else
    MSG_DEBUG("body", "Movement not found");

  ASSERT(current_mvt != NULL);
}

void Body::PlayAnimation()
{
  std::ostringstream name;
  name << "animation" << RandomLocal().GetLong(0, animation_number - 1);
  SetClotheOnce(name.str());
  SetMovementOnce(name.str());
}

void Body::SetClotheOnce(const std::string& name)
{
  MSG_DEBUG("body", " %s use clothe %s once", owner->GetName().c_str(), name.c_str());
  if (current_clothe && current_clothe->GetName() == name) return;

  if (clothes_lst.find(name) != clothes_lst.end())
  {
    if (!previous_clothe)
      previous_clothe = current_clothe;
    current_clothe = clothes_lst.find(name)->second;
    BuildSqueleton();
    main_rotation_rad = 0;
    need_rebuild = true;
  }
  else
    MSG_DEBUG("body", "Clothe not found");

  ASSERT(current_clothe != NULL);
}

void Body::SetMovementOnce(const std::string& name)
{
  MSG_DEBUG("body", " %s use movement %s once", owner->GetName().c_str(), name.c_str());
  if (current_mvt && current_mvt->GetType() == name) return;

  // Dirty trick to get the "black" movement to be played fully
  if(current_clothe && current_clothe->GetName() == "black"  && name != "black") return;

  if (mvt_lst.find(name) != mvt_lst.end())
  {
    if (!previous_mvt)
    {
      previous_mvt = current_mvt;
    }

    current_mvt = mvt_lst.find(name)->second;
    current_frame = 0;
    current_loop = 0;
    last_refresh = Time::GetInstance()->Read();
    main_rotation_rad = 0;
    need_rebuild = true;
  }
  else
    MSG_DEBUG("body", "Movement not found");

  ASSERT(current_mvt != NULL);
}

void Body::GetTestRect(uint &l, uint&r, uint &t, uint &b) const
{
  if(direction == DIRECTION_RIGHT)
  {
    l = current_mvt->GetTestLeft();
    r = current_mvt->GetTestRight();
  }
  else
  {
    r = current_mvt->GetTestLeft();
    l = current_mvt->GetTestRight();
  }
  t = current_mvt->GetTestTop();
  b = current_mvt->GetTestBottom();
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
  if(current_mvt->GetType() == "walk")
  {
    SetMovement("breathe");
    SetFrame(0);
  }
}

bool Body::IsWalking() const
{
  return walk_events > 0 /* && current_mvt->type == "walk" */;
}

uint Body::GetMovementDuration() const
{
  return current_mvt->GetFrames().size() * current_mvt->GetFrameDuration();
}

uint Body::GetFrameCount() const
{
  return current_mvt->GetFrames().size();
}

void Body::SetFrame(uint no)
{
  ASSERT(no < current_mvt->GetFrames().size());
  current_frame = no;
  current_loop = 0;
  need_rebuild = true;
}

void Body::MakeParticles(const Point2i& pos)
{
  Build();

  for (int layer=0;layer < (int)current_clothe->GetLayers().size() ;layer++) {
    if (current_clothe->GetLayers()[layer]->GetType() != "weapon")
      ParticleEngine::AddNow(new BodyMemberParticle(current_clothe->GetLayers()[layer]->GetSprite(),
						    current_clothe->GetLayers()[layer]->GetPos()+pos));
  }
}

void Body::MakeTeleportParticles(const Point2i& pos, const Point2i& dst)
{
  Build();

  for (int layer=0;layer < (int)current_clothe->GetLayers().size() ;layer++) {
    if (current_clothe->GetLayers()[layer]->GetType() != "weapon")
      ParticleEngine::AddNow(new TeleportMemberParticle(current_clothe->GetLayers()[layer]->GetSprite(),
							current_clothe->GetLayers()[layer]->GetPos()+pos,
							current_clothe->GetLayers()[layer]->GetPos()+dst,
							int(direction)));
  }
}

void Body::SetRotation(double angle)
{
  MSG_DEBUG("body", "%s -> new angle: %i", owner->GetName().c_str(), angle);
  main_rotation_rad = angle;
  need_rebuild = true;
}

const std::string& Body::GetMovement() const
{
  return current_mvt->GetType();
}

const std::string& Body::GetClothe() const
{
  return current_clothe->GetName();
}

void Body::DebugState() const
{
  MSG_DEBUG("body.state", "clothe: %s\tmovement: %s\t%i", current_clothe->GetName().c_str(),
	    current_mvt->GetType().c_str(), current_frame);
  MSG_DEBUG("body.state", "(played once)clothe: %s\tmovement: %s",
            (previous_clothe?previous_clothe->GetName().c_str():"(NULL)"),
            (previous_mvt?previous_mvt->GetType().c_str():"(NULL)"));
  MSG_DEBUG("body.state", "need rebuild = %i",need_rebuild);
}
