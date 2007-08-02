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
 * Virtual class to handle weapon in wormux.
 * Weapon projectile are handled in WeaponLauncher (see launcher.cpp and launcher.h).
 *****************************************************************************/

#ifndef WEAPON_H
#define WEAPON_H
#include <string>
#include "weapon_cfg.h"
#include "gui/progress_bar.h"
#include "include/base.h"
#include "particles/particle.h"
#include "sound/sound_sample.h"
#include "tool/debug.h"
#include <libxml++/libxml++.h>

class Character;
class Sprite;

// Infinite ammos constant
extern const int INFINITE_AMMO;

extern const uint BUTTON_ICO_WIDTH;
extern const uint BUTTON_ICO_HEIGHT;

extern const uint WEAPON_ICO_WIDTH;
extern const uint WEAPON_ICO_HEIGHT;

class WeaponStrengthBar : public ProgressBar
{
 public:
  bool visible ;
} ;

//-----------------------------------------------------------------------------

class Weapon
{
public:
  typedef enum
  {
    WEAPON_BAZOOKA,        WEAPON_AUTOMATIC_BAZOOKA, WEAPON_RIOT_BOMB, WEAPON_GRENADE,
    WEAPON_DISCO_GRENADE,  WEAPON_CLUSTER_BOMB,      WEAPON_GUN,       WEAPON_SHOTGUN,
    WEAPON_SUBMACHINE_GUN, WEAPON_BASEBALL,

    WEAPON_DYNAMITE,      WEAPON_MINE,

    WEAPON_SUPERTUX,      WEAPON_AIR_ATTACK,  WEAPON_ANVIL, WEAPON_GNU,
    WEAPON_POLECAT,       WEAPON_BOUNCE_BALL,

    WEAPON_TELEPORTATION, WEAPON_GRAPPLE,  WEAPON_LOWGRAV,   WEAPON_SUICIDE,
    WEAPON_SKIP_TURN,     WEAPON_JETPACK,     WEAPON_PARACHUTE, WEAPON_AIR_HAMMER,
    WEAPON_CONSTRUCT,     WEAPON_SNIPE_RIFLE, WEAPON_BLOWTORCH, WEAPON_SYRINGE
  } Weapon_type;
#define WEAPON_FIRST WEAPON_BAZOOKA
#define WEAPON_LAST  WEAPON_SYRINGE
  typedef enum {
    INVALID = 0,
    HEAVY,
    RIFLE,
    THROW,
    SPECIAL,
    DUEL,
    MOVE,
    TOOL
  } category_t;

protected:
  Weapon::Weapon_type m_type;
  Weapon::category_t  m_category;

  std::string m_id;
  std::string m_name;
  std::string m_help;
  bool m_is_active;
  Sprite *m_image;
  Sprite *m_weapon_fire;
  uint m_fire_remanence_time;

  typedef enum {
    weapon_origin_HAND,
    weapon_origin_OVER
  } weapon_origin_t;
  weapon_origin_t origin;

  Point2i hole_delta; // relative position of the hole of the weapon
  Point2i position;   // Position of the weapon

  // Time when the weapon is selected for the animation
  uint m_time_anim_begin;

  // Actual strength of the weapon
  double m_strength;

  // time of beginning to load (for choosing the strength)
  uint m_first_time_loading;

  // time of the last fire
  uint m_last_fire_time;

  // time between 2 shot
  uint m_time_between_each_shot;

  // change weapon after ? (for the grapple = true)
  bool m_can_change_weapon;

  // Extra parameters
  EmptyWeaponConfig *extra_params;

  typedef enum weapon_visibility {
    ALWAYS_VISIBLE,
    NEVER_VISIBLE,
    VISIBLE_ONLY_WHEN_ACTIVE,
    VISIBLE_ONLY_WHEN_INACTIVE
  } weapon_visibility_t;

  // Visibility
  weapon_visibility_t m_visibility;
  weapon_visibility_t m_unit_visibility;

  // how many times can we use this weapon (since the beginning of the game) ?
  int m_initial_nb_ammo;
  int m_initial_nb_unit_per_ammo;
  bool use_unit_on_first_shoot;
  bool can_be_used_on_closed_map;

  SoundSample loading_sound;

public:
  // weapon's icon
  Sprite * icon;

  // if max_strength != 0, display the strength bar
  double max_strength;

  bool use_flipping;
  const category_t& Category() const { return m_category; };

protected:
  virtual void p_Select();
  virtual void p_Deselect();
  virtual void Refresh() = 0;
  virtual bool p_Shoot() = 0;

public:
  Weapon(Weapon_type type,
         const std::string &id,
         EmptyWeaponConfig * params,
         weapon_visibility_t visibility = ALWAYS_VISIBLE);
  virtual ~Weapon();

  // Select or deselect the weapon
  void Select();
  void Deselect();

  // Gestion de l'arme
  void Manage();
  bool CanChangeWeapon() const ;

  // Draw the weapon
  virtual void Draw();
  virtual void DrawWeaponFire();

  void DrawAmmoUnits() const;

  Sprite & GetIcon() const;
  // Manage the numbers of ammunitions
  bool EnoughAmmo() const;
  void UseAmmo() const;
  bool EnoughAmmoUnit() const;
  void UseAmmoUnit() const;
  int ReadInitialNbAmmo() const;
  int ReadInitialNbUnit() const;

