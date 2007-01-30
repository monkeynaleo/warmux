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
 * 2D Affine Transformation.
 *****************************************************************************/

#ifndef AFFINE_TRANSFORM_2D
#define AFFINE_TRANSFORM_2D

#include <math.h>
#include <stdio.h>
#include "point.h"

class AffineTransform2D {
 protected:
  // The matrix
  double x1, x2, xt;
  double y1, y2, yt;
  double w1, w2, wt;
 public:
  AffineTransform2D();
  void Init();
  void Set(AffineTransform2D &mat);
  void SetRotation(double rad_angle);
  void SetTranslation(double trans_x, double trans_y);
  void SetShrink(double shrink_x, double shrink_y);
  void SetShear(double shear_x, double shear_y);
  /* Matrix multiplication */
  AffineTransform2D operator*(const AffineTransform2D &mat) const;
  Point2i operator*(Point2i p) const;
  Point2d operator*(Point2d p) const;
  void DisplayMatrix();
};

#endif /* AFFINE_TRANSFORM_2D */
