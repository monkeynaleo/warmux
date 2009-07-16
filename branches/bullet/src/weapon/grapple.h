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

#ifndef GRAPPLE_H
#define GRAPPLE_H
//-----------------------------------------------------------------------------
#include "weapon.h"
#include "include/base.h"
#include <vector>
#include <Box2D.h>
#include "physic/physical_obj.h"
//-----------------------------------------------------------------------------
class GrappleConfig;
class Rope;
class RopeNode;

class Grapple : public Weapon
{
  private:


    // Rope launching data.
    double m_initial_angle;
    //uint m_launch_time;
    //uint m_hooked_time;

    Sprite* m_node_sprite;
     Rope * m_rope;
    SoundSample cable_sound;

  protected:
    void Refresh();
    void p_Deselect() ;
    bool p_Shoot();



  public:

    GrappleConfig& cfg();

    Grapple();
    ~Grapple();
    void Draw();

    virtual void ActionStopUse() {/* DetachRope(); */};
    // force detaching rope if time is out
    virtual void SignalTurnEnd();


    void UpdateTranslationStrings();
    std::string GetWeaponWinString(const char *TeamName, uint items_count) const;

    // Keys management
    void HandleKeyPressed_Up(bool shift);
    void HandleKeyRefreshed_Up(bool shift);
    void HandleKeyReleased_Up(bool shift);

    void HandleKeyPressed_Down(bool shift);
    void HandleKeyRefreshed_Down(bool shift);
    void HandleKeyReleased_Down(bool shift);

    void HandleKeyPressed_MoveRight(bool shift);
    void HandleKeyRefreshed_MoveRight(bool shift);
    void HandleKeyReleased_MoveRight(bool shift);

    void HandleKeyPressed_MoveLeft(bool shift);
    void HandleKeyRefreshed_MoveLeft(bool shift);
    void HandleKeyReleased_MoveLeft(bool shift);

    void HandleKeyPressed_Shoot(bool shift);
    void HandleKeyRefreshed_Shoot(bool) { };
    void HandleKeyReleased_Shoot(bool) { };

    void PrintDebugRope();
};


class Rope : public GameObj
{
public:

  Rope();
  ~Rope();

  void GoUp();
  void GoDown();
  void GoLeft();
  void GoRight();
  void StopUp();
  void StopDown();
  void StopLeft();
  void StopRight();

  bool IsAttached();
  void DetachRope();


  //if set to true, rope will attach on collision
  void SetAttachMode(bool i_attache_mode);

  virtual void Draw();

  virtual void Activate();
  virtual void Generate();
  virtual void Desactivate();

  virtual void SignalGroundCollision(const Point2d&);

protected:
  Point2i m_fixation_point;
  bool go_left, go_right;
  bool seek_attack_point;
  bool is_attached;

  Sprite* m_hook_sprite;

  std::vector<RopeNode *> m_rope_nodes;
  std::vector<b2Body *> m_rope_anchor;
  bool AttachRope();
  void Refresh() { };


};

class RopeNode : public GameObj
{
public :
  RopeNode();
  virtual void Draw();
protected:
  Sprite* m_node_sprite;


  void Refresh() { };

};
//-----------------------------------------------------------------------------
#endif /* GRAPPLE_H */