  bool CanBeUsedOnClosedMap() const;
  bool UseCrossHair() const { return min_angle != max_angle; };

  // Calculate weapon position
  virtual void PosXY (int &x, int &y) const;

  // Create a new action "shoot/stop_use" in action handler
  void NewActionWeaponShoot() const;
  void NewActionWeaponStopUse() const;

  // Prepare the shoot : set the angle and strenght of the weapon
  // Begin the shooting animation of the character
  void PrepareShoot(double strength, double angle);

  // Shot with the weapon
  // Return true if we have been able to trigger the weapon
  bool Shoot();

  // The weapon is still in use (animation for instance) ?
  virtual bool IsInUse() const;

  // the weapon is ready to use ? (is there bullets left ?)
  virtual bool IsReady() const ;

  // Begin to load, to choose the strength
  virtual void InitLoading() ;

  // Are we loading to choose the strength
  virtual bool IsLoading() const;

  // Stop loading
  virtual void StopLoading() ;

  // update strength (so the strength bar can be updated)
  virtual void UpdateStrength();

  const Point2i GetGunHolePosition() const;

  // Choose a target.
  virtual void ChooseTarget (Point2i mouse_pos);

  // Notify a move. It is usefull only for weapon which have strong
  // interactions with the physical engine such as grapple
  virtual void NotifyMove(bool /*collision*/){};

  // Handle a keyboard event.

  // Key Shoot management
  virtual void HandleKeyPressed_Shoot();
  virtual void HandleKeyRefreshed_Shoot();
  virtual void HandleKeyReleased_Shoot();

  // To override standard moves of character
  virtual void HandleKeyPressed_MoveRight();
  virtual void HandleKeyRefreshed_MoveRight();
  virtual void HandleKeyReleased_MoveRight();

  virtual void HandleKeyPressed_MoveLeft();
  virtual void HandleKeyRefreshed_MoveLeft();
  virtual void HandleKeyReleased_MoveLeft();

  virtual void HandleKeyPressed_Up();
  virtual void HandleKeyRefreshed_Up();
  virtual void HandleKeyReleased_Up();

  virtual void HandleKeyPressed_Down();
  virtual void HandleKeyRefreshed_Down();
  virtual void HandleKeyReleased_Down();

  virtual void HandleKeyPressed_Jump();
  virtual void HandleKeyRefreshed_Jump();
  virtual void HandleKeyReleased_Jump();

  virtual void HandleKeyPressed_HighJump();
  virtual void HandleKeyRefreshed_HighJump();
  virtual void HandleKeyReleased_HighJump();

  virtual void HandleKeyPressed_BackJump();
  virtual void HandleKeyRefreshed_BackJump();
  virtual void HandleKeyReleased_BackJump();

  // Other keys
  virtual void HandleKeyReleased_Num1(){};
  virtual void HandleKeyReleased_Num2(){};
  virtual void HandleKeyReleased_Num3(){};
  virtual void HandleKeyReleased_Num4(){};
  virtual void HandleKeyReleased_Num5(){};
  virtual void HandleKeyReleased_Num6(){};
  virtual void HandleKeyReleased_Num7(){};
  virtual void HandleKeyReleased_Num8(){};
  virtual void HandleKeyReleased_Num9(){};
  virtual void HandleKeyReleased_Less(){};
  virtual void HandleKeyReleased_More(){};

  // Handle a mouse event
  virtual void HandleMouseLeftClicReleased(){};
  virtual void HandleMouseWheelUp(){};
  virtual void HandleMouseWheelDown(){};

  // Get informed that the turn is over.
  virtual void SignalTurnEnd();

  // Stop using this weapon (only used with lowgrav and jetpack)
  virtual void ActionStopUse();

  // Load parameters from the xml config file
  // Return true if xml has been succesfully load
  bool LoadXml(const xmlpp::Element * weapon);

  // return the strength of the weapon
  const double ReadStrength() const;

  // Data access
  const std::string& GetName() const;
  const std::string& GetID() const;
  const std::string& GetHelp() const;
  Weapon_type GetType() const;

  // For localization purposes, each weapon needs to have its own
  // "%s team has won %d <weapon>" function
  virtual std::string GetWeaponWinString(const char *TeamName, uint items_count);

  // Allows or not the character selection with mouse click (tab is allowed)
  // This is used in weapons like the automated bazooka, where it's required
  // a target. Default is true.
  bool mouse_character_selection;

  inline void SetMinAngle(double min) {min_angle = min;}
  inline const double &GetMinAngle() const {return min_angle;}
  inline void SetMaxAngle(double max) {max_angle = max;}
  inline const double &GetMaxAngle() const {return max_angle;}
private:
  // Angle in radian between -PI to PI
  double min_angle, max_angle;

  /* If you need this, implement it (correctly)*/
  Weapon(const Weapon&);
  const Weapon& operator=(const Weapon&);
  /*********************************************/
};

#define DECLARE_GETWEAPONSTRING() \
std::string GetWeaponWinString(const char *TeamName, uint items_count ) const

//-----------------------------------------------------------------------------
#endif
