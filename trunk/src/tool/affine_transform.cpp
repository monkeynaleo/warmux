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
* 2D Affine Transformation
*****************************************************************************/

#include "affine_transform.h"

AffineTransform2D::AffineTransform2D()
{
  Init();
}

void AffineTransform2D::Init()
{
  x1 = 1; x2 = 0; xt = 0;
  y1 = 0; y2 = 1; yt = 0;
  w1 = 0; w2 = 0; wt = 1;
}

void AffineTransform2D::Set(AffineTransform2D &mat)
{
  x1 = mat.x1;
  x2 = mat.x2;
  xt = mat.xt;
  y1 = mat.y1;
  y2 = mat.y2;
  yt = mat.yt;
  w1 = mat.w1;
  w2 = mat.w2;
  wt = mat.wt;
}

void AffineTransform2D::SetRotation(double rad_angle)
{
  Init();
  x1 = y2 = cos(rad_angle);
  x2 = - sin(rad_angle);
  y1 = - x2;
}

void AffineTransform2D::SetTranslation(double trans_x, double trans_y)
{
  Init();
  xt = trans_x;
  yt = trans_y;
}

void AffineTransform2D::SetShrink(double shrink_x, double shrink_y)
{
  Init();
  x1 = shrink_x;
  y2 = shrink_y;
}

void AffineTransform2D::SetShear(double shear_x, double shear_y)
{
  Init();
  x2 = shear_x;
  y1 = shear_y;
}

/* Matrix multiplication */
AffineTransform2D AffineTransform2D::operator*(const AffineTransform2D &mat) const
{
  AffineTransform2D tmp;
  tmp.x1 = x1 * mat.x1 + x2 * mat.y1 + xt * mat.w1;
  tmp.x2 = x1 * mat.x2 + x2 * mat.y2 + xt * mat.w2;
  tmp.xt = x1 * mat.xt + x2 * mat.yt + xt * mat.wt;

  tmp.y1 = y1 * mat.x1 + y2 * mat.y1 + yt * mat.w1;
  tmp.y2 = y1 * mat.x2 + y2 * mat.y2 + yt * mat.w2;
  tmp.yt = y1 * mat.xt + y2 * mat.yt + yt * mat.wt;

  tmp.w1 = w1 * mat.x1 + w2 * mat.y1 + wt * mat.w1;
  tmp.w2 = w1 * mat.x2 + w2 * mat.y2 + wt * mat.w2;
  tmp.wt = w1 * mat.xt + w2 * mat.yt + wt * mat.wt;
  return tmp;
}

Point2i AffineTransform2D::operator*(Point2i p) const
{
  Point2i tmp;
  tmp.x = (int)(x1 * (double)p.x + x2 * (double)p.y + xt);
  tmp.y = (int)(y1 * (double)p.x + y2 * (double)p.y + yt);
  return tmp;
}

Point2d AffineTransform2D::operator*(Point2d p) const
{
  Point2d tmp;
  tmp.x = (double)(x1 * (double)p.x + x2 * (double)p.y + xt);
  tmp.y = (double)(y1 * (double)p.x + y2 * (double)p.y + yt);
  return tmp;
}

void AffineTransform2D::DisplayMatrix()
{
  printf("[ %3.3f %3.3f %3.3f ]\n", x1, x2, xt);
  printf("[ %3.3f %3.3f %3.3f ]\n", y1, y2, yt);
  printf("[ %3.3f %3.3f %3.3f ]\n", w1, w2, wt);
}

