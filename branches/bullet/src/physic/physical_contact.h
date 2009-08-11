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
 *TODO : write description
 *****************************************************************************/

#ifndef PHYSICAL_CONTACT_H
#define PHYSICAL_CONTACT_H
#include <vector>
#include <string>
#include "WORMUX_point.h"
#include "WORMUX_rectangle.h"
#include "WORMUX_types.h"
#include "tool/xml_document.h"
#include "graphic/color.h"

class PhysicalShape;

class PhysicalContact
{
  
public: 
  
  // A
  virtual void GetSpeedA(double &norm, double &angle_rad) const = 0;
  virtual Point2d GetSpeedA() const = 0;
  virtual PhysicalShape *GetShapeA() = 0;

  // B
  virtual void GetSpeedB(double &norm, double &angle_rad) const = 0;
  virtual Point2d GetSpeedB() const = 0;
  virtual PhysicalShape *GetShapeB() = 0;

 };


#endif
