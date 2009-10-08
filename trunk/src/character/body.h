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

#ifndef BODY_H
#define BODY_H
#include <map>

#include <vector>
#include "include/base.h"
#include <WORMUX_point.h>
#include "tool/xml_document.h"
#include "character/movement.h"

// Forward declarations
class Character;
class BodyList;
class Member;
class Movement;
class Clothe;
class Profile;
typedef struct _xmlNode xmlNode;

enum BodyDirection
{
  DIRECTION_LEFT = -1,
  DIRECTION_RIGHT = 1
};
typedef enum BodyDirection BodyDirection_t;

/*
 * FIXME: this class is either very useless either very badly used.
 * It would be nice to keep members in private section. There is no
 * copy constructor, this is really suspect.... */
class c_junction
{
  public:
    Member * member;
    Member * parent;

    c_junction(): 
      member(NULL), 
      parent(NULL) {};
};

typedef class c_junction junction;

class Body
{
  private:
    /* If you need this, implement it (correctly) */
    const Body & operator = (const Body & obj);
    /**********************************************/
    friend class BodyList;
    friend class SkinMenu;

    std::map<std::string, Member *>   members_lst;
    std::map<std::string, Clothe *>   clothes_lst;
    std::map<std::string, Movement *> mvt_lst;
    const Clothe *                    current_clothe;
    Movement *                        current_mvt;
    uint                              current_loop;
    uint                              current_frame;

    // When a movement/clothe is played once, those variables save the previous state
    const Clothe *                    previous_clothe;
    Movement *                        previous_mvt;

    // For weapon position handling
    Member *                          weapon_member;

    // Time elapsed since last refresh
    uint                              last_refresh;

    int                               walk_events;
    double                            main_rotation_rad;
    std::vector<junction *>           skel_lst; // Body skeleton:
                                                // Order to use to build the body
                                                // First element: member to build
                                                // Secnd element: parent member
    BodyDirection_t                   direction;
    int                               animation_number;
    bool                              need_rebuild;
    const Character *                 owner;
    const xmlNode *                   mainXmlNode;
    const std::string                 mainFolder;
    
    void ResetMovement() const;
    void ApplyMovement(Movement * mvt, 
                       uint       frame);
    void ApplySqueleton();

    void ProcessFollowCrosshair(member_mvt & mb_mvt);
    void ProcessFollowHalfCrosshair(member_mvt & mb_mvt);
    void ProcessFollowSpeed(member_mvt & mb_mvt); 
    void ProcessFollowDirection(member_mvt & mb_mvt);

    void BuildSqueleton();
    void AddChildMembers(Member * parent);
    void DrawWeaponMember(const Point2i & _pos);
    void LoadMembers(xmlNodeArray &      nodes,
                     const std::string & main_folder);
    void LoadClothes(xmlNodeArray &  nodes,
                     const xmlNode * xml);
    void LoadMovements(xmlNodeArray &  nodes,
                       const xmlNode * xml);
    void FreeSkeletonVector();

  public:
    Body(const xmlNode *     xml, 
         const std::string & main_folder);
    Body(const Body & cpy);
    void Init(void);
    ~Body();

    //// MISC
    void                    Draw(const Point2i & pos);
    void                    PlayAnimation();
    void                    Build();
    void                    UpdateWeaponPosition(const Point2i & pos);
    void                    RefreshSprites();
    void                    StartWalk();
    void                    StopWalk();
    void                    ResetWalk() { walk_events = 0; };
    bool                    IsWalking() const ;
    void                    MakeParticles(const Point2i & pos);
    void                    MakeTeleportParticles(const Point2i & pos,
                                                  const Point2i & dst);
    void                    DebugState() const;

    //// SETTERS
    void                    SetClothe(const std::string & name);
    void                    SetMovement(const std::string & name);
    // use this only during one movement ...
    void                    SetClotheOnce(const std::string & name);
    // play the movement only once ...
    void                    SetMovementOnce(const std::string & name);
    void                    SetRotation(double angle);
    void                    SetFrame(uint no);
    void                    SetDirection(BodyDirection_t dir)    { direction = dir;      };
    inline void             SetOwner(const Character * belonger) { owner     = belonger; };

    //// GETTERS
    static Point2i          GetSize() { return Point2i(30,45); };
    const std::string &     GetMovement() const;
    const std::string &     GetClothe() const;
    std::string             GetFrameLoop() const;
    void                    GetTestRect(uint & l, 
                                        uint & r, 
                                        uint & t, 
                                        uint & b) const;
    const BodyDirection_t & GetDirection() const { return direction; };
    void                    GetRelativeHandPosition(Point2i & result) const;
    uint                    GetMovementDuration() const;
    uint                    GetFrame() const { return current_frame; };
    uint                    GetFrameCount() const;
};

#endif //BODY_H
