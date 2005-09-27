/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 * Weapon dynamite : lorqu'on "tire", un baton de dynamite est lâché. Celui
 * explos après un laps de temps. La baton fait alors un gros trou dans la
 * carte, souffle les vers qui sont autour en leur faisant perdre de l'énergie.
 *****************************************************************************/

#ifndef DYNAMITE_H
#define DYNAMITE_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../team/character.h"
#ifdef CL
# include <ClanLib/display.h>
#endif
#include "weapon.h"

struct Sprite;

//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

// La représentation d'une dynamite
class BatonDynamite : public WeaponProjectile
{
private:
#ifdef CL
  CL_Sprite explosion;
#else
  Sprite *explosion;
#endif 
  bool explosion_active;

public:
  BatonDynamite();

  void Init();
  void Reset();
  void Draw();
  void Refresh();

protected:
  void SignalCollision();
  void SignalGhostState (bool was_dead);
};

//-----------------------------------------------------------------------------

class DynamiteConfig : public ExplosiveWeaponConfig 
{ 
public: 
  uint duree; 
public:
  DynamiteConfig();
  virtual void LoadXml(xmlpp::Element *elem);
};

//-----------------------------------------------------------------------------

// L'arme dynamite
class Dynamite : public Weapon
{
private:
  // channel used for sound
  int channel;

  // Image du trou de l'explosion
#ifdef CL
  CL_Surface impact;
#else
  SDL_Surface *impact;
#endif
   
  // Objet réprésentant la dynamite (= l'objet qui tombe et explose)
  BatonDynamite baton;

  void FinExplosion ();
  void p_Init();
  bool p_Shoot();

public:
  Dynamite();
  void p_Select();
  void Refresh();
  DynamiteConfig &cfg();
};

extern Dynamite dynamite;
//-----------------------------------------------------------------------------
} // namespace Wormux
#endif
