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
 * Ninja rope.
 *****************************************************************************/

#ifndef NINJA_ROPE_H
#define NINJA_ROPE_H
//-----------------------------------------------------------------------------
#include "weapon.h"
#include "../include/base.h"
//-----------------------------------------------------------------------------

class NinjaRope : public Weapon
{
  private:
    typedef struct 
    {
      Point2i pos;
      double angle;
      int sense;
    } rope_node_t;

    uint last_mvt;
    double last_broken_node_angle;
    double last_broken_node_sense;

    // Rope launching data.
    bool m_attaching;
    double m_initial_angle;
    uint m_launch_time;
    uint m_hooked_time;
    Sprite* m_hook_sprite;
    Sprite* m_node_sprite;

  public:
    std::list<rope_node_t> rope_nodes;
    Point2i fixation_point;
    bool go_left, go_right;
    double delta_len ;

    NinjaRope();
    void Active();
    void Draw();
    void NotifyMove(bool collision);
    void SignalTurnEnd();
    EmptyWeaponConfig& cfg();

    virtual void HandleKeyPressed_Up();
    virtual void HandleKeyRefreshed_Up();
    virtual void HandleKeyReleased_Up();
    
    virtual void HandleKeyPressed_Down();
    virtual void HandleKeyRefreshed_Down();
    virtual void HandleKeyReleased_Down();

    virtual void HandleKeyPressed_MoveRight();
    virtual void HandleKeyRefreshed_MoveRight();
    virtual void HandleKeyReleased_MoveRight();
    
    virtual void HandleKeyPressed_MoveLeft();
    virtual void HandleKeyRefreshed_MoveLeft();
    virtual void HandleKeyReleased_MoveLeft();

    virtual void HandleKeyPressed_Shoot();
    virtual void HandleKeyRefreshed_Shoot();
    virtual void HandleKeyReleased_Shoot();
  protected:
    void Refresh();
    void p_Deselect();
    bool p_Shoot();
    void GoUp();
    void GoDown();
    void GoLeft();
    void GoRight();
    void StopLeft();
    void StopRight();
    bool TryAttachRope();
    void UnattachRope();
    bool TryAddNode(int CurrentSense) ;
    bool TryBreakNode(int CurrentSense) ;
};

//-----------------------------------------------------------------------------
#endif /* NINJA_ROPE_H */
