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
 * Polygon class. Store point of a polygon and handle affine transformation
 *****************************************************************************/

#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "../tool/affine_transform.h"
#include "surface.h"

class Surface;

/** Use to draw the polygon */
class PolygonBuffer {
 public:
  Sint16 * vx;
  Sint16 * vy;
  int buffer_size;
  int array_size;
  PolygonBuffer();
  ~PolygonBuffer();
  int GetSize() const;
  void SetSize(const int size);
};

/** Store information about a simple shape */
class Polygon {
 protected:
  Surface * texture;
  Color * plane_color;
  Color * border_color;
  // Original shape
  std::vector<Point2d> original_shape;
  // Shape position after an affine transformation
  PolygonBuffer * shape_buffer;

 public:
  Polygon();
  Polygon(const std::vector<Point2d> shape);
  Polygon(const Polygon & poly);
  void Init();
  // Point handling
  void AddPoint(const Point2d & p);
  void ApplyTransformation(const AffineTransform2D & trans);
  void AddBezierCurve(const Point2d anchor1, const Point2d control1,
                      const Point2d control2, const Point2d anchor2, const int num_steps = 20);
  Polygon * GetBezierInterpolation(double smooth_value = 0.0);
  void Expand(const double expand_value);
  PolygonBuffer * GetPolygonBuffer() const;
  // Type of the polygon
  bool IsTextured() const;
  bool IsPlaneColor() const;
  bool IsBordered() const;
  // Texture handling
  Surface * GetTexture() const;
  void SetTexture(Surface * texture_surface);
  // Color handling
  void SetBorderColor(const Color & color);
  void SetPlaneColor(const Color & color);
  Color & GetBorderColor() const;
  Color & GetPlaneColor() const;
};

#endif /* POLYGON_H */
