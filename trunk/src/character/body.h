/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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

#ifndef BODY_H
#define BODY_H
#include <map>

#include "movement.h"
#include "tool/point.h"
#include "tool/xml_document.h"

// Forward declarations
class Character;
class BodyList;
class Member;
class Clothe;
class Profile;

/*
 * FIXME: this class is either very useless either very badly used.
 * It would be nice to keep members in private section. There is no
 * copy constructor, this is really suspect.... */
class c_junction
{
public:
  Member* member;
  Member* parent;
  c_junction(): member(NULL), parent(NULL) {};
};

typedef class c_junction junction;

class Body
{
  /* If you need this, implement it (correctly) */
  const Body& operator=(const Body&);
  /**********************************************/

public:
  typedef enum
  {
    DIRECTION_LEFT = -1,
    DIRECTION_RIGHT = 1
  } Direction_t;
private:
  friend class BodyList;
  std::map<std::string, Member*> members_lst;
  std::map<std::string, Clothe*> clothes_lst;
  std::map<std::string, Movement*> mvt_lst;

  Clothe* current_clothe;
  Movement* current_mvt;

  // When a movement/clothe is played once, those variable saves the previous state;
  Movement* play_once_mvt_sauv;
  Clothe* play_once_clothe_sauv;
  uint play_once_frame_sauv;

  // For weapon position handling
  Member* weapon_member;
  Point2i weapon_pos;

  uint last_refresh; // Time elapsed since last refresh
  uint current_frame;
  int walk_events;

  double main_rotation_rad;

  std::vector<junction> squel_lst; // Squeleton of the body:
                                        // Order to use to build the body
                                        // First element: member to build
                                        // Secnd element: parent member
  Body::Direction_t direction;

  int animation_number;
  bool need_rebuild;

  void ResetMovement();
  void ApplyMovement(Movement* mvt, uint frame);
  void ApplySqueleton();

  void BuildSqueleton();
  void AddChildMembers(Member* parent);
  const Character* owner;

public:

  Body(xmlpp::Element *xml, Profile* res);
  Body(const Body&);
  ~Body();

  Point2i GetSize() {return Point2i(30,45);};

  void Draw(const Point2i& pos);
  void SetClothe(std::string name);
  void SetMovement(std::string name);
  void SetClotheOnce(std::string name); //use this only during one movement
  void SetMovementOnce(std::string name); //play the movement only once
  void SetRotation(double angle);
  void SetFrame(uint no);
  void SetDirection(Body::Direction_t dir);
  inline void SetOwner(const Character* belonger) { owner = belonger; };
  void PlayAnimation();
  void Build();
  void UpdateWeaponPosition(const Point2i& pos);

  const std::string& GetMovement();
  const std::string& GetClothe();
  void GetTestRect(uint &l, uint &r, uint &t, uint &b);
  const Direction_t &GetDirection() const;
  const Point2i &GetHandPosition() const;
  uint GetMovementDuration();
  uint GetFrame() { return current_frame; };
  uint GetFrameCount();

  void StartWalk();
  void StopWalk();
  void ResetWalk();
  bool IsWalking() { return walk_events > 0 && current_mvt->type == "walk";};

  void MakeParticles(const Point2i& pos);
  void MakeTeleportParticles(const Point2i& pos, const Point2i& dst);
  void DebugState();
};

#endif //BODY_H
