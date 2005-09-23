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
 *  Refresh du vent
 *****************************************************************************/

#ifndef WIND_H
#define WIND_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../object/physical_obj.h"
#include "../tool/xml_document.h"
#include "../game/config.h"
#include "../gui/progress_bar.h"
#include <vector>

struct SDL_Surface;
struct Sprite;

//-----------------------------------------------------------------------------
namespace Wormux {
//-----------------------------------------------------------------------------

class WindParticle : public PhysicalObj
{
public:
#ifdef CL
  CL_Sprite sprite;
#else
  Sprite *sprite;
#endif
     
public:
  WindParticle();
  void Reset();
  void Init();
  void Resize(double size);
  void Draw();
  void Refresh();
};

//-----------------------------------------------------------------------------

class Wind
{
public:
  WindParticle *wind_particle_array;

private:
  BarreProg barre;
  long m_val,m_nv_val;
  uint m_last_move;

  uint m_last_part_mvt;

public:
  Wind();
  ~Wind();
  double GetStrength() const;
  void ChooseRandomVal();
  void SetVal (long val);
  void Init();
  void Refresh();
  void Reset();
  void Draw();
  void DrawParticles();
  void LoadXML(xmlpp::Element *xml);
#ifdef CL
  void LoadXML_Sprite(CL_ResourceManager* res);
#else
   // TODO
#endif
};


extern Wind wind;
//-----------------------------------------------------------------------------
} // namespace Wormux
//-----------------------------------------------------------------------------
#endif

